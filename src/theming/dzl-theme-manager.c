/* dzl-theme-manager.c
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

#define G_LOG_DOMAIN "dzl-theme-manager"

#include "config.h"

#include <string.h>

#include "theming/dzl-css-provider.h"
#include "theming/dzl-theme-manager.h"
#include "util/dzl-macros.h"

struct _DzlThemeManager
{
  GObject     parent_instance;
  GHashTable *providers_by_path;
};

G_DEFINE_TYPE (DzlThemeManager, dzl_theme_manager, G_TYPE_OBJECT)

static void
dzl_theme_manager_finalize (GObject *object)
{
  DzlThemeManager *self = (DzlThemeManager *)object;

  g_clear_pointer (&self->providers_by_path, g_hash_table_unref);

  G_OBJECT_CLASS (dzl_theme_manager_parent_class)->finalize (object);
}

static void
dzl_theme_manager_class_init (DzlThemeManagerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = dzl_theme_manager_finalize;
}

static void
dzl_theme_manager_init (DzlThemeManager *self)
{
  self->providers_by_path = g_hash_table_new_full (g_str_hash,
                                                   g_str_equal,
                                                   g_free,
                                                   g_object_unref);
}

DzlThemeManager *
dzl_theme_manager_new (void)
{
  return g_object_new (DZL_TYPE_THEME_MANAGER, NULL);
}

static gboolean
has_child_resources (const gchar *path)
{
  g_auto(GStrv) children = NULL;

  if (g_str_has_prefix (path, "resource://"))
    path += strlen ("resource://");

  children = g_resources_enumerate_children (path, 0, NULL);

  return children != NULL && children[0] != NULL;
}

/**
 * dzl_theme_manager_add_resources:
 * @self: a #DzlThemeManager
 * @resource_path: A path to a #GResources directory
 *
 * This will automatically register resources found within @resource_path.
 *
 * If @resource_path starts with "resource://", embedded #GResources will be
 * used to locate the theme files. Otherwise, @resource_path is expected to be
 * a path on disk that may or may not exist.
 *
 * If the @resource_path contains a directory named "themes", that directory
 * will be traversed for files matching the theme name and variant. For
 * example, if using the Adwaita theme, "themes/Adwaita.css" will be loaded. If
 * the dark variant is being used, "themes/Adwaita-dark.css" will be loaeded. If
 * no matching theme file is located, "themes/shared.css" will be loaded.
 *
 * When the current theme changes, the CSS will be reloaded to adapt.
 *
 * The "icons" sub-directory will be used to locate icon themes.
 */
void
dzl_theme_manager_add_resources (DzlThemeManager *self,
                                 const gchar     *resource_path)
{
  g_autoptr(GtkCssProvider) provider = NULL;
  g_autofree gchar *css_dir = NULL;
  g_autofree gchar *icons_dir = NULL;
  const gchar *real_path = resource_path;
  GtkIconTheme *theme;

  g_return_if_fail (DZL_IS_THEME_MANAGER (self));
  g_return_if_fail (resource_path != NULL);

  theme = gtk_icon_theme_get_default ();

  if (g_str_has_prefix (real_path, "resource://"))
    real_path += strlen ("resource://");

  /*
   * Create a CSS provider that will load the proper variant based on the
   * current application theme, using @resource_path/css as the base directory
   * to locate theming files.
   */
  css_dir = g_build_path ("/", resource_path, "themes/", NULL);
  g_debug ("Including CSS overrides from %s", css_dir);

  if (has_child_resources (css_dir))
    {
      provider = dzl_css_provider_new (css_dir);
      g_hash_table_insert (self->providers_by_path, g_strdup (resource_path), g_object_ref (provider));

      /* Use APPLICATION+1 to place ourselves higher than libraries that
       * incorrectly use APPLICATION as their priority. This allows the
       * application (whose themes we'll be loading) to have higher
       * priorities than libraries like VTE.
       */
      gtk_style_context_add_provider_for_screen (gdk_screen_get_default (),
                                                 GTK_STYLE_PROVIDER (provider),
                                                 GTK_STYLE_PROVIDER_PRIORITY_APPLICATION+1);
    }

  /*
   * Add the icons sub-directory so that Gtk can locate the themed
   * icons (svg, png, etc).
   */
  icons_dir = g_build_path ("/", real_path, "icons/", NULL);
  g_debug ("Loading icon resources from %s", icons_dir);
  if (!g_str_equal (real_path, resource_path))
    {
      g_auto(GStrv) children = NULL;

      /* Okay, this is a resource-based path. Make sure the
       * path contains children so we don't slow down the
       * theme loading code with tons of useless directories.
       */
      children = g_resources_enumerate_children (icons_dir, 0, NULL);
      if (children != NULL && children[0] != NULL)
        gtk_icon_theme_add_resource_path (theme, icons_dir);
    }
  else
    {
      /* Make sure the directory exists so that we don't needlessly
       * slow down the icon loading paths.
       */
      if (g_file_test (icons_dir, G_FILE_TEST_IS_DIR))
        gtk_icon_theme_append_search_path (theme, icons_dir);
    }
}

/**
 * dzl_theme_manager_remove_resources:
 * @self: a #DzlThemeManager
 * @resource_path: A previously registered resources path
 *
 * This removes the CSS providers that were registered using @resource_path.
 *
 * You must have previously called dzl_theme_manager_add_resources() for
 * this function to do anything.
 *
 * Since icons cannot be unloaded, previously loaded icons will continue to
 * be available even after calling this function.
 */
void
dzl_theme_manager_remove_resources (DzlThemeManager *self,
                                    const gchar     *resource_path)
{
  GtkStyleProvider *provider;

  g_return_if_fail (DZL_IS_THEME_MANAGER (self));
  g_return_if_fail (resource_path != NULL);

  if (NULL != (provider = g_hash_table_lookup (self->providers_by_path, resource_path)))
    {
      g_debug ("Removing CSS overrides from %s", resource_path);
      gtk_style_context_remove_provider_for_screen (gdk_screen_get_default (), provider);
      g_hash_table_remove (self->providers_by_path, resource_path);
    }
}
