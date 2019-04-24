/* dzl-application.c
 *
 * Copyright (C) 2014-2017 Christian Hergert <chergert@redhat.com>
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

#define G_LOG_DOMAIN "dzl-application"

#include "config.h"

#include "app/dzl-application.h"
#include "util/dzl-macros.h"

/**
 * SECTION:dzl-application
 * @title: DzlApplication
 * @short_description: Application base class with goodies
 *
 * #DzlApplication is an extension of #GtkApplication with extra features to
 * integrate various libdazzle subsystems with your application. We suggest
 * subclassing #DzlApplication.
 *
 * The #DzlApplication class provides:
 *
 *  - Automatic menu merging including the "app-menu".
 *  - Automatic Icon loading based on resources-base-path.
 *  - Automatic theme tracking to load CSS variants based on user themes.
 *
 * The #DzlApplication class automatically manages loading alternate CSS based
 * on the active theme by tracking #GtkSettings:gtk-theme-name. Additionally,
 * it supports menu merging including the base "app-menu" as loaded by automatic
 * #GResources in #GApplication:resource-base-path. It will autom
 */

typedef struct
{
  /*
   * The theme manager is used to load CSS resources based on the
   * GtkSettings:gtk-theme-name property. We add plugin resource
   * paths to this to ensure that we load plugin CSS files too.
   */
  DzlThemeManager *theme_manager;

  /*
   * The menu manager deals with merging menu elements from multiple
   * GtkBuilder files containing <menu> elements. We use the resource
   * path to map to the merge_id in the hashtable.
   */
  DzlMenuManager *menu_manager;
  GHashTable *menu_merge_ids;

  /*
   * The shortcut manager can be used to autoload keyboard themes from
   * plugins or the application resources.
   */
  DzlShortcutManager *shortcut_manager;

  /*
   * Deferred resource loading. This can be used to call
   * dzl_application_add_resources() before ::startup() has been called. Upon
   * ::startup(), we'll apply these. If this is set to NULL, ::startup() has
   * already been called.
   */
  GPtrArray *deferred_resources;

} DzlApplicationPrivate;

