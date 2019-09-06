/* dzl-preferences-view.c
 *
 * Copyright (C) 2015 Christian Hergert <chergert@redhat.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
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

#define G_LOG_DOMAIN "dzl-preferences-view"

#include "config.h"

#include <glib/gi18n.h>

#include "prefs/dzl-preferences-file-chooser-button.h"
#include "prefs/dzl-preferences-font-button.h"
#include "prefs/dzl-preferences-group-private.h"
#include "prefs/dzl-preferences-page-private.h"
#include "prefs/dzl-preferences-spin-button.h"
#include "prefs/dzl-preferences-switch.h"
#include "prefs/dzl-preferences-view.h"
#include "util/dzl-util-private.h"

typedef struct
{
  GActionGroup          *actions;
  GSequence             *pages;
  GHashTable            *widgets;

  GtkScrolledWindow     *scroller;
  GtkStack              *page_stack;
  GtkStackSidebar       *page_stack_sidebar;
  GtkSearchEntry        *search_entry;
  GtkStack              *subpage_stack;
  GtkBox                *sidebar;
  GtkStackSwitcher      *top_stack_switcher;

  guint                  last_widget_id;

  guint                  use_sidebar : 1;
  guint                  show_search_entry : 1;
} DzlPreferencesViewPrivate;

typedef struct
{
  GtkWidget *widget;
  gulong     handler;
  guint      id;
} TrackedWidget;

static void dzl_preferences_iface_init (DzlPreferencesInterface *iface);

G_DEFINE_TYPE_WITH_CODE (DzlPreferencesView, dzl_preferences_view, GTK_TYPE_BIN,
                         G_ADD_PRIVATE (DzlPreferencesView)
                         G_IMPLEMENT_INTERFACE (DZL_TYPE_PREFERENCES, dzl_preferences_iface_init))

enum {
  PROP_0,
  PROP_USE_SIDEBAR,
  PROP_SHOW_SEARCH_ENTRY,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

static void
tracked_widget_free (gpointer data)
{
  TrackedWidget *tracked = data;

  if (tracked->widget != NULL)
    {
      dzl_clear_signal_handler (tracked->widget, &tracked->handler);
      tracked->widget = NULL;
    }

  tracked->handler = 0;
  tracked->id = 0;

  g_slice_free (TrackedWidget, tracked);
}

G_DEFINE_AUTOPTR_CLEANUP_FUNC (TrackedWidget, tracked_widget_free)

static void
dzl_preferences_view_track (DzlPreferencesView *self,
                            guint               id,
                            GtkWidget          *widget)
{
  DzlPreferencesViewPrivate *priv = dzl_preferences_view_get_instance_private (self);
  TrackedWidget *tracked;

  g_assert (DZL_IS_PREFERENCES_VIEW (self));
  g_assert (id > 0);
  g_assert (GTK_IS_WIDGET (widget));

  tracked = g_slice_new0 (TrackedWidget);
  tracked->widget = widget;
  tracked->id = id;
  tracked->handler = g_signal_connect (widget,
                                       "destroy",
                                       G_CALLBACK (gtk_widget_destroyed),
                                       &tracked->widget);

  g_hash_table_insert (priv->widgets, GUINT_TO_POINTER (id), tracked);
}

static void
dzl_preferences_view_refilter_cb (GtkWidget *widget,
                                  gpointer   user_data)
{
  DzlPreferencesPage *page = (DzlPreferencesPage *)widget;
  DzlPatternSpec *spec = user_data;

  g_assert (DZL_IS_PREFERENCES_PAGE (page));

  dzl_preferences_page_refilter (page, spec);
}

static void
dzl_preferences_view_refilter (DzlPreferencesView *self,
                               const gchar        *search_text)
{
  DzlPreferencesViewPrivate *priv = dzl_preferences_view_get_instance_private (self);
  DzlPatternSpec *spec = NULL;

  g_assert (DZL_IS_PREFERENCES_VIEW (self));

  if (!dzl_str_empty0 (search_text))
    spec = dzl_pattern_spec_new (search_text);

  gtk_container_foreach (GTK_CONTAINER (priv->page_stack),
                         dzl_preferences_view_refilter_cb,
                         spec);
  gtk_container_foreach (GTK_CONTAINER (priv->subpage_stack),
                         dzl_preferences_view_refilter_cb,
                         spec);

  g_clear_pointer (&spec, dzl_pattern_spec_unref);
}

static gint
sort_by_priority (gconstpointer a,
                  gconstpointer b,
                  gpointer      user_data)
{
  gint prioritya = 0;
  gint priorityb = 0;

  g_object_get ((gpointer)a, "priority", &prioritya, NULL);
  g_object_get ((gpointer)b, "priority", &priorityb, NULL);

  return prioritya - priorityb;
}

static void
dzl_preferences_view_notify_visible_child (DzlPreferencesView *self,
                                           GParamSpec         *pspec,
                                           GtkStack           *stack)
{
  DzlPreferencesViewPrivate *priv = dzl_preferences_view_get_instance_private (self);
  DzlPreferencesPage *page;
  GHashTableIter iter;
  gpointer value;

  g_assert (DZL_IS_PREFERENCES_VIEW (self));

  /* Short circuit if we are destroying everything */
  if (gtk_widget_in_destruction (GTK_WIDGET (self)))
    return;

  gtk_widget_hide (GTK_WIDGET (priv->subpage_stack));

  /*
   * If there are any selections in list groups, re-select it to cause
   * the subpage to potentially reappear.
   */

  if (NULL == (page = DZL_PREFERENCES_PAGE (gtk_stack_get_visible_child (stack))))
    return;

  g_hash_table_iter_init (&iter, page->groups_by_name);

  while (g_hash_table_iter_next (&iter, NULL, &value))
    {
      DzlPreferencesGroup *group = value;
      GtkSelectionMode mode = GTK_SELECTION_NONE;

      g_assert (DZL_IS_PREFERENCES_GROUP (group));

      if (!group->is_list)
        continue;

      g_object_get (group, "mode", &mode, NULL);

      if (mode == GTK_SELECTION_SINGLE)
        {
          GtkListBoxRow *selected;

          selected = gtk_list_box_get_selected_row (group->list_box);

          g_assert (!selected || GTK_IS_LIST_BOX_ROW (selected));

          if (selected != NULL && gtk_widget_activate (GTK_WIDGET (selected)))
            break;
        }
    }
}

