/* dzl-shortcut-manager.c
 *
 * Copyright (C) 2016 Christian Hergert <chergert@redhat.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define G_LOG_DOMAIN "dzl-shortcut-manager.h"

#include "config.h"

#include <glib/gi18n.h>

#include "dzl-debug.h"

#include "shortcuts/dzl-shortcut-controller.h"
#include "shortcuts/dzl-shortcut-label.h"
#include "shortcuts/dzl-shortcut-manager.h"
#include "shortcuts/dzl-shortcut-private.h"
#include "shortcuts/dzl-shortcut-private.h"
#include "shortcuts/dzl-shortcuts-group.h"
#include "shortcuts/dzl-shortcuts-section.h"
#include "shortcuts/dzl-shortcuts-shortcut.h"
#include "util/dzl-gtk.h"
#include "util/dzl-util-private.h"

typedef struct
{
  /*
   * This is the currently selected theme by the user (or default until
   * a theme has been set). You can change this with the
   * dzl_shortcut_manager_set_theme() function.
   */
  DzlShortcutTheme *theme;

  /*
   * To avoid re-implementing lots of behavior, we use an internal theme
   * to store all the built-in keybindings for shortcut controllers. Then,
   * when loading themes (particularly default), we copy these into that
   * theme to give the effect of inheritance.
   */
  DzlShortcutTheme *internal_theme;

  /*
   * This is an array of all of the themes owned by the manager. It does
   * not, however, contain the @internal_theme instance.
   */
  GPtrArray *themes;

  /*
   * This is the user directory to save changes to the theme so they can
   * be reloaded later.
   */
  gchar *user_dir;

  /*
   * To simplify the process of registering entries, we allow them to be
   * called from the instance init function. But we only want to see those
   * entries once. If we did this from class_init(), we'd run into issues
   * with gtk not being initialized yet (and we need access to keymaps).
   *
   * This allows us to keep a unique pointer to know if we've already
   * dealt with some entries by discarding them up front.
   */
  GHashTable *seen_entries;

  /*
   * We store a tree of various shortcut data so that we can build the
   * shortcut window using the registered controller actions. This is
   * done in dzl_shortcut_manager_add_shortcuts_to_window().
   */
  GNode *root;

  /*
   * GHashTable to match command/action to a nodedata, useful to generate
   * a tooltip-text string for a given widget.
   */
  GHashTable *command_id_to_node_data;

  /*
   * We keep track of the search paths for loading themes here. Each element is
   * a string containing the path to the file-system resource. If the path
   * starts with 'resource://" it is assumed a resource embedded in the current
   * process.
   */
  GQueue search_path;

  /*
   * Upon making changes to @search path, we need to reload the themes. This
   * is a GSource identifier to indicate our queued reload request.
   */
  guint reload_handler;
} DzlShortcutManagerPrivate;

enum {
  PROP_0,
  PROP_THEME,
  PROP_THEME_NAME,
  PROP_USER_DIR,
  N_PROPS
};

enum {
  CHANGED,
  N_SIGNALS
};

static void list_model_iface_init               (GListModelInterface *iface);
static void initable_iface_init                 (GInitableIface      *iface);
static void dzl_shortcut_manager_load_directory (DzlShortcutManager  *self,
                                                 const gchar         *resource_dir,
                                                 GCancellable        *cancellable);
static void dzl_shortcut_manager_load_resources (DzlShortcutManager  *self,
                                                 const gchar         *resource_dir,
                                                 GCancellable        *cancellable);
static void dzl_shortcut_manager_merge          (DzlShortcutManager  *self,
                                                 DzlShortcutTheme    *theme);

G_DEFINE_TYPE_WITH_CODE (DzlShortcutManager, dzl_shortcut_manager, G_TYPE_OBJECT,
                         G_ADD_PRIVATE (DzlShortcutManager)
                         G_IMPLEMENT_INTERFACE (G_TYPE_INITABLE, initable_iface_init)
                         G_IMPLEMENT_INTERFACE (G_TYPE_LIST_MODEL, list_model_iface_init))

static GParamSpec *properties [N_PROPS];
static guint signals [N_SIGNALS];

static gboolean
free_node_data (GNode    *node,
                gpointer  user_data)
{
  DzlShortcutNodeData *data = node->data;

  g_assert (data != NULL);
  g_assert (DZL_IS_SHORTCUT_NODE_DATA (data));

  data->magic = 0xAAAAAAAA;

  g_slice_free (DzlShortcutNodeData, data);

  return FALSE;
}

static void
destroy_theme (gpointer data)
{
  g_autoptr(DzlShortcutTheme) theme = data;

  g_assert (DZL_IS_SHORTCUT_THEME (theme));

  _dzl_shortcut_theme_set_manager (theme, NULL);
}

void
dzl_shortcut_manager_reload (DzlShortcutManager *self,
                             GCancellable       *cancellable)
{
  DzlShortcutManagerPrivate *priv = dzl_shortcut_manager_get_instance_private (self);
  g_autofree gchar *theme_name = NULL;
  g_autofree gchar *parent_theme_name = NULL;
  DzlShortcutTheme *theme = NULL;
  guint previous_len;

  DZL_ENTRY;

  g_assert (DZL_IS_SHORTCUT_MANAGER (self));
  g_assert (!cancellable || G_IS_CANCELLABLE (cancellable));

  DZL_TRACE_MSG ("reloading shortcuts, current theme is “%s”",
                 priv->theme ? dzl_shortcut_theme_get_name (priv->theme) : "internal");

  /*
   * If there is a queued reload when we get here, just remove it. When called
   * from a queued callback, this will already be zeroed.
   */
  if (priv->reload_handler != 0)
    {
      g_source_remove (priv->reload_handler);
      priv->reload_handler = 0;
    }

  if (priv->theme != NULL)
    {
      /*
       * Keep a copy of the current theme name so that we can return to the
       * same theme if it is still available. If it has disappeared, then we
       * will try to fallback to the parent theme.
       */
      theme_name = g_strdup (dzl_shortcut_theme_get_name (priv->theme));
      parent_theme_name = g_strdup (dzl_shortcut_theme_get_parent_name (priv->theme));
      _dzl_shortcut_theme_detach (priv->theme);
      g_clear_object (&priv->theme);
    }

  /*
   * Now remove all of our old themes and notify listeners via the GListModel
   * interface so things like preferences can update. We ensure that we place
   * a "default" item in the list as we should always have one. We'll append to
   * it when loading the default theme anyway.
   *
   * The default theme always inherits from internal so that we can store
   * our widget/controller defined shortcuts separate from the mutable default
   * theme which various applications might want to tweak in their overrides.
   */
  previous_len = priv->themes->len;
  g_ptr_array_remove_range (priv->themes, 0, previous_len);
  g_ptr_array_add (priv->themes, g_object_new (DZL_TYPE_SHORTCUT_THEME,
                                               "name", "default",
                                               "title", _("Default Shortcuts"),
                                               "parent-name", "internal",
                                               NULL));
  _dzl_shortcut_theme_set_manager (g_ptr_array_index (priv->themes, 0), self);
  g_list_model_items_changed (G_LIST_MODEL (self), 0, previous_len, 1);

  /*
   * Okay, now we can go and load all the files in the search path. After
   * loading a file, the loader code will call dzl_shortcut_manager_merge()
   * to layer that theme into any base theme which matches the name. This
   * allows application plugins to simply load a keytheme file to have it
   * merged into the parent keytheme.
   */
  for (const GList *iter = priv->search_path.tail; iter != NULL; iter = iter->prev)
    {
      const gchar *directory = iter->data;

      if (g_str_has_prefix (directory, "resource://"))
        dzl_shortcut_manager_load_resources (self, directory, cancellable);
      else
        dzl_shortcut_manager_load_directory (self, directory, cancellable);
    }

  DZL_TRACE_MSG ("Attempting to reset theme to %s",
                 theme_name ?: parent_theme_name ?: "internal");

  /* Now try to reapply the same theme if we can find it. */
  if (theme_name != NULL)
    {
      theme = dzl_shortcut_manager_get_theme_by_name (self, theme_name);
      if (theme != NULL)
        dzl_shortcut_manager_set_theme (self, theme);
    }

  if (priv->theme == NULL && parent_theme_name != NULL)
    {
      theme = dzl_shortcut_manager_get_theme_by_name (self, parent_theme_name);
      if (theme != NULL)
        dzl_shortcut_manager_set_theme (self, theme);
    }

  /* Notify possibly changed properties */
  g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_THEME]);
  g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_THEME_NAME]);

  DZL_EXIT;
}