enum {
  PROP_0,
  PROP_MENU_MANAGER,
  PROP_SHORTCUT_MANAGER,
  PROP_THEME_MANAGER,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

G_DEFINE_TYPE_WITH_PRIVATE (DzlApplication, dzl_application, GTK_TYPE_APPLICATION)

DzlApplication *
dzl_application_new (const gchar       *application_id,
                     GApplicationFlags  flags)
{
  g_return_val_if_fail (application_id == NULL || g_application_id_is_valid (application_id), NULL);

  return g_object_new (DZL_TYPE_APPLICATION,
                       "application-id", application_id,
                       "flags", flags,
                       NULL);
}

static void
dzl_application_real_add_resources (DzlApplication *self,
                                    const gchar    *resource_path)
{
  DzlApplicationPrivate *priv = dzl_application_get_instance_private (self);
  g_autoptr(GError) error = NULL;
  g_autofree gchar *menu_path = NULL;
  g_autofree gchar *keythemes_path = NULL;
  guint merge_id;

  g_assert (DZL_IS_APPLICATION (self));
  g_assert (resource_path != NULL);

  /* We use interned strings for hash table keys */
  resource_path = g_intern_string (resource_path);

  /*
   * Allow the theme manager to monitor the css/Adwaita.css or other themes
   * based on gtk-theme-name. The theme manager also loads icons.
   */
  dzl_theme_manager_add_resources (priv->theme_manager, resource_path);

  /*
   * If the resource path has a gtk/menus.ui file, we want to auto-load and
   * merge the menus.
   */
  menu_path = g_build_filename (resource_path, "gtk", "menus.ui", NULL);

  if (g_str_has_prefix (menu_path, "resource://"))
    merge_id = dzl_menu_manager_add_resource (priv->menu_manager, menu_path, &error);
  else
    merge_id = dzl_menu_manager_add_filename (priv->menu_manager, menu_path, &error);

  if (merge_id != 0)
    g_hash_table_insert (priv->menu_merge_ids, (gchar *)resource_path, GUINT_TO_POINTER (merge_id));

  if (error != NULL &&
      !(g_error_matches (error, G_RESOURCE_ERROR, G_RESOURCE_ERROR_NOT_FOUND) ||
        g_error_matches (error, G_FILE_ERROR, G_FILE_ERROR_NOENT)))
    g_warning ("%s", error->message);

  /*
   * Load any shortcut theme information from the plugin or application
   * resources. We always append so that the application resource dir is
   * loaded before any plugin paths.
   */
  keythemes_path = g_build_filename (resource_path, "shortcuts", NULL);
  dzl_shortcut_manager_append_search_path (priv->shortcut_manager, keythemes_path);
}

static void
dzl_application_real_remove_resources (DzlApplication *self,
                                       const gchar    *resource_path)
{
  DzlApplicationPrivate *priv = dzl_application_get_instance_private (self);
  g_autofree gchar *keythemes_path = NULL;
  guint merge_id;

  g_assert (DZL_IS_APPLICATION (self));
  g_assert (resource_path != NULL);

  /* We use interned strings for hash table lookups */
  resource_path = g_intern_string (resource_path);

  /* Remove any loaded CSS providers for @resource_path/css/. */
  dzl_theme_manager_remove_resources (priv->theme_manager, resource_path);

  /* Remove any merged menus from the @resource_path/gtk/menus.ui */
  merge_id = GPOINTER_TO_UINT (g_hash_table_lookup (priv->menu_merge_ids, resource_path));
  if (merge_id != 0)
    {
      if (g_hash_table_contains (priv->menu_merge_ids, resource_path))
        g_hash_table_remove (priv->menu_merge_ids, resource_path);
      dzl_menu_manager_remove (priv->menu_manager, merge_id);
    }

  /* Remove keythemes path from the shortcuts manager */
  keythemes_path = g_strjoin (NULL, "resource://", resource_path, "/shortcuts", NULL);
  dzl_shortcut_manager_remove_search_path (priv->shortcut_manager, keythemes_path);
}

static void
dzl_application_app_menu_items_changed (DzlApplication *self,
                                        guint           position,
                                        guint           removed,
                                        guint           added,
                                        GMenuModel     *model)
{
  g_assert (DZL_IS_APPLICATION (self));
  g_assert (G_IS_MENU_MODEL (model));

  /* Handle initial/spurious case up-front */
  if (removed == 0 && added == 0)
    return;

  /* If we are doing our initial add of items, then we should register this
   * model with the GtkApplication. We do this in a delayed fashion to avoid
   * setting it before it is necessary.
   */
  if (removed == 0 && added == g_menu_model_get_n_items (model))
    gtk_application_set_app_menu (GTK_APPLICATION (self), model);
}

static void
dzl_application_startup (GApplication *app)
{
  DzlApplication *self = (DzlApplication *)app;
  DzlApplicationPrivate *priv = dzl_application_get_instance_private (self);
  const gchar *resource_base_path;
  GMenu *app_menu;

  g_assert (DZL_IS_APPLICATION (self));

  G_APPLICATION_CLASS (dzl_application_parent_class)->startup (app);

  /*
   * We cannot register resources before chaining startup because
   * the GtkSettings and other plumbing will not yet be initialized.
   */

  /* Register our resources that are part of libdazzle. */
  DZL_APPLICATION_GET_CLASS (self)->add_resources (self, "resource:///org/gnome/dazzle/");

  /* Now register the application resources */
  if (NULL != (resource_base_path = g_application_get_resource_base_path (app)))
    {
      g_autofree gchar *resource_path = NULL;

      resource_path = g_strdup_printf ("resource://%s", resource_base_path);
      DZL_APPLICATION_GET_CLASS (self)->add_resources (self, resource_path);
    }

  /* If the application has "app-menu" defined in menus.ui, we want to
   * assign it to the application. If we used the base GtkApplication for
   * menus, this would be done for us. But since we are doing menu merging,
   * we need to do it manually.
   *
   * This is done via a signal callback so that we can avoid setting the
   * menu in cases where it is empty.
   */
  app_menu = dzl_menu_manager_get_menu_by_id (priv->menu_manager, "app-menu");
  g_signal_connect_object (app_menu,
                           "items-changed",
                           G_CALLBACK (dzl_application_app_menu_items_changed),
                           self,
                           G_CONNECT_SWAPPED);
  dzl_application_app_menu_items_changed (self, 0, 0,
                                          g_menu_model_get_n_items (G_MENU_MODEL (app_menu)),
                                          G_MENU_MODEL (app_menu));

  /*
   * Now apply our deferred resources.
   */
  for (guint i = 0; i < priv->deferred_resources->len; i++)
    {
      const gchar *path = g_ptr_array_index (priv->deferred_resources, i);
      DZL_APPLICATION_GET_CLASS (self)->add_resources (self, path);
    }
  g_clear_pointer (&priv->deferred_resources, g_ptr_array_unref);

  /*
   * Now force reload the keyboard shortcuts without defering to the main
   * loop or anything.
   */
  dzl_shortcut_manager_reload (priv->shortcut_manager, NULL);
}

static void
dzl_application_finalize (GObject *object)
{
  DzlApplication *self = (DzlApplication *)object;
  DzlApplicationPrivate *priv = dzl_application_get_instance_private (self);

  g_clear_pointer (&priv->deferred_resources, g_ptr_array_unref);
  g_clear_pointer (&priv->menu_merge_ids, g_hash_table_unref);
  g_clear_object (&priv->theme_manager);
  g_clear_object (&priv->menu_manager);
  g_clear_object (&priv->shortcut_manager);

  G_OBJECT_CLASS (dzl_application_parent_class)->finalize (object);
}

static void
dzl_application_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  DzlApplication *self = DZL_APPLICATION (object);