static void
dzl_preferences_view_finalize (GObject *object)
{
  DzlPreferencesView *self = (DzlPreferencesView *)object;
  DzlPreferencesViewPrivate *priv = dzl_preferences_view_get_instance_private (self);

  g_clear_pointer (&priv->pages, g_sequence_free);
  g_clear_pointer (&priv->widgets, g_hash_table_unref);
  g_clear_object (&priv->actions);

  G_OBJECT_CLASS (dzl_preferences_view_parent_class)->finalize (object);
}

static void
dzl_preferences_view_get_property (GObject    *object,
                                   guint       prop_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  DzlPreferencesView *self = DZL_PREFERENCES_VIEW (object);

  switch (prop_id)
    {
    case PROP_USE_SIDEBAR:
      g_value_set_boolean (value, dzl_preferences_view_get_use_sidebar (self));
      break;

    case PROP_SHOW_SEARCH_ENTRY:
      g_value_set_boolean (value, dzl_preferences_view_get_show_search_entry (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_preferences_view_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  DzlPreferencesView *self = DZL_PREFERENCES_VIEW (object);

  switch (prop_id)
    {
    case PROP_USE_SIDEBAR:
      dzl_preferences_view_set_use_sidebar (self, g_value_get_boolean (value));
      break;

    case PROP_SHOW_SEARCH_ENTRY:
      dzl_preferences_view_set_show_search_entry (self, g_value_get_boolean (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_preferences_view_class_init (DzlPreferencesViewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = dzl_preferences_view_finalize;
  object_class->get_property = dzl_preferences_view_get_property;
  object_class->set_property = dzl_preferences_view_set_property;

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/dazzle/ui/dzl-preferences-view.ui");
  gtk_widget_class_set_css_name (widget_class, "dzlpreferencesview");
  gtk_widget_class_bind_template_child_private (widget_class, DzlPreferencesView, page_stack);
  gtk_widget_class_bind_template_child_private (widget_class, DzlPreferencesView, page_stack_sidebar);
  gtk_widget_class_bind_template_child_private (widget_class, DzlPreferencesView, scroller);
  gtk_widget_class_bind_template_child_private (widget_class, DzlPreferencesView, search_entry);
  gtk_widget_class_bind_template_child_private (widget_class, DzlPreferencesView, sidebar);
  gtk_widget_class_bind_template_child_private (widget_class, DzlPreferencesView, subpage_stack);
  gtk_widget_class_bind_template_child_private (widget_class, DzlPreferencesView, top_stack_switcher);

  properties [PROP_USE_SIDEBAR] =
    g_param_spec_boolean ("use-sidebar",
                          "Use Sidebar",
                          "Use Sidebar",
                          FALSE,
                          (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  properties [PROP_SHOW_SEARCH_ENTRY] =
    g_param_spec_boolean ("show-search-entry",
                          "Show SearchEntry",
                          "Show SearchEntry in the sidebar",
                          FALSE,
                          (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
go_back_activate (GSimpleAction *action,
                  GVariant      *parameter,
                  gpointer       user_data)
{
  DzlPreferencesView *self = user_data;
  DzlPreferencesViewPrivate *priv = dzl_preferences_view_get_instance_private (self);

  g_assert (DZL_IS_PREFERENCES_VIEW (self));

  gtk_widget_hide (GTK_WIDGET (priv->subpage_stack));
}

void
dzl_preferences_view_reapply_filter (DzlPreferencesView *self)
{
  DzlPreferencesViewPrivate *priv = dzl_preferences_view_get_instance_private (self);

  g_return_if_fail (DZL_IS_PREFERENCES_VIEW (self));

  dzl_preferences_view_refilter (self, gtk_entry_get_text (GTK_ENTRY (priv->search_entry)));
}

static void
dzl_preferences_view_search_entry_changed (DzlPreferencesView *self,
                                           GtkSearchEntry     *search_entry)
{
  g_assert (DZL_IS_PREFERENCES_VIEW (self));
  g_assert (GTK_IS_SEARCH_ENTRY (search_entry));

  dzl_preferences_view_reapply_filter (self);
}

static void
dzl_preferences_view_search_entry_stop_search (DzlPreferencesView *self,
                                               GtkSearchEntry     *search_entry)
{
  g_assert (DZL_IS_PREFERENCES_VIEW (self));
  g_assert (GTK_IS_SEARCH_ENTRY (search_entry));

  gtk_entry_set_text (GTK_ENTRY(search_entry), "");
}

static void
dzl_preferences_view_notify_subpage_stack_visible (DzlPreferencesView *self,
                                                   GParamSpec         *pspec,
                                                   GtkStack           *subpage_stack)
{
  DzlPreferencesViewPrivate *priv = dzl_preferences_view_get_instance_private (self);

  g_assert (DZL_IS_PREFERENCES_VIEW (self));
  g_assert (GTK_IS_STACK (subpage_stack));

  /*
   * Because the subpage stack can cause us to have a wider display than
   * the screen has, we need to allow scrolling. This can happen because
   * side-by-side we could be just a bit bigger than 1280px which is a
   * fairly common laptop screen size (especially under HiDPI).
   *
   * https://bugzilla.gnome.org/show_bug.cgi?id=772700
   */

  if (gtk_widget_get_visible (GTK_WIDGET (subpage_stack)))
    g_object_set (priv->scroller, "hscrollbar-policy", GTK_POLICY_AUTOMATIC, NULL);
  else
    g_object_set (priv->scroller, "hscrollbar-policy", GTK_POLICY_NEVER, NULL);
}

static void
dzl_preferences_view_init (DzlPreferencesView *self)
{
  DzlPreferencesViewPrivate *priv = dzl_preferences_view_get_instance_private (self);
  static const GActionEntry entries[] = {
    { "go-back", go_back_activate },
  };

  priv->use_sidebar = TRUE;
  priv->show_search_entry = TRUE;

  gtk_widget_init_template (GTK_WIDGET (self));

  g_signal_connect_object (priv->search_entry,
                           "changed",
                           G_CALLBACK (dzl_preferences_view_search_entry_changed),
                           self,
                           G_CONNECT_SWAPPED);

  g_signal_connect_object (priv->search_entry,
                           "stop-search",
                           G_CALLBACK (dzl_preferences_view_search_entry_stop_search),
                           self,
                           G_CONNECT_SWAPPED);

  g_signal_connect_object (priv->page_stack,
                           "notify::visible-child",
                           G_CALLBACK (dzl_preferences_view_notify_visible_child),
                           self,
                           G_CONNECT_SWAPPED);

  g_signal_connect_object (priv->subpage_stack,
                           "notify::visible",
                           G_CALLBACK (dzl_preferences_view_notify_subpage_stack_visible),
                           self,
                           G_CONNECT_SWAPPED);

  priv->pages = g_sequence_new (NULL);
  priv->widgets = g_hash_table_new_full (g_direct_hash, g_direct_equal,
                                         NULL, tracked_widget_free);

  priv->actions = G_ACTION_GROUP (g_simple_action_group_new ());
  g_action_map_add_action_entries (G_ACTION_MAP (priv->actions),
                                   entries, G_N_ELEMENTS (entries),
                                   self);
}

static GtkWidget *
dzl_preferences_view_get_page (DzlPreferencesView *self,
                               const gchar        *page_name)
{
  DzlPreferencesViewPrivate *priv = dzl_preferences_view_get_instance_private (self);

  g_assert (DZL_IS_PREFERENCES_VIEW (self));
  g_assert (page_name != NULL);

  if (strchr (page_name, '.') != NULL)
    return gtk_stack_get_child_by_name (priv->subpage_stack, page_name);
  else
    return gtk_stack_get_child_by_name (priv->page_stack, page_name);
}

static void
dzl_preferences_view_add_page (DzlPreferences *preferences,
                               const gchar    *page_name,
                               const gchar    *title,
                               gint            priority)
{
  DzlPreferencesView *self = (DzlPreferencesView *)preferences;
  DzlPreferencesViewPrivate *priv = dzl_preferences_view_get_instance_private (self);
  DzlPreferencesPage *page;
  GSequenceIter *iter;
  GtkStack *stack;
  gint position = -1;

  g_assert (DZL_IS_PREFERENCES (preferences));
  g_assert (DZL_IS_PREFERENCES_VIEW (self));
  g_assert (page_name != NULL);
  g_assert (title != NULL || strchr (page_name, '.'));

  if (strchr (page_name, '.') != NULL)
    stack = priv->subpage_stack;
  else
    stack = priv->page_stack;

  if (gtk_stack_get_child_by_name (stack, page_name))
    return;

  page = g_object_new (DZL_TYPE_PREFERENCES_PAGE,
                       "name", page_name,
                       "priority", priority,
                       "visible", TRUE,
                       NULL);

  if (stack == priv->page_stack)
    {
      iter = g_sequence_insert_sorted (priv->pages, page, sort_by_priority, NULL);
      position = g_sequence_iter_get_position (iter);
    }

  gtk_container_add_with_properties (GTK_CONTAINER (stack), GTK_WIDGET (page),
                                     "position", position,
                                     "name", page_name,
                                     "title", title,
                                     NULL);
}

static void
dzl_preferences_view_add_group (DzlPreferences *preferences,
                                const gchar    *page_name,
                                const gchar    *group_name,
                                const gchar    *title,
                                gint            priority)
{
  DzlPreferencesView *self = (DzlPreferencesView *)preferences;
  DzlPreferencesGroup *group;
  GtkWidget *page;

  g_assert (DZL_IS_PREFERENCES_VIEW (self));
  g_assert (page_name != NULL);
  g_assert (group_name != NULL);

  page = dzl_preferences_view_get_page (self, page_name);

  if (page == NULL)
    {
      g_warning ("No page named \"%s\" could be found.", page_name);
      return;
    }

  group = g_object_new (DZL_TYPE_PREFERENCES_GROUP,
                        "name", group_name,
                        "priority", priority,
                        "title", title,
                        "visible", TRUE,
                        NULL);
  dzl_preferences_page_add_group (DZL_PREFERENCES_PAGE (page), group);
}

static void
dzl_preferences_view_add_list_group (DzlPreferences   *preferences,
                                     const gchar      *page_name,
                                     const gchar      *group_name,
                                     const gchar      *title,
                                     GtkSelectionMode  mode,
                                     gint              priority)
{
  DzlPreferencesView *self = (DzlPreferencesView *)preferences;
  DzlPreferencesGroup *group;
  GtkWidget *page;

  g_assert (DZL_IS_PREFERENCES_VIEW (self));
  g_assert (page_name != NULL);
  g_assert (group_name != NULL);

  page = dzl_preferences_view_get_page (self, page_name);

  if (page == NULL)
    {
      g_warning ("No page named \"%s\" could be found.", page_name);
      return;
    }

  group = g_object_new (DZL_TYPE_PREFERENCES_GROUP,
                        "is-list", TRUE,
                        "mode", mode,
                        "name", group_name,
                        "priority", priority,
                        "title", title,
                        "visible", TRUE,
                        NULL);
  dzl_preferences_page_add_group (DZL_PREFERENCES_PAGE (page), group);
}

static guint
dzl_preferences_view_add_radio (DzlPreferences *preferences,
                                const gchar    *page_name,
                                const gchar    *group_name,
                                const gchar    *schema_id,
                                const gchar    *key,
                                const gchar    *path,
                                const gchar    *variant_string,
                                const gchar    *title,
                                const gchar    *subtitle,
                                const gchar    *keywords,
                                gint            priority)
{
  DzlPreferencesView *self = (DzlPreferencesView *)preferences;
  DzlPreferencesViewPrivate *priv = dzl_preferences_view_get_instance_private (self);
  DzlPreferencesSwitch *widget;
  DzlPreferencesGroup *group;
  g_autoptr(GVariant) variant = NULL;
  GtkWidget *page;
  guint widget_id;

  g_assert (DZL_IS_PREFERENCES_VIEW (self));
  g_assert (page_name != NULL);
  g_assert (group_name != NULL);
  g_assert (schema_id != NULL);
  g_assert (key != NULL);
  g_assert (title != NULL);

  page = dzl_preferences_view_get_page (self, page_name);

  if (page == NULL)
    {
      g_warning ("No page named \"%s\" could be found.", page_name);
      return 0;
    }

  group = dzl_preferences_page_get_group (DZL_PREFERENCES_PAGE (page), group_name);

  if (group == NULL)
    {
      g_warning ("No such preferences group \"%s\" in page \"%s\"",
                 group_name, page_name);
      return 0;
    }

  if (variant_string != NULL)
    {
      g_autoptr(GError) error = NULL;

      variant = g_variant_parse (NULL, variant_string, NULL, NULL, &error);

      if (variant == NULL)
        g_warning ("%s", error->message);
    }

  widget = g_object_new (DZL_TYPE_PREFERENCES_SWITCH,
                         "is-radio", TRUE,
                         "key", key,
                         "keywords", keywords,
                         "path", path,
                         "priority", priority,
                         "schema-id", schema_id,
                         "subtitle", subtitle,
                         "target", variant,
                         "title", title,
                         "visible", TRUE,
                         NULL);

  dzl_preferences_group_add (group, GTK_WIDGET (widget));

  widget_id = ++priv->last_widget_id;
  dzl_preferences_view_track (self, widget_id, GTK_WIDGET (widget));

  return widget_id;
}

static guint
dzl_preferences_view_add_switch (DzlPreferences *preferences,
                                 const gchar    *page_name,
                                 const gchar    *group_name,
                                 const gchar    *schema_id,
                                 const gchar    *key,
                                 const gchar    *path,
                                 const gchar    *variant_string,
                                 const gchar    *title,
                                 const gchar    *subtitle,
                                 const gchar    *keywords,
                                 gint            priority)
{
  DzlPreferencesView *self = (DzlPreferencesView *)preferences;
  DzlPreferencesViewPrivate *priv = dzl_preferences_view_get_instance_private (self);
  DzlPreferencesSwitch *widget;
  DzlPreferencesGroup *group;
  g_autoptr(GVariant) variant = NULL;
  GtkWidget *page;
  guint widget_id;

  g_assert (DZL_IS_PREFERENCES_VIEW (self));
  g_assert (page_name != NULL);
  g_assert (group_name != NULL);
  g_assert (schema_id != NULL);
  g_assert (key != NULL);
  g_assert (title != NULL);

  page = dzl_preferences_view_get_page (self, page_name);

  if (page == NULL)
    {
      g_warning ("No page named \"%s\" could be found.", page_name);
      return 0;
    }

  group = dzl_preferences_page_get_group (DZL_PREFERENCES_PAGE (page), group_name);

  if (group == NULL)
    {
      g_warning ("No such preferences group \"%s\" in page \"%s\"",
                 group_name, page_name);
      return 0;
    }

  if (variant_string != NULL)
    {
      g_autoptr(GError) error = NULL;

      variant = g_variant_parse (NULL, variant_string, NULL, NULL, &error);

      if (variant == NULL)
        g_warning ("%s", error->message);
    }

  widget = g_object_new (DZL_TYPE_PREFERENCES_SWITCH,
                         "key", key,
                         "keywords", keywords,
                         "path", path,
                         "priority", priority,
                         "schema-id", schema_id,
                         "subtitle", subtitle,
                         "target", variant,
                         "title", title,
                         "visible", TRUE,
                         NULL);

  dzl_preferences_group_add (group, GTK_WIDGET (widget));

  widget_id = ++priv->last_widget_id;
  dzl_preferences_view_track (self, widget_id, GTK_WIDGET (widget));

  return widget_id;
}

static guint
dzl_preferences_view_add_spin_button (DzlPreferences *preferences,
                                      const gchar    *page_name,
                                      const gchar    *group_name,
                                      const gchar    *schema_id,
                                      const gchar    *key,
                                      const gchar    *path,
                                      const gchar    *title,
                                      const gchar    *subtitle,
                                      const gchar    *keywords,
                                      gint            priority)
{
  DzlPreferencesView *self = (DzlPreferencesView *)preferences;
  DzlPreferencesViewPrivate *priv = dzl_preferences_view_get_instance_private (self);
  DzlPreferencesSpinButton *widget;
  DzlPreferencesGroup *group;
  GtkWidget *page;
  guint widget_id;

  g_assert (DZL_IS_PREFERENCES_VIEW (self));
  g_assert (page_name != NULL);
  g_assert (group_name != NULL);
  g_assert (schema_id != NULL);
  g_assert (key != NULL);
  g_assert (title != NULL);

  page = dzl_preferences_view_get_page (self, page_name);

  if (page == NULL)
    {
      g_warning ("No page named \"%s\" could be found.", page_name);
      return 0;
    }

  group = dzl_preferences_page_get_group (DZL_PREFERENCES_PAGE (page), group_name);


  if (group == NULL)
    {
      g_warning ("No such preferences group \"%s\" in page \"%s\"",
                 group_name, page_name);
      return 0;
    }

  widget = g_object_new (DZL_TYPE_PREFERENCES_SPIN_BUTTON,
                         "key", key,
                         "keywords", keywords,
                         "path", path,
                         "priority", priority,
                         "schema-id", schema_id,
                         "subtitle", subtitle,
                         "title", title,
                         "visible", TRUE,
                         NULL);

  dzl_preferences_group_add (group, GTK_WIDGET (widget));

  widget_id = ++priv->last_widget_id;
  dzl_preferences_view_track (self, widget_id, GTK_WIDGET (widget));

  return widget_id;
}

static guint
dzl_preferences_view_add_font_button (DzlPreferences *preferences,
                                      const gchar    *page_name,
                                      const gchar    *group_name,
                                      const gchar    *schema_id,
                                      const gchar    *key,
                                      const gchar    *title,
                                      const gchar    *keywords,
                                      gint            priority)
{
  DzlPreferencesView *self = (DzlPreferencesView *)preferences;
  DzlPreferencesViewPrivate *priv = dzl_preferences_view_get_instance_private (self);
  DzlPreferencesSwitch *widget;
  DzlPreferencesGroup *group;
  GtkWidget *page;
  guint widget_id;

  g_assert (DZL_IS_PREFERENCES_VIEW (self));
  g_assert (page_name != NULL);
  g_assert (group_name != NULL);
  g_assert (schema_id != NULL);
  g_assert (key != NULL);
  g_assert (title != NULL);

  page = dzl_preferences_view_get_page (self, page_name);

  if (page == NULL)
    {
      g_warning ("No page named \"%s\" could be found.", page_name);
      return 0;
    }

  group = dzl_preferences_page_get_group (DZL_PREFERENCES_PAGE (page), group_name);

  if (group == NULL)
    {
      g_warning ("No such preferences group \"%s\" in page \"%s\"",
                 group_name, page_name);
      return 0;
    }

  widget = g_object_new (DZL_TYPE_PREFERENCES_FONT_BUTTON,
                         "key", key,
                         "keywords", keywords,
                         "priority", priority,
                         "schema-id", schema_id,
                         "title", title,
                         "visible", TRUE,
                         NULL);

  dzl_preferences_group_add (group, GTK_WIDGET (widget));

  widget_id = ++priv->last_widget_id;
  dzl_preferences_view_track (self, widget_id, GTK_WIDGET (widget));

  return widget_id;
}

static guint
dzl_preferences_view_add_file_chooser (DzlPreferences       *preferences,
                                       const gchar          *page_name,
                                       const gchar          *group_name,
                                       const gchar          *schema_id,
                                       const gchar          *key,
                                       const gchar          *path,
                                       const gchar          *title,
                                       const gchar          *subtitle,
                                       GtkFileChooserAction  action,
                                       const gchar          *keywords,
                                       gint                  priority)
{
  DzlPreferencesView *self = (DzlPreferencesView *)preferences;
  DzlPreferencesViewPrivate *priv = dzl_preferences_view_get_instance_private (self);
  DzlPreferencesFileChooserButton *widget;
  DzlPreferencesGroup *group;
  GtkWidget *page;
  guint widget_id;

  g_assert (DZL_IS_PREFERENCES_VIEW (self));
  g_assert (page_name != NULL);
  g_assert (group_name != NULL);
  g_assert (schema_id != NULL);
  g_assert (key != NULL);
  g_assert (title != NULL);

  page = dzl_preferences_view_get_page (self, page_name);

  if (page == NULL)
    {
      g_warning ("No page named \"%s\" could be found.", page_name);
      return 0;
    }

  group = dzl_preferences_page_get_group (DZL_PREFERENCES_PAGE (page), group_name);

  if (group == NULL)
    {
      g_warning ("No such preferences group \"%s\" in page \"%s\"",
                 group_name, page_name);
      return 0;
    }

  widget = g_object_new (DZL_TYPE_PREFERENCES_FILE_CHOOSER_BUTTON,
                         "action", action,
                         "key", key,
                         "priority", priority,
                         "schema-id", schema_id,
                         "path", path,
                         "subtitle", subtitle,
                         "title", title,
                         "keywords", keywords,
                         "visible", TRUE,
                         NULL);

  dzl_preferences_group_add (group, GTK_WIDGET (widget));

  widget_id = ++priv->last_widget_id;
  dzl_preferences_view_track (self, widget_id, GTK_WIDGET (widget));

  return widget_id;
}

static guint
dzl_preferences_view_add_custom (DzlPreferences *preferences,
                                 const gchar    *page_name,
                                 const gchar    *group_name,
                                 GtkWidget      *widget,
                                 const gchar    *keywords,
                                 gint            priority)
{
  DzlPreferencesView *self = (DzlPreferencesView *)preferences;
  DzlPreferencesViewPrivate *priv = dzl_preferences_view_get_instance_private (self);
  DzlPreferencesBin *container;
  DzlPreferencesGroup *group;
  GtkWidget *page;
  guint widget_id;

  g_assert (DZL_IS_PREFERENCES_VIEW (self));
  g_assert (page_name != NULL);
  g_assert (group_name != NULL);
  g_assert (GTK_IS_WIDGET (widget));

  page = dzl_preferences_view_get_page (self, page_name);

  if (page == NULL)
    {
      g_warning ("No page named \"%s\" could be found.", page_name);
      return 0;
    }

  group = dzl_preferences_page_get_group (DZL_PREFERENCES_PAGE (page), group_name);

  if (group == NULL)
    {
      g_warning ("No such preferences group \"%s\" in page \"%s\"",
                 group_name, page_name);
      return 0;
    }

  widget_id = ++priv->last_widget_id;

  gtk_widget_show (widget);
  gtk_widget_show (GTK_WIDGET (group));

  if (DZL_IS_PREFERENCES_BIN (widget))
    container = DZL_PREFERENCES_BIN (widget);
  else
    container = g_object_new (DZL_TYPE_PREFERENCES_BIN,
                              "child", widget,
                              "keywords", keywords,
                              "priority", priority,
                              "visible", TRUE,
                              NULL);

  dzl_preferences_group_add (group, GTK_WIDGET (container));

  dzl_preferences_view_track (self, widget_id, GTK_WIDGET (widget));

  return widget_id;
}

static gboolean
dzl_preferences_view_remove_id (DzlPreferences *preferences,
                                guint           widget_id)
{
  DzlPreferencesView *self = (DzlPreferencesView *)preferences;
  DzlPreferencesViewPrivate *priv = dzl_preferences_view_get_instance_private (self);
  g_autoptr(TrackedWidget) tracked = NULL;

  g_assert (DZL_IS_PREFERENCES_VIEW (self));
  g_assert (widget_id != 0);

  tracked = g_hash_table_lookup (priv->widgets, GUINT_TO_POINTER (widget_id));

  if (tracked != NULL)
    {
      GtkWidget *widget = tracked->widget;

      /* We have to steal the structure so that we retain access to
       * the structure after removing it from the hashtable.
       */
      g_hash_table_steal (priv->widgets, GUINT_TO_POINTER (widget_id));

      if (widget != NULL && !gtk_widget_in_destruction (widget))
        {
          GtkWidget *parent = gtk_widget_get_ancestor (widget, GTK_TYPE_LIST_BOX_ROW);

          /* in case we added our own row ancestor, destroy it */
          if (parent != NULL && !gtk_widget_in_destruction (parent))
            gtk_widget_destroy (parent);
          else
            gtk_widget_destroy (widget);
        }

      return TRUE;
    }

  return FALSE;
}

static void
dzl_preferences_view_set_page (DzlPreferences *preferences,
                               const gchar    *page_name,
                               GHashTable     *map)
{
  DzlPreferencesView *self = (DzlPreferencesView *)preferences;
  DzlPreferencesViewPrivate *priv = dzl_preferences_view_get_instance_private (self);
  GtkWidget *page;

  g_assert (DZL_IS_PREFERENCES_VIEW (self));
  g_assert (page_name != NULL);

  page = dzl_preferences_view_get_page (self, page_name);

  if (page == NULL)
    {
      g_warning ("No such page \"%s\"", page_name);
      return;
    }

  if (strchr (page_name, '.') != NULL)
    {
      gtk_container_foreach (GTK_CONTAINER (priv->subpage_stack),
                             (GtkCallback)gtk_widget_hide,
                             NULL);
      dzl_preferences_page_set_map (DZL_PREFERENCES_PAGE (page), map);
      gtk_stack_set_visible_child (priv->subpage_stack, page);
      gtk_widget_show (page);
      gtk_widget_show (GTK_WIDGET (priv->subpage_stack));
    }
  else
    {
      gtk_stack_set_visible_child (priv->page_stack, page);
      gtk_widget_hide (GTK_WIDGET (priv->subpage_stack));
    }
}

static GtkWidget *
dzl_preferences_view_get_widget (DzlPreferences *preferences,
                                 guint           widget_id)
{
  DzlPreferencesView *self = (DzlPreferencesView *)preferences;
  DzlPreferencesViewPrivate *priv = dzl_preferences_view_get_instance_private (self);
  TrackedWidget *tracked;

  g_assert (DZL_IS_PREFERENCES_VIEW (self));

  tracked = g_hash_table_lookup (priv->widgets, GINT_TO_POINTER (widget_id));

  return tracked ? tracked->widget : NULL;
}

static guint
dzl_preferences_view_add_table_row_va (DzlPreferences *preferences,
                                       const gchar    *page_name,
                                       const gchar    *group_name,
                                       GtkWidget      *first_widget,
                                       va_list         args)
{
  DzlPreferencesView *self = (DzlPreferencesView *)preferences;
  DzlPreferencesViewPrivate *priv = dzl_preferences_view_get_instance_private (self);
  DzlPreferencesGroup *group;
  GtkWidget *page;
  GtkWidget *column = first_widget;
  GtkWidget *row;
  GtkBox *box;
  guint column_id = 0;
  guint widget_id;

  g_assert (DZL_IS_PREFERENCES_VIEW (self));
  g_assert (page_name != NULL);
  g_assert (group_name != NULL);
  g_assert (GTK_IS_WIDGET (column));

  page = dzl_preferences_view_get_page (self, page_name);

  if (page == NULL)
    {
      g_warning ("No page named \"%s\" could be found.", page_name);
      return 0;
    }

  group = dzl_preferences_page_get_group (DZL_PREFERENCES_PAGE (page), group_name);

  if (group == NULL)
    {
      g_warning ("No such preferences group \"%s\" in page \"%s\"",
                 group_name, page_name);
      return 0;
    }

  row = g_object_new (DZL_TYPE_PREFERENCES_BIN,
                      "visible", TRUE,
                      NULL);
  box = g_object_new (GTK_TYPE_BOX,
                      "orientation", GTK_ORIENTATION_HORIZONTAL,
                      "visible", TRUE,
                      NULL);
  gtk_container_add (GTK_CONTAINER (row), GTK_WIDGET (box));

  do
    {
      GtkSizeGroup *size_group;

      if ((size_group = dzl_preferences_group_get_size_group (group, column_id)))
        gtk_size_group_add_widget (size_group, column);

      gtk_container_add_with_properties (GTK_CONTAINER (box), column,
                                         "expand", FALSE,
                                         NULL);

      column = va_arg (args, GtkWidget*);
      column_id++;
    }
  while (column != NULL);

  dzl_preferences_group_add (group, GTK_WIDGET (row));

  widget_id = ++priv->last_widget_id;
  dzl_preferences_view_track (self, widget_id, GTK_WIDGET (row));

  if ((row = gtk_widget_get_ancestor (row, GTK_TYPE_LIST_BOX_ROW)))
    gtk_widget_set_can_focus (row, FALSE);

  return widget_id;
}

static void
dzl_preferences_iface_init (DzlPreferencesInterface *iface)
{
  iface->add_page = dzl_preferences_view_add_page;
  iface->add_group = dzl_preferences_view_add_group;
  iface->add_list_group  = dzl_preferences_view_add_list_group;
  iface->add_radio = dzl_preferences_view_add_radio;
  iface->add_font_button = dzl_preferences_view_add_font_button;
  iface->add_switch = dzl_preferences_view_add_switch;
  iface->add_spin_button = dzl_preferences_view_add_spin_button;
  iface->add_file_chooser = dzl_preferences_view_add_file_chooser;
  iface->add_custom = dzl_preferences_view_add_custom;
  iface->set_page = dzl_preferences_view_set_page;
  iface->remove_id = dzl_preferences_view_remove_id;
  iface->get_widget = dzl_preferences_view_get_widget;
  iface->add_table_row_va = dzl_preferences_view_add_table_row_va;
}

GtkWidget *
dzl_preferences_view_new (void)
{
  return g_object_new (DZL_TYPE_PREFERENCES_VIEW, NULL);
}

gboolean
dzl_preferences_view_get_use_sidebar (DzlPreferencesView *self)
{
  DzlPreferencesViewPrivate *priv = dzl_preferences_view_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_PREFERENCES_VIEW (self), FALSE);

  return priv->use_sidebar;
}

void
dzl_preferences_view_set_use_sidebar (DzlPreferencesView *self,
                                      gboolean            use_sidebar)
{
  DzlPreferencesViewPrivate *priv = dzl_preferences_view_get_instance_private (self);

  g_return_if_fail (DZL_IS_PREFERENCES_VIEW (self));

  use_sidebar = !!use_sidebar;

  if (priv->use_sidebar != use_sidebar)
    {
      priv->use_sidebar = use_sidebar;

      gtk_widget_set_visible (GTK_WIDGET (priv->sidebar), use_sidebar);
      gtk_widget_set_visible (GTK_WIDGET (priv->top_stack_switcher), !use_sidebar);

      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_USE_SIDEBAR]);
    }
}

gboolean
dzl_preferences_view_get_show_search_entry (DzlPreferencesView *self)
{
  DzlPreferencesViewPrivate *priv = dzl_preferences_view_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_PREFERENCES_VIEW (self), FALSE);

  return priv->show_search_entry;
}

void
dzl_preferences_view_set_show_search_entry (DzlPreferencesView *self,
                                            gboolean            show_search_entry)
{
  DzlPreferencesViewPrivate *priv = dzl_preferences_view_get_instance_private (self);

  g_return_if_fail (DZL_IS_PREFERENCES_VIEW (self));

  if (!dzl_preferences_view_get_use_sidebar (self))
    return;

  if (priv->show_search_entry != show_search_entry)
    {
      priv->show_search_entry = show_search_entry;

      gtk_widget_set_visible (GTK_WIDGET (priv->search_entry), show_search_entry);

      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_SHOW_SEARCH_ENTRY]);
    }
}