static gboolean
dzl_shortcut_manager_do_reload (gpointer data)
{
  DzlShortcutManager *self = data;
  DzlShortcutManagerPrivate *priv = dzl_shortcut_manager_get_instance_private (self);

  g_assert (DZL_IS_SHORTCUT_MANAGER (self));

  priv->reload_handler = 0;
  dzl_shortcut_manager_reload (self, NULL);
  return G_SOURCE_REMOVE;
}

void
dzl_shortcut_manager_queue_reload (DzlShortcutManager *self)
{
  DzlShortcutManagerPrivate *priv = dzl_shortcut_manager_get_instance_private (self);

  DZL_ENTRY;

  g_assert (DZL_IS_SHORTCUT_MANAGER (self));

  if (priv->reload_handler == 0)
    {
      /*
       * Reload at a high priority to happen immediately, but defer
       * until getting to the main loop.
       */
      priv->reload_handler =
        gdk_threads_add_idle_full (G_PRIORITY_HIGH,
                                   dzl_shortcut_manager_do_reload,
                                   g_object_ref (self),
                                   g_object_unref);
    }

  DZL_EXIT;
}

static void
dzl_shortcut_manager_finalize (GObject *object)
{
  DzlShortcutManager *self = (DzlShortcutManager *)object;
  DzlShortcutManagerPrivate *priv = dzl_shortcut_manager_get_instance_private (self);

  g_clear_pointer (&priv->command_id_to_node_data, g_hash_table_unref);

  if (priv->root != NULL)
    {
      g_node_traverse (priv->root, G_IN_ORDER, G_TRAVERSE_ALL, -1, free_node_data, NULL);
      g_node_destroy (priv->root);
      priv->root = NULL;
    }

  if (priv->theme != NULL)
    {
      _dzl_shortcut_theme_detach (priv->theme);
      g_clear_object (&priv->theme);
    }

  g_clear_pointer (&priv->seen_entries, g_hash_table_unref);
  g_clear_pointer (&priv->themes, g_ptr_array_unref);
  g_clear_pointer (&priv->user_dir, g_free);
  g_clear_object (&priv->internal_theme);

  G_OBJECT_CLASS (dzl_shortcut_manager_parent_class)->finalize (object);
}