  switch (prop_id)
    {
    case PROP_MENU_MANAGER:
      g_value_set_object (value, dzl_application_get_menu_manager (self));
      break;

    case PROP_SHORTCUT_MANAGER:
      g_value_set_object (value, dzl_application_get_shortcut_manager (self));
      break;

    case PROP_THEME_MANAGER:
      g_value_set_object (value, dzl_application_get_theme_manager (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_application_class_init (DzlApplicationClass *klass)
{
  GApplicationClass *g_app_class = G_APPLICATION_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = dzl_application_finalize;
  object_class->get_property = dzl_application_get_property;

  g_app_class->startup = dzl_application_startup;

  klass->add_resources = dzl_application_real_add_resources;
  klass->remove_resources = dzl_application_real_remove_resources;

  properties [PROP_MENU_MANAGER] =
    g_param_spec_object ("menu-manager", NULL, NULL,
                         DZL_TYPE_MENU_MANAGER,
                         (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  properties [PROP_SHORTCUT_MANAGER] =
    g_param_spec_object ("shortcut-manager", NULL, NULL,
                         DZL_TYPE_SHORTCUT_MANAGER,
                         (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  properties [PROP_THEME_MANAGER] =
    g_param_spec_object ("theme-manager", NULL, NULL,
                         DZL_TYPE_THEME_MANAGER,
                         (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
dzl_application_init (DzlApplication *self)
{
  DzlApplicationPrivate *priv = dzl_application_get_instance_private (self);
  g_autoptr(GPropertyAction) shortcut_theme_action = NULL;

  g_application_set_default (G_APPLICATION (self));

  priv->deferred_resources = g_ptr_array_new ();
  priv->theme_manager = dzl_theme_manager_new ();
  priv->menu_manager = dzl_menu_manager_new ();
  priv->menu_merge_ids = g_hash_table_new (NULL, NULL);
  priv->shortcut_manager = g_object_ref (dzl_shortcut_manager_get_default ());

  shortcut_theme_action = g_property_action_new ("shortcut-theme", priv->shortcut_manager, "theme-name");
  g_action_map_add_action (G_ACTION_MAP (self), G_ACTION (shortcut_theme_action));
}

/**
 * dzl_application_get_menu_manager:
 * @self: a #DzlApplication
 *
 * Gets the menu manager for the application.
 *
 * Returns: (transfer none): A #DzlMenuManager
 */
DzlMenuManager *
dzl_application_get_menu_manager (DzlApplication *self)
{
  DzlApplicationPrivate *priv = dzl_application_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_APPLICATION (self), NULL);

  return priv->menu_manager;
}

/**
 * dzl_application_get_theme_manager:
 * @self: a #DzlApplication
 *
 * Get the theme manager for the application.
 *
 * Returns: (transfer none): A #DzlThemeManager
 */
DzlThemeManager *
dzl_application_get_theme_manager (DzlApplication *self)
{
  DzlApplicationPrivate *priv = dzl_application_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_APPLICATION (self), NULL);

  return priv->theme_manager;
}

/**
 * dzl_application_get_menu_by_id:
 * @self: a #DzlApplication
 * @menu_id: the id of the menu to locate
 *
 * Similar to gtk_application_get_menu_by_id() but takes into account
 * menu merging which could have occurred upon loading plugins.
 *
 * Returns: (transfer none): A #GMenu
 */
GMenu *
dzl_application_get_menu_by_id (DzlApplication *self,
                                const gchar    *menu_id)
{
  DzlApplicationPrivate *priv = dzl_application_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_APPLICATION (self), NULL);
  g_return_val_if_fail (menu_id != NULL, NULL);

  return dzl_menu_manager_get_menu_by_id (priv->menu_manager, menu_id);
}

/**
 * dzl_application_add_resources:
 * @self: a #DzlApplication
 * @resource_path: the location of the resources.
 *
 * This adds @resource_path to the list of "automatic resources".
 *
 * If @resource_path starts with "resource://", then the corresponding
 * #GResources path will be searched for resources. Otherwise, @resource_path
 * should be a path to a location on disk.
 *
 * The #DzlApplication will locate resources such as CSS themes, icons, and
 * keyboard shortcuts using @resource_path.
 */
void
dzl_application_add_resources (DzlApplication *self,
                               const gchar    *resource_path)
{
  DzlApplicationPrivate *priv = dzl_application_get_instance_private (self);

  g_return_if_fail (DZL_IS_APPLICATION (self));
  g_return_if_fail (resource_path != NULL);

  if (priv->deferred_resources != NULL)
    {
      g_ptr_array_add (priv->deferred_resources, (gpointer)g_intern_string (resource_path));
      return;
    }

  DZL_APPLICATION_GET_CLASS (self)->add_resources (self, resource_path);
}

/**
 * dzl_application_remove_resources:
 * @self: a #DzlApplication
 * @resource_path: the location of the resources.
 *
 * This attempts to undo as many side-effects as possible from a call to
 * dzl_application_add_resources().
 */
void
dzl_application_remove_resources (DzlApplication *self,
                                  const gchar    *resource_path)
{
  DzlApplicationPrivate *priv = dzl_application_get_instance_private (self);

  g_return_if_fail (DZL_IS_APPLICATION (self));
  g_return_if_fail (resource_path != NULL);

  if (priv->deferred_resources != NULL)
    {
      g_ptr_array_remove (priv->deferred_resources, (gpointer)g_intern_string (resource_path));
      return;
    }

  DZL_APPLICATION_GET_CLASS (self)->remove_resources (self, resource_path);
}

/**
 * dzl_application_get_shortcut_manager:
 * @self: a #DzlApplication
 *
 * Gets the #DzlShortcutManager for the application.
 *
 * Returns: (transfer none): A #DzlShortcutManager
 */
DzlShortcutManager *
dzl_application_get_shortcut_manager (DzlApplication *self)
{
  DzlApplicationPrivate *priv = dzl_application_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_APPLICATION (self), NULL);

  return priv->shortcut_manager;
}
