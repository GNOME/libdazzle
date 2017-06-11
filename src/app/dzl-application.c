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

#include "app/dzl-application.h"

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

} DzlApplicationPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (DzlApplication, dzl_application, GTK_TYPE_APPLICATION)

static void
dzl_application_real_add_resource_path (DzlApplication *self,
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
  dzl_theme_manager_add_resource_path (priv->theme_manager, resource_path);

  /*
   * If the resource path has a gtk/menus.ui file, we want to auto-load and
   * merge the menus.
   */
  menu_path = g_build_filename (resource_path, "gtk", "menus.ui", NULL);
  merge_id = dzl_menu_manager_add_resource (priv->menu_manager, menu_path, &error);
  g_hash_table_insert (priv->menu_merge_ids, (gchar *)resource_path, GUINT_TO_POINTER (merge_id));
  if (error != NULL && !g_error_matches (error, G_RESOURCE_ERROR, G_RESOURCE_ERROR_NOT_FOUND))
    g_warning ("%s", error->message);

  /*
   * Load any shortcut theme information from the plugin or application
   * resources. We always append so that the application resource dir is
   * loaded before any plugin paths.
   */
  keythemes_path = g_build_filename (resource_path, "keythemes", NULL);
  dzl_shortcut_manager_append_search_path (priv->shortcut_manager, keythemes_path);
}

static void
dzl_application_real_remove_resource_path (DzlApplication *self,
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
  dzl_theme_manager_remove_resource_path (priv->theme_manager, resource_path);

  /* Remove any merged menus from the @resource_path/gtk/menus.ui */
  merge_id = GPOINTER_TO_UINT (g_hash_table_lookup (priv->menu_merge_ids, resource_path));
  if (merge_id != 0)
    dzl_menu_manager_remove (priv->menu_manager, merge_id);

  /* Remove keythemes path from the shortcuts manager */
  keythemes_path = g_build_filename (resource_path, "keythemes", NULL);
  dzl_shortcut_manager_remove_search_path (priv->shortcut_manager, keythemes_path);
}

static void
dzl_application_startup (GApplication *app)
{
  DzlApplication *self = (DzlApplication *)app;
  DzlApplicationPrivate *priv = dzl_application_get_instance_private (self);
  GMenu *app_menu;

  g_assert (DZL_IS_APPLICATION (self));

  priv->theme_manager = dzl_theme_manager_new ();
  priv->menu_manager = dzl_menu_manager_new ();
  priv->menu_merge_ids = g_hash_table_new (NULL, NULL);
  priv->shortcut_manager = g_object_ref (dzl_shortcut_manager_get_default ());

  G_APPLICATION_CLASS (dzl_application_parent_class)->startup (app);

  /*
   * We cannot register resources before chaining startup because
   * the GtkSettings and other plumbing will not yet be initialized.
   */
  dzl_application_add_resource_path (self, "/org/gnome/dazzle");
  dzl_application_add_resource_path (self, g_application_get_resource_base_path (app));
  app_menu = dzl_menu_manager_get_menu_by_id (priv->menu_manager, "app-menu");
  gtk_application_set_app_menu (GTK_APPLICATION (self), G_MENU_MODEL (app_menu));
}

static void
dzl_application_shutdown (GApplication *app)
{
  DzlApplication *self = (DzlApplication *)app;
  DzlApplicationPrivate *priv = dzl_application_get_instance_private (self);

  g_assert (DZL_IS_APPLICATION (self));

  G_APPLICATION_CLASS (dzl_application_parent_class)->shutdown (app);

  g_clear_pointer (&priv->menu_merge_ids, g_hash_table_unref);
  g_clear_object (&priv->theme_manager);
  g_clear_object (&priv->menu_manager);
  g_clear_object (&priv->shortcut_manager);
}

static void
dzl_application_class_init (DzlApplicationClass *klass)
{
  GApplicationClass *g_app_class = G_APPLICATION_CLASS (klass);

  g_app_class->startup = dzl_application_startup;
  g_app_class->shutdown = dzl_application_shutdown;

  klass->add_resource_path = dzl_application_real_add_resource_path;
  klass->remove_resource_path = dzl_application_real_remove_resource_path;
}

static void
dzl_application_init (DzlApplication *self)
{
  g_application_set_default (G_APPLICATION (self));
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
 * dzl_application_add_resource_path:
 * @self: a #DzlApplication
 * @resource_path: a path to a #GResources path
 *
 * This adds @resource_path to the list of automatic resources.
 *
 * The #DzlApplication will locate resources such as CSS themes, icons, and
 * keybindings.
 */
void
dzl_application_add_resource_path (DzlApplication *self,
                                   const gchar    *resource_path)
{
  g_return_if_fail (DZL_IS_APPLICATION (self));
  g_return_if_fail (resource_path != NULL);

  DZL_APPLICATION_GET_CLASS (self)->add_resource_path (self, resource_path);
}

/**
 * dzl_application_remove_resource_path:
 * @self: a #DzlApplication
 * @resource_path: a path to a #GResources path
 *
 * This removes @resource_path to the list of automatic resources. This
 * directory should have been previously registered with
 * dzl_application_add_resource_path().
 */
void
dzl_application_remove_resource_path (DzlApplication *self,
                                      const gchar    *resource_path)
{
  g_return_if_fail (DZL_IS_APPLICATION (self));
  g_return_if_fail (resource_path != NULL);

  DZL_APPLICATION_GET_CLASS (self)->remove_resource_path (self, resource_path);
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