static void
dzl_shortcut_manager_get_property (GObject    *object,
                                   guint       prop_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  DzlShortcutManager *self = (DzlShortcutManager *)object;

  switch (prop_id)
    {
    case PROP_THEME:
      g_value_set_object (value, dzl_shortcut_manager_get_theme (self));
      break;

    case PROP_THEME_NAME:
      g_value_set_string (value, dzl_shortcut_manager_get_theme_name (self));
      break;

    case PROP_USER_DIR:
      g_value_set_string (value, dzl_shortcut_manager_get_user_dir (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_shortcut_manager_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  DzlShortcutManager *self = (DzlShortcutManager *)object;

  switch (prop_id)
    {
    case PROP_THEME:
      dzl_shortcut_manager_set_theme (self, g_value_get_object (value));
      break;

    case PROP_THEME_NAME:
      dzl_shortcut_manager_set_theme_name (self, g_value_get_string (value));
      break;

    case PROP_USER_DIR:
      dzl_shortcut_manager_set_user_dir (self, g_value_get_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_shortcut_manager_class_init (DzlShortcutManagerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = dzl_shortcut_manager_finalize;
  object_class->get_property = dzl_shortcut_manager_get_property;
  object_class->set_property = dzl_shortcut_manager_set_property;

  properties [PROP_THEME] =
    g_param_spec_object ("theme",
                         "Theme",
                         "The current key theme.",
                         DZL_TYPE_SHORTCUT_THEME,
                         (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  properties [PROP_THEME_NAME] =
    g_param_spec_string ("theme-name",
                         "Theme Name",
                         "The name of the current theme",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  properties [PROP_USER_DIR] =
    g_param_spec_string ("user-dir",
                         "User Dir",
                         "The directory for saved user modifications",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);

  signals [CHANGED] =
    g_signal_new ("changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0, NULL, NULL, NULL, G_TYPE_NONE, 0);
}

static guint
shortcut_entry_hash (gconstpointer key)
{
  DzlShortcutEntry *entry = (DzlShortcutEntry *)key;
  guint command_hash = 0;
  guint section_hash = 0;
  guint group_hash = 0;
  guint title_hash = 0;
  guint subtitle_hash = 0;

  if (entry->command != NULL)
    command_hash = g_str_hash (entry->command);

  if (entry->section != NULL)
    section_hash = g_str_hash (entry->section);

  if (entry->group != NULL)
    group_hash = g_str_hash (entry->group);

  if (entry->title != NULL)
    title_hash = g_str_hash (entry->title);

  if (entry->subtitle != NULL)
    subtitle_hash = g_str_hash (entry->subtitle);

  return (command_hash ^ section_hash ^ group_hash ^ title_hash ^ subtitle_hash);
}

static void
dzl_shortcut_manager_init (DzlShortcutManager *self)
{
  DzlShortcutManagerPrivate *priv = dzl_shortcut_manager_get_instance_private (self);

  priv->command_id_to_node_data = g_hash_table_new (g_str_hash, g_str_equal);
  priv->seen_entries = g_hash_table_new (shortcut_entry_hash, NULL);
  priv->themes = g_ptr_array_new_with_free_func (destroy_theme);
  priv->root = g_node_new (NULL);
  priv->internal_theme = g_object_new (DZL_TYPE_SHORTCUT_THEME,
                                       "name", "internal",
                                       NULL);
}

static void
dzl_shortcut_manager_load_directory (DzlShortcutManager  *self,
                                     const gchar         *directory,
                                     GCancellable        *cancellable)
{
  g_autoptr(GDir) dir = NULL;
  const gchar *name;

  DZL_ENTRY;

  g_assert (DZL_IS_SHORTCUT_MANAGER (self));
  g_assert (directory != NULL);
  g_assert (!cancellable || G_IS_CANCELLABLE (cancellable));

  DZL_TRACE_MSG ("directory = %s", directory);

  if (!g_file_test (directory, G_FILE_TEST_IS_DIR))
    DZL_EXIT;

  if (NULL == (dir = g_dir_open (directory, 0, NULL)))
    DZL_EXIT;

  while (NULL != (name = g_dir_read_name (dir)))
    {
      g_autofree gchar *path = g_build_filename (directory, name, NULL);
      g_autoptr(DzlShortcutTheme) theme = NULL;
      g_autoptr(GError) local_error = NULL;

      theme = dzl_shortcut_theme_new (NULL);

      if (dzl_shortcut_theme_load_from_path (theme, path, cancellable, &local_error))
        {
          _dzl_shortcut_theme_set_manager (theme, self);
          dzl_shortcut_manager_merge (self, theme);
        }
      else
        g_warning ("%s", local_error->message);
    }

  DZL_EXIT;
}

static void
dzl_shortcut_manager_load_resources (DzlShortcutManager *self,
                                     const gchar        *resource_dir,
                                     GCancellable       *cancellable)
{
  g_auto(GStrv) children = NULL;

  DZL_ENTRY;

  g_assert (DZL_IS_SHORTCUT_MANAGER (self));
  g_assert (resource_dir != NULL);
  g_assert (g_str_has_prefix (resource_dir, "resource://"));
  g_assert (!cancellable || G_IS_CANCELLABLE (cancellable));

  DZL_TRACE_MSG ("resource_dir = %s", resource_dir);

  if (g_str_has_prefix (resource_dir, "resource://"))
    resource_dir += strlen ("resource://");

  children = g_resources_enumerate_children (resource_dir, 0, NULL);

  if (children != NULL)
    {
      for (guint i = 0; children[i] != NULL; i++)
        {
          g_autofree gchar *path = g_build_path ("/", resource_dir, children[i], NULL);
          g_autoptr(DzlShortcutTheme) theme = NULL;
          g_autoptr(GError) local_error = NULL;
          g_autoptr(GBytes) bytes = NULL;
          const gchar *data;
          gsize len = 0;

          if (NULL == (bytes = g_resources_lookup_data (path, 0, NULL)))
            continue;

          data = g_bytes_get_data (bytes, &len);
          theme = dzl_shortcut_theme_new (NULL);

          if (dzl_shortcut_theme_load_from_data (theme, data, len, &local_error))
            {
              _dzl_shortcut_theme_set_manager (theme, self);
              dzl_shortcut_manager_merge (self, theme);
            }
          else
            g_warning ("%s", local_error->message);
        }
    }

  DZL_EXIT;
}

static gboolean
dzl_shortcut_manager_initiable_init (GInitable     *initable,
                                     GCancellable  *cancellable,
                                     GError       **error)
{
  DzlShortcutManager *self = (DzlShortcutManager *)initable;

  g_assert (DZL_IS_SHORTCUT_MANAGER (self));
  g_assert (!cancellable || G_IS_CANCELLABLE (cancellable));

  dzl_shortcut_manager_reload (self, cancellable);

  return TRUE;
}

static void
initable_iface_init (GInitableIface *iface)
{
  iface->init = dzl_shortcut_manager_initiable_init;
}

/**
 * dzl_shortcut_manager_get_default:
 *
 * Gets the singleton #DzlShortcutManager for the process.
 *
 * Returns: (transfer none) (not nullable): An #DzlShortcutManager.
 */
DzlShortcutManager *
dzl_shortcut_manager_get_default (void)
{
  static DzlShortcutManager *instance;

  if (instance == NULL)
    {
      instance = g_object_new (DZL_TYPE_SHORTCUT_MANAGER, NULL);
      g_object_add_weak_pointer (G_OBJECT (instance), (gpointer *)&instance);
    }

  return instance;
}

/**
 * dzl_shortcut_manager_get_theme:
 * @self: (nullable): A #DzlShortcutManager or %NULL
 *
 * Gets the "theme" property.
 *
 * Returns: (transfer none) (not nullable): An #DzlShortcutTheme.
 */
DzlShortcutTheme *
dzl_shortcut_manager_get_theme (DzlShortcutManager *self)
{
  DzlShortcutManagerPrivate *priv;

  g_return_val_if_fail (!self || DZL_IS_SHORTCUT_MANAGER (self), NULL);

  if (self == NULL)
    self = dzl_shortcut_manager_get_default ();

  priv = dzl_shortcut_manager_get_instance_private (self);

  if G_LIKELY (priv->theme != NULL)
    return priv->theme;

  for (guint i = 0; i < priv->themes->len; i++)
    {
      DzlShortcutTheme *theme = g_ptr_array_index (priv->themes, i);

      if (g_strcmp0 (dzl_shortcut_theme_get_name (theme), "default") == 0)
        {
          priv->theme = g_object_ref (theme);
          return priv->theme;
        }
    }

  return priv->internal_theme;
}

/**
 * dzl_shortcut_manager_set_theme:
 * @self: An #DzlShortcutManager
 * @theme: (not nullable): An #DzlShortcutTheme
 *
 * Sets the theme for the shortcut manager.
 */
void
dzl_shortcut_manager_set_theme (DzlShortcutManager *self,
                                DzlShortcutTheme   *theme)
{
  DzlShortcutManagerPrivate *priv = dzl_shortcut_manager_get_instance_private (self);

  DZL_ENTRY;

  g_return_if_fail (DZL_IS_SHORTCUT_MANAGER (self));
  g_return_if_fail (DZL_IS_SHORTCUT_THEME (theme));

  /*
   * It is important that DzlShortcutController instances watch for
   * notify::theme so that they can reset their state. Otherwise, we
   * could be transitioning between incorrect contexts.
   */

  if (priv->theme != theme)
    {
      if (priv->theme != NULL)
        {
          _dzl_shortcut_theme_detach (priv->theme);
          g_clear_object (&priv->theme);
        }

      if (theme != NULL)
        {
          priv->theme = g_object_ref (theme);
          _dzl_shortcut_theme_attach (priv->theme);
        }

      DZL_TRACE_MSG ("theme set to “%s”",
                     theme ? dzl_shortcut_theme_get_name (theme) : "internal");

      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_THEME]);
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_THEME_NAME]);
    }
}

/*
 * dzl_shortcut_manager_run_phase:
 * @self: a #DzlShortcutManager
 * @event: the event in question
 * @chord: the current chord for the toplevel
 * @phase: the phase (capture, bubble)
 * @widget: the widget the event was destined for
 * @focus: the current focus widget
 *
 * Runs a particular phase of the event dispatch.
 *
 * A phase can be either capture or bubble. Capture tries to deliver the
 * event starting from the root down to the given widget. Bubble tries to
 * deliver the event starting from the widget up to the toplevel.
 *
 * These two phases allow stealing before or after, depending on the needs
 * of the keybindings.
 *
 * Returns: A #DzlShortcutMatch
 */
static DzlShortcutMatch
dzl_shortcut_manager_run_phase (DzlShortcutManager     *self,
                                const GdkEventKey      *event,
                                const DzlShortcutChord *chord,
                                int                     phase,
                                GtkWidget              *widget,
                                GtkWidget              *focus)
{
  GtkWidget *ancestor = widget;
  GQueue queue = G_QUEUE_INIT;
  DzlShortcutMatch ret = DZL_SHORTCUT_MATCH_NONE;

  g_assert (DZL_IS_SHORTCUT_MANAGER (self));
  g_assert (event != NULL);
  g_assert (chord != NULL);
  g_assert ((phase & DZL_SHORTCUT_PHASE_GLOBAL) == 0);
  g_assert (GTK_IS_WIDGET (widget));
  g_assert (GTK_IS_WIDGET (focus));

  /*
   * Collect all the widgets that might be needed for this phase and order them
   * so that we can process from first-to-last. Capture phase is
   * toplevel-to-widget, and bubble is widget-to-toplevel.  Dispatch only has
   * the the widget itself.
   */
  do
    {
      if (phase == DZL_SHORTCUT_PHASE_CAPTURE)
        g_queue_push_head (&queue, g_object_ref (ancestor));
      else
        g_queue_push_tail (&queue, g_object_ref (ancestor));
      ancestor = gtk_widget_get_parent (ancestor);
    }
  while (phase != DZL_SHORTCUT_PHASE_DISPATCH && ancestor != NULL);

  /*
   * Now look through our widget chain to find a match to activate.
   */
  for (const GList *iter = queue.head; iter; iter = iter->next)
    {
      GtkWidget *current = iter->data;
      DzlShortcutController *controller;

      controller = dzl_shortcut_controller_try_find (current);

      if (controller != NULL)
        {
          /*
           * Now try to activate the event using the controller. If we get
           * any result other than DZL_SHORTCUT_MATCH_NONE, we need to stop
           * processing and swallow the event.
           *
           * Multiple controllers can have a partial match, but if any hits
           * a partial match, it's undefined behavior to also have a shortcut
           * which would activate.
           */
          ret = _dzl_shortcut_controller_handle (controller, event, chord, phase, focus);
          if (ret)
            DZL_GOTO (cleanup);
        }

      /*
       * If we are in the dispatch phase, we will only see our target widget for
       * the event delivery. Try to dispatch the event and if so we consider
       * the event handled.
       */
      if (phase == DZL_SHORTCUT_PHASE_DISPATCH)
        {
          if (gtk_widget_event (current, (GdkEvent *)event))
            {
              ret = DZL_SHORTCUT_MATCH_EQUAL;
              DZL_GOTO (cleanup);
            }
        }
    }

cleanup:
  g_queue_foreach (&queue, (GFunc)g_object_unref, NULL);
  g_queue_clear (&queue);

  DZL_RETURN (ret);
}

static DzlShortcutMatch
dzl_shortcut_manager_run_global (DzlShortcutManager     *self,
                                 const GdkEventKey      *event,
                                 const DzlShortcutChord *chord,
                                 DzlShortcutPhase        phase,
                                 DzlShortcutController  *root,
                                 GtkWidget              *widget)
{
  g_assert (DZL_IS_SHORTCUT_MANAGER (self));
  g_assert (event != NULL);
  g_assert (chord != NULL);
  g_assert (phase == DZL_SHORTCUT_PHASE_CAPTURE ||
            phase == DZL_SHORTCUT_PHASE_BUBBLE);
  g_assert (DZL_IS_SHORTCUT_CONTROLLER (root));
  g_assert (GTK_WIDGET (widget));

  /*
   * The goal of this function is to locate a shortcut within any
   * controller registered with the root controller (or the root
   * controller itself) that is registered as a "global shortcut".
   */

  phase |= DZL_SHORTCUT_PHASE_GLOBAL;

  return _dzl_shortcut_controller_handle (root, event, chord, phase, widget);
}

static gboolean
dzl_shortcut_manager_run_fallbacks (DzlShortcutManager     *self,
                                    GtkWidget              *widget,
                                    GtkWidget              *toplevel,
                                    const DzlShortcutChord *chord)
{
  DzlShortcutManagerPrivate *priv = dzl_shortcut_manager_get_instance_private (self);
  static DzlShortcutChord *inspector_chord;

  DZL_ENTRY;

  g_assert (DZL_IS_SHORTCUT_MANAGER (self));
  g_assert (GTK_IS_WIDGET (widget));
  g_assert (GTK_IS_WIDGET (toplevel));
  g_assert (chord != NULL);

  if (dzl_shortcut_chord_get_length (chord) == 1)
    {
      GApplication *app = g_application_get_default ();
      const gchar *action;
      GdkModifierType state;
      guint keyval;

      dzl_shortcut_chord_get_nth_key (chord, 0, &keyval, &state);

      /* Special case shift-tab, which is shown as ISO_Left_Tab when
       * we converted into a Dazzle chord.
       */
      if (keyval == GDK_KEY_ISO_Left_Tab && state == 0)
        {
          if (gtk_bindings_activate (G_OBJECT (toplevel), keyval, GDK_SHIFT_MASK))
            DZL_RETURN (TRUE);
        }

      /* See if the toplevel activates this, like Tab, etc */
      if (gtk_bindings_activate (G_OBJECT (toplevel), keyval, state))
        DZL_RETURN (TRUE);

      /* See if there is a mnemonic active that should be activated */
      if (GTK_IS_WINDOW (toplevel) &&
          gtk_window_mnemonic_activate (GTK_WINDOW (toplevel), keyval, state))
        DZL_RETURN (TRUE);

      /*
       * See if we have something defined for this theme that
       * can be activated directly.
       */
      action = _dzl_shortcut_theme_lookup_action (priv->internal_theme, chord);

      if (action != NULL)
        {
          g_autofree gchar *prefix = NULL;
          g_autofree gchar *name = NULL;
          g_autoptr(GVariant) target = NULL;

          dzl_g_action_name_parse_full (action, &prefix, &name, &target);

          if (dzl_gtk_widget_action (toplevel, prefix, name, target))
            DZL_RETURN (TRUE);
        }

      /*
       * If we this is the ctrl+shift+d keybinding to activate the inspector,
       * then try to see if we should handle that manually.
       */
      if G_UNLIKELY (inspector_chord == NULL)
        inspector_chord = dzl_shortcut_chord_new_from_string ("<ctrl><shift>d");
      if (dzl_shortcut_chord_equal (chord, inspector_chord))
        {
          g_autoptr(GSettings) settings = g_settings_new ("org.gtk.Settings.Debug");

          if (g_settings_get_boolean (settings, "enable-inspector-keybinding"))
            {
              gtk_window_set_interactive_debugging (TRUE);
              DZL_RETURN (TRUE);
            }
        }

      /*
       * Now fallback to trying to activate the action within GtkApplication
       * as the legacy Gtk bindings would do.
       */

      if (GTK_IS_APPLICATION (app))
        {
          g_autofree gchar *accel = dzl_shortcut_chord_to_string (chord);
          g_auto(GStrv) actions = NULL;

          actions = gtk_application_get_actions_for_accel (GTK_APPLICATION (app), accel);

          if (actions != NULL)
            {
              for (guint i = 0; actions[i] != NULL; i++)
                {
                  g_autofree gchar *prefix = NULL;
                  g_autofree gchar *name = NULL;
                  g_autoptr(GVariant) param = NULL;

                  action = actions[i];

                  if (!dzl_g_action_name_parse_full (action, &prefix, &name, &param))
                    {
                      g_warning ("Failed to parse: %s", action);
                      continue;
                    }

                  if (dzl_gtk_widget_action (widget, prefix, name, param))
                    DZL_RETURN (TRUE);
                }
            }
        }
    }

  DZL_RETURN (FALSE);
}

/**
 * dzl_shortcut_manager_handle_event:
 * @self: (nullable): An #DzlShortcutManager
 * @toplevel: A #GtkWidget or %NULL.
 * @event: A #GdkEventKey event to handle.
 *
 * This function will try to dispatch @event to the proper widget and
 * #DzlShortcutContext. If the event is handled, then %TRUE is returned.
 *
 * You should call this from #GtkWidget::key-press-event handler in your
 * #GtkWindow toplevel.
 *
 * Returns: %TRUE if the event was handled.
 */
gboolean
dzl_shortcut_manager_handle_event (DzlShortcutManager *self,
                                   const GdkEventKey  *event,
                                   GtkWidget          *toplevel)
{
  g_autoptr(DzlShortcutChord) chord = NULL;
  DzlShortcutController *root;
  DzlShortcutMatch match;
  GtkWidget *widget;
  GtkWidget *focus;
  gboolean ret = GDK_EVENT_PROPAGATE;

  DZL_ENTRY;

  g_return_val_if_fail (!self || DZL_IS_SHORTCUT_MANAGER (self), FALSE);
  g_return_val_if_fail (!toplevel || GTK_IS_WINDOW (toplevel), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  if (self == NULL)
    self = dzl_shortcut_manager_get_default ();

  /* We don't support anything but key-press */
  if (event->type != GDK_KEY_PRESS)
    DZL_RETURN (GDK_EVENT_PROPAGATE);

  /* We might need to discover our toplevel from the event */
  if (toplevel == NULL)
    {
      gpointer user_data;

      gdk_window_get_user_data (event->window, &user_data);
      g_return_val_if_fail (GTK_IS_WIDGET (user_data), FALSE);

      toplevel = gtk_widget_get_toplevel (user_data);
      g_return_val_if_fail (GTK_IS_WINDOW (toplevel), FALSE);
    }

  /* Sanitiy checks */
  g_assert (DZL_IS_SHORTCUT_MANAGER (self));
  g_assert (GTK_IS_WINDOW (toplevel));
  g_assert (event != NULL);

  /* Synthesize focus as the toplevel if there is none */
  widget = focus = gtk_window_get_focus (GTK_WINDOW (toplevel));
  if (widget == NULL)
    widget = focus = toplevel;

  /*
   * We want to push this event into the toplevel controller. If it
   * gives us back a chord, then we can try to dispatch that up/down
   * the controller tree.
   */
  root = dzl_shortcut_controller_find (toplevel);
  chord = _dzl_shortcut_controller_push (root, event);
  if (chord == NULL)
    DZL_RETURN (GDK_EVENT_PROPAGATE);

#ifdef DZL_ENABLE_TRACE
  {
    g_autofree gchar *str = dzl_shortcut_chord_to_string (chord);
    DZL_TRACE_MSG ("current chord: %s", str);
  }
#endif

  /*
   * Now we have our chord/event to dispatch to the individual controllers
   * on widgets. We can run through the phases to capture/dispatch/bubble.
   */
  if ((match = dzl_shortcut_manager_run_global (self, event, chord, DZL_SHORTCUT_PHASE_CAPTURE, root, widget)) ||
      (match = dzl_shortcut_manager_run_phase (self, event, chord, DZL_SHORTCUT_PHASE_CAPTURE, widget, focus)) ||
      (match = dzl_shortcut_manager_run_phase (self, event, chord, DZL_SHORTCUT_PHASE_DISPATCH, widget, focus)) ||
      (match = dzl_shortcut_manager_run_phase (self, event, chord, DZL_SHORTCUT_PHASE_BUBBLE, widget, focus)) ||
      (match = dzl_shortcut_manager_run_global (self, event, chord, DZL_SHORTCUT_PHASE_BUBBLE, root, widget)) ||
      (match = dzl_shortcut_manager_run_fallbacks (self, widget, toplevel, chord)))
    ret = GDK_EVENT_STOP;

  DZL_TRACE_MSG ("match = %d", match);

  /* No match, clear our current chord */
  if (match != DZL_SHORTCUT_MATCH_PARTIAL)
    _dzl_shortcut_controller_clear (root);

  DZL_RETURN (ret);
}

const gchar *
dzl_shortcut_manager_get_theme_name (DzlShortcutManager *self)
{
  DzlShortcutTheme *theme;

  g_return_val_if_fail (DZL_IS_SHORTCUT_MANAGER (self), NULL);

  theme = dzl_shortcut_manager_get_theme (self);

  g_return_val_if_fail (DZL_IS_SHORTCUT_THEME (theme), NULL);

  return dzl_shortcut_theme_get_name (theme);
}

void
dzl_shortcut_manager_set_theme_name (DzlShortcutManager *self,
                                     const gchar        *name)
{
  DzlShortcutManagerPrivate *priv;

  if (self == NULL)
    self = dzl_shortcut_manager_get_default ();

  priv = dzl_shortcut_manager_get_instance_private (self);

  if (name == NULL)
    name = "default";

  for (guint i = 0; i < priv->themes->len; i++)
    {
      DzlShortcutTheme *theme = g_ptr_array_index (priv->themes, i);
      const gchar *theme_name = dzl_shortcut_theme_get_name (theme);

      if (g_strcmp0 (name, theme_name) == 0)
        {
          dzl_shortcut_manager_set_theme (self, theme);
          return;
        }
    }

  g_warning ("No such shortcut theme “%s”", name);
}

static guint
dzl_shortcut_manager_get_n_items (GListModel *model)
{
  DzlShortcutManager *self = (DzlShortcutManager *)model;
  DzlShortcutManagerPrivate *priv = dzl_shortcut_manager_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_SHORTCUT_MANAGER (self), 0);

  return priv->themes->len;
}

static GType
dzl_shortcut_manager_get_item_type (GListModel *model)
{
  return DZL_TYPE_SHORTCUT_THEME;
}

static gpointer
dzl_shortcut_manager_get_item (GListModel *model,
                               guint       position)
{
  DzlShortcutManager *self = (DzlShortcutManager *)model;
  DzlShortcutManagerPrivate *priv = dzl_shortcut_manager_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_SHORTCUT_MANAGER (self), NULL);
  g_return_val_if_fail (position < priv->themes->len, NULL);

  return g_object_ref (g_ptr_array_index (priv->themes, position));
}

static void
list_model_iface_init (GListModelInterface *iface)
{
  iface->get_n_items = dzl_shortcut_manager_get_n_items;
  iface->get_item_type = dzl_shortcut_manager_get_item_type;
  iface->get_item = dzl_shortcut_manager_get_item;
}

const gchar *
dzl_shortcut_manager_get_user_dir (DzlShortcutManager *self)
{
  DzlShortcutManagerPrivate *priv = dzl_shortcut_manager_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_SHORTCUT_MANAGER (self), NULL);

  if (priv->user_dir == NULL)
    {
      priv->user_dir = g_build_filename (g_get_user_data_dir (),
                                         g_get_prgname (),
                                         NULL);
    }

  return priv->user_dir;
}

void
dzl_shortcut_manager_set_user_dir (DzlShortcutManager *self,
                                   const gchar        *user_dir)
{
  DzlShortcutManagerPrivate *priv = dzl_shortcut_manager_get_instance_private (self);

  g_return_if_fail (DZL_IS_SHORTCUT_MANAGER (self));

  if (g_strcmp0 (user_dir, priv->user_dir) != 0)
    {
      g_free (priv->user_dir);
      priv->user_dir = g_strdup (user_dir);
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_USER_DIR]);
    }
}

void
dzl_shortcut_manager_remove_search_path (DzlShortcutManager *self,
                                         const gchar        *directory)
{
  DzlShortcutManagerPrivate *priv;

  g_return_if_fail (!self || DZL_IS_SHORTCUT_MANAGER (self));
  g_return_if_fail (directory != NULL);

  if (self == NULL)
    self = dzl_shortcut_manager_get_default ();

  priv = dzl_shortcut_manager_get_instance_private (self);

  for (GList *iter = priv->search_path.head; iter != NULL; iter = iter->next)
    {
      gchar *path = iter->data;

      if (g_strcmp0 (path, directory) == 0)
        {
          /* TODO: Remove any merged keybindings */

          g_queue_delete_link (&priv->search_path, iter);
          g_free (path);

          dzl_shortcut_manager_queue_reload (self);

          break;
        }
    }
}

void
dzl_shortcut_manager_append_search_path (DzlShortcutManager *self,
                                         const gchar        *directory)
{
  DzlShortcutManagerPrivate *priv;

  g_return_if_fail (!self || DZL_IS_SHORTCUT_MANAGER (self));
  g_return_if_fail (directory != NULL);

  if (self == NULL)
    self = dzl_shortcut_manager_get_default ();

  priv = dzl_shortcut_manager_get_instance_private (self);

  g_queue_push_tail (&priv->search_path, g_strdup (directory));

  dzl_shortcut_manager_queue_reload (self);
}

void
dzl_shortcut_manager_prepend_search_path (DzlShortcutManager *self,
                                          const gchar        *directory)
{
  DzlShortcutManagerPrivate *priv;

  g_return_if_fail (!self || DZL_IS_SHORTCUT_MANAGER (self));
  g_return_if_fail (directory != NULL);

  if (self == NULL)
    self = dzl_shortcut_manager_get_default ();

  priv = dzl_shortcut_manager_get_instance_private (self);

  g_queue_push_head (&priv->search_path, g_strdup (directory));

  dzl_shortcut_manager_queue_reload (self);
}

/**
 * dzl_shortcut_manager_get_search_path:
 * @self: A #DzlShortcutManager
 *
 * This function will get the list of search path entries. These are used to
 * load themes for the application. You should set this search path for
 * themes before calling g_initable_init() on the search manager.
 *
 * Returns: (transfer none) (element-type utf8): A #GList containing each of
 *   the search path items used to load shortcut themes.
 */
const GList *
dzl_shortcut_manager_get_search_path (DzlShortcutManager *self)
{
  DzlShortcutManagerPrivate *priv;

  if (self == NULL)
    self = dzl_shortcut_manager_get_default ();

  priv = dzl_shortcut_manager_get_instance_private (self);

  return priv->search_path.head;
}

static GNode *
dzl_shortcut_manager_find_child (DzlShortcutManager  *self,
                                 GNode               *parent,
                                 DzlShortcutNodeType  type,
                                 const gchar         *name)
{
  g_assert (DZL_IS_SHORTCUT_MANAGER (self));
  g_assert (parent != NULL);
  g_assert (type != 0);
  g_assert (name != NULL);

  for (GNode *iter = parent->children; iter != NULL; iter = iter->next)
    {
      DzlShortcutNodeData *data = iter->data;

      g_assert (DZL_IS_SHORTCUT_NODE_DATA (data));

      if (data->type == type && data->name == name)
        return iter;
    }

  return NULL;
}

static GNode *
dzl_shortcut_manager_get_group (DzlShortcutManager *self,
                                const gchar        *section,
                                const gchar        *group)
{
  DzlShortcutManagerPrivate *priv = dzl_shortcut_manager_get_instance_private (self);
  DzlShortcutNodeData *data;
  GNode *parent;
  GNode *node;

  g_assert (DZL_IS_SHORTCUT_MANAGER (self));
  g_assert (section != NULL);
  g_assert (group != NULL);

  node = dzl_shortcut_manager_find_child (self, priv->root, DZL_SHORTCUT_NODE_SECTION, section);

  if (node == NULL)
    {
      data = g_slice_new0 (DzlShortcutNodeData);
      data->magic = DZL_SHORTCUT_NODE_DATA_MAGIC;
      data->type = DZL_SHORTCUT_NODE_SECTION;
      data->name = g_intern_string (section);
      data->title = g_intern_string (section);
      data->subtitle = NULL;

      node = g_node_append_data (priv->root, data);
    }

  parent = node;

  node = dzl_shortcut_manager_find_child (self, parent, DZL_SHORTCUT_NODE_GROUP, group);

  if (node == NULL)
    {
      data = g_slice_new0 (DzlShortcutNodeData);
      data->magic = DZL_SHORTCUT_NODE_DATA_MAGIC;
      data->type = DZL_SHORTCUT_NODE_GROUP;
      data->name = g_intern_string (group);
      data->title = g_intern_string (group);
      data->subtitle = NULL;

      node = g_node_append_data (parent, data);
    }

  g_assert (node != NULL);
  g_assert (DZL_IS_SHORTCUT_NODE_DATA (node->data));

  return node;
}

void
dzl_shortcut_manager_add_action (DzlShortcutManager *self,
                                 const gchar        *detailed_action_name,
                                 const gchar        *section,
                                 const gchar        *group,
                                 const gchar        *title,
                                 const gchar        *subtitle)
{
  DzlShortcutManagerPrivate *priv;
  DzlShortcutNodeData *data;
  GNode *parent;

  g_return_if_fail (!self || DZL_IS_SHORTCUT_MANAGER (self));
  g_return_if_fail (detailed_action_name != NULL);
  g_return_if_fail (title != NULL);

  if (self == NULL)
    self = dzl_shortcut_manager_get_default ();

  priv = dzl_shortcut_manager_get_instance_private (self);

  section = g_intern_string (section);
  group = g_intern_string (group);
  title = g_intern_string (title);
  subtitle = g_intern_string (subtitle);

  parent = dzl_shortcut_manager_get_group (self, section, group);

  g_assert (parent != NULL);

  data = g_slice_new0 (DzlShortcutNodeData);
  data->magic = DZL_SHORTCUT_NODE_DATA_MAGIC;
  data->type = DZL_SHORTCUT_NODE_ACTION;
  data->name = g_intern_string (detailed_action_name);
  data->title = title;
  data->subtitle = subtitle;

  g_node_append_data (parent, data);

  g_hash_table_insert (priv->command_id_to_node_data, (gpointer)data->name, data);

  g_signal_emit (self, signals [CHANGED], 0);
}

void
dzl_shortcut_manager_add_command (DzlShortcutManager *self,
                                  const gchar        *command,
                                  const gchar        *section,
                                  const gchar        *group,
                                  const gchar        *title,
                                  const gchar        *subtitle)
{
  DzlShortcutManagerPrivate *priv;
  DzlShortcutNodeData *data;
  GNode *parent;

  g_return_if_fail (!self || DZL_IS_SHORTCUT_MANAGER (self));
  g_return_if_fail (command != NULL);
  g_return_if_fail (title != NULL);

  if (self == NULL)
    self = dzl_shortcut_manager_get_default ();

  priv = dzl_shortcut_manager_get_instance_private (self);

  section = g_intern_string (section);
  group = g_intern_string (group);
  title = g_intern_string (title);
  subtitle = g_intern_string (subtitle);

  parent = dzl_shortcut_manager_get_group (self, section, group);

  g_assert (parent != NULL);

  data = g_slice_new0 (DzlShortcutNodeData);
  data->magic = DZL_SHORTCUT_NODE_DATA_MAGIC;
  data->type = DZL_SHORTCUT_NODE_COMMAND;
  data->name = g_intern_string (command);
  data->title = title;
  data->subtitle = subtitle;

  g_node_append_data (parent, data);

  g_hash_table_insert (priv->command_id_to_node_data, (gpointer)data->name, data);

  g_signal_emit (self, signals [CHANGED], 0);
}

static DzlShortcutsShortcut *
create_shortcut (const DzlShortcutChord *chord,
                 const gchar            *title,
                 const gchar            *subtitle)
{
  g_autofree gchar *accel = dzl_shortcut_chord_to_string (chord);

  return g_object_new (DZL_TYPE_SHORTCUTS_SHORTCUT,
                       "accelerator", accel,
                       "subtitle", subtitle,
                       "title", title,
                       "visible", TRUE,
                       NULL);
}

/**
 * dzl_shortcut_manager_add_shortcuts_to_window:
 * @self: A #DzlShortcutManager
 * @window: A #DzlShortcutsWindow
 *
 * Adds shortcuts registered with the #DzlShortcutManager to the
 * #DzlShortcutsWindow.
 */
void
dzl_shortcut_manager_add_shortcuts_to_window (DzlShortcutManager *self,
                                              DzlShortcutsWindow *window)
{
  DzlShortcutManagerPrivate *priv;
  DzlShortcutTheme *theme;
  GNode *parent;

  g_return_if_fail (!self || DZL_IS_SHORTCUT_MANAGER (self));
  g_return_if_fail (DZL_IS_SHORTCUTS_WINDOW (window));

  if (self == NULL)
    self = dzl_shortcut_manager_get_default ();

  priv = dzl_shortcut_manager_get_instance_private (self);

  theme = dzl_shortcut_manager_get_theme (self);

  /*
   * The GNode tree is in four levels. priv->root is the root of the tree and
   * contains no data items itself. It is just our stable root. The children
   * of priv->root are our section nodes. Each section node has group nodes
   * as children. Finally, the shortcut nodes are the leaves.
   */

  parent = priv->root;

  for (const GNode *sections = parent->children; sections != NULL; sections = sections->next)
    {
      DzlShortcutNodeData *section_data = sections->data;
      DzlShortcutsSection *section;

      g_assert (DZL_IS_SHORTCUT_NODE_DATA (section_data));

      section = g_object_new (DZL_TYPE_SHORTCUTS_SECTION,
                              "title", section_data->title,
                              "section-name", section_data->title,
                              "visible", TRUE,
                              NULL);

      for (const GNode *groups = sections->children; groups != NULL; groups = groups->next)
        {
          DzlShortcutNodeData *group_data = groups->data;
          DzlShortcutsGroup *group;

          g_assert (DZL_IS_SHORTCUT_NODE_DATA (group_data));

          group = g_object_new (DZL_TYPE_SHORTCUTS_GROUP,
                                "title", group_data->title,
                                "visible", TRUE,
                                NULL);

          for (const GNode *iter = groups->children; iter != NULL; iter = iter->next)
            {
              DzlShortcutNodeData *data = iter->data;
              const DzlShortcutChord *chord = NULL;
              DzlShortcutsShortcut *shortcut;

              g_assert (DZL_IS_SHORTCUT_NODE_DATA (data));

              if (data->type == DZL_SHORTCUT_NODE_ACTION)
                chord = dzl_shortcut_theme_get_chord_for_action (theme, data->name);
              else if (data->type == DZL_SHORTCUT_NODE_COMMAND)
                chord = dzl_shortcut_theme_get_chord_for_command (theme, data->name);

              shortcut = create_shortcut (chord, data->title, data->subtitle);
              gtk_container_add (GTK_CONTAINER (group), GTK_WIDGET (shortcut));
            }

          gtk_container_add (GTK_CONTAINER (section), GTK_WIDGET (group));
        }

      gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (section));
    }
}

GNode *
_dzl_shortcut_manager_get_root (DzlShortcutManager *self)
{
  DzlShortcutManagerPrivate *priv = dzl_shortcut_manager_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_SHORTCUT_MANAGER (self), NULL);

  return priv->root;
}

/**
 * dzl_shortcut_manager_add_shortcut_entries:
 * @self: (nullable): a #DzlShortcutManager or %NULL for the default
 * @shortcuts: (array length=n_shortcuts): shortcuts to add
 * @n_shortcuts: the number of entries in @shortcuts
 * @translation_domain: (nullable): the gettext domain to use for translations
 *
 * This method will add @shortcuts to the #DzlShortcutManager.
 *
 * This provides a simple way for widgets to add their shortcuts to the manager
 * so that they may be overriden by themes or the end user.
 */
void
dzl_shortcut_manager_add_shortcut_entries (DzlShortcutManager     *self,
                                           const DzlShortcutEntry *shortcuts,
                                           guint                   n_shortcuts,
                                           const gchar            *translation_domain)
{
  DzlShortcutManagerPrivate *priv;

  g_return_if_fail (!self || DZL_IS_SHORTCUT_MANAGER (self));
  g_return_if_fail (shortcuts != NULL || n_shortcuts == 0);

  if (self == NULL)
    self = dzl_shortcut_manager_get_default ();

  priv = dzl_shortcut_manager_get_instance_private (self);

  /* Ignore duplicate calls with the same entries. This is out of convenience
   * to allow registering shortcuts from instance init (and thusly after the
   * GdkDisplay has been connected.
   */
  if (g_hash_table_contains (priv->seen_entries, shortcuts))
    return;

  g_hash_table_insert (priv->seen_entries, (gpointer)shortcuts, NULL);

  for (guint i = 0; i < n_shortcuts; i++)
    {
      const DzlShortcutEntry *entry = &shortcuts[i];

      if (entry->command == NULL)
        {
          g_warning ("Shortcut entry missing command id");
          continue;
        }

      if (entry->default_accel != NULL)
        dzl_shortcut_theme_set_accel_for_command (priv->internal_theme,
                                                  entry->command,
                                                  entry->default_accel,
                                                  entry->phase);

      dzl_shortcut_manager_add_command (self,
                                        entry->command,
                                        g_dgettext (translation_domain, entry->section),
                                        g_dgettext (translation_domain, entry->group),
                                        g_dgettext (translation_domain, entry->title),
                                        g_dgettext (translation_domain, entry->subtitle));
    }
}

/**
 * dzl_shortcut_manager_get_theme_by_name:
 * @self: a #DzlShortcutManager
 * @theme_name: (nullable): the name of a theme or %NULL of the internal theme
 *
 * Locates a theme by the name of the theme.
 *
 * If @theme_name is %NULL, then the internal theme is used. You probably dont
 * need to use that as it is used by various controllers to hook up their
 * default actions.
 *
 * Returns: (transfer none) (nullable): A #DzlShortcutTheme or %NULL.
 */
DzlShortcutTheme *
dzl_shortcut_manager_get_theme_by_name (DzlShortcutManager *self,
                                        const gchar        *theme_name)
{
  DzlShortcutManagerPrivate *priv = dzl_shortcut_manager_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_SHORTCUT_MANAGER (self), NULL);

  if (theme_name == NULL || g_strcmp0 (theme_name, "internal") == 0)
    return priv->internal_theme;

  for (guint i = 0; i < priv->themes->len; i++)
    {
      DzlShortcutTheme *theme = g_ptr_array_index (priv->themes, i);

      g_assert (DZL_IS_SHORTCUT_THEME (theme));

      if (g_strcmp0 (theme_name, dzl_shortcut_theme_get_name (theme)) == 0)
        return theme;
    }

  return NULL;
}

DzlShortcutTheme *
_dzl_shortcut_manager_get_internal_theme (DzlShortcutManager *self)
{
  DzlShortcutManagerPrivate *priv = dzl_shortcut_manager_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_SHORTCUT_MANAGER (self), NULL);

  return priv->internal_theme;
}

static void
dzl_shortcut_manager_merge (DzlShortcutManager *self,
                            DzlShortcutTheme   *theme)
{
  DzlShortcutManagerPrivate *priv = dzl_shortcut_manager_get_instance_private (self);
  g_autoptr(DzlShortcutTheme) alloc_layer = NULL;
  DzlShortcutTheme *base_layer;
  const gchar *name;

  DZL_ENTRY;

  g_return_if_fail (DZL_IS_SHORTCUT_MANAGER (self));
  g_return_if_fail (DZL_IS_SHORTCUT_THEME (theme));

  /*
   * One thing we are trying to avoid here is having separate code paths for
   * adding the "first theme modification" from merging additional layers from
   * plugins and the like. Having the same merge path in all situations
   * hopefully will help us avoid some bugs.
   */

  name = dzl_shortcut_theme_get_name (theme);

  if (dzl_str_empty0 (name))
    {
      g_warning ("Attempt to merge theme with empty name");
      DZL_EXIT;
    }

  base_layer = dzl_shortcut_manager_get_theme_by_name (self, name);

  if (base_layer == NULL)
    {
      const gchar *parent_name;
      const gchar *title;
      const gchar *subtitle;

      parent_name = dzl_shortcut_theme_get_parent_name (theme);
      title = dzl_shortcut_theme_get_title (theme);
      subtitle = dzl_shortcut_theme_get_subtitle (theme);

      alloc_layer = g_object_new (DZL_TYPE_SHORTCUT_THEME,
                                  "name", name,
                                  "parent-name", parent_name,
                                  "subtitle", subtitle,
                                  "title", title,
                                  NULL);

      base_layer = alloc_layer;

      /*
       * Now notify the GListModel consumers that our internal theme list
       * has changed to include the newly created base layer.
       */
      g_ptr_array_add (priv->themes, g_object_ref (alloc_layer));
      _dzl_shortcut_theme_set_manager (alloc_layer, self);
      g_list_model_items_changed (G_LIST_MODEL (self), priv->themes->len - 1, 0, 1);
    }

  /*
   * Okay, now we need to go through all the custom contexts, and global
   * shortcuts in the theme and merge them into the base_layer. However, we
   * will defer that work to the DzlShortcutTheme module so it has access to
   * the internal structures.
   */
  _dzl_shortcut_theme_merge (base_layer, theme);

  DZL_EXIT;
}

/**
 * _dzl_shortcut_manager_get_command_info:
 * @self: a #DzlShortcutManager
 * @command_id: the command-id
 * @title: (out) (optional): a location for the title
 * @subtitle: (out) (optional): a location for the subtitle
 *
 * Gets command information about command-id
 *
 * Returns: %TRUE if the command-id was found and out parameters were set.
 *
 * Since: 3.32
 */
gboolean
_dzl_shortcut_manager_get_command_info (DzlShortcutManager  *self,
                                        const gchar         *command_id,
                                        const gchar        **title,
                                        const gchar        **subtitle)
{
  DzlShortcutManagerPrivate *priv;
  DzlShortcutNodeData *node;

  if (self == NULL)
    self = dzl_shortcut_manager_get_default ();

  g_return_val_if_fail (DZL_IS_SHORTCUT_MANAGER (self), FALSE);

  priv = dzl_shortcut_manager_get_instance_private (self);

  if ((node = g_hash_table_lookup (priv->command_id_to_node_data, command_id)))
    {
      if (title != NULL)
        *title = node->title;

      if (subtitle != NULL)
        *subtitle = node->subtitle;

      return TRUE;
    }

  return FALSE;
}
