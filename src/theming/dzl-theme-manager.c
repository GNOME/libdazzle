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

#include <libpeas/peas.h>

#include "dzl-css-provider.h"
#include "dzl-theme-manager.h"

struct _DzlThemeManager
{
  GObject         parent_instance;

  GtkCssProvider *app_provider;
  GHashTable     *plugin_providers;
};

G_DEFINE_TYPE (DzlThemeManager, dzl_theme_manager, G_TYPE_OBJECT)

static void
provider_destroy_notify (gpointer data)
{
  GtkStyleProvider *provider = data;
  GdkScreen *screen = gdk_screen_get_default ();

  g_assert (GTK_IS_STYLE_PROVIDER (provider));
  g_assert (GDK_IS_SCREEN (screen));

  gtk_style_context_remove_provider_for_screen (screen, provider);
  g_object_unref (provider);
}

static void
dzl_theme_manager_load_plugin (DzlThemeManager *self,
                               PeasPluginInfo  *plugin_info,
                               PeasEngine      *engine)
{
  GtkCssProvider *provider;
  const gchar *module_name;
  GdkScreen *screen;
  gchar *path;

  g_assert (DZL_IS_THEME_MANAGER (self));
  g_assert (plugin_info != NULL);
  g_assert (PEAS_IS_ENGINE (engine));

  module_name = peas_plugin_info_get_module_name (plugin_info);
  screen = gdk_screen_get_default ();

  path = g_strdup_printf ("/org/gnome/builder/plugins/%s", module_name);
  provider = dzl_css_provider_new (path);
  gtk_style_context_add_provider_for_screen (screen, GTK_STYLE_PROVIDER (provider),
                                             GTK_STYLE_PROVIDER_PRIORITY_APPLICATION + 1);
  g_hash_table_insert (self->plugin_providers, g_strdup (module_name), provider);
  g_free (path);

  path = g_strdup_printf ("/org/gnome/builder/plugins/%s/icons/", module_name);
  gtk_icon_theme_add_resource_path (gtk_icon_theme_get_default (), path);
  g_free (path);
}

static void
dzl_theme_manager_unload_plugin (DzlThemeManager *self,
                                 PeasPluginInfo  *plugin_info,
                                 PeasEngine      *engine)
{
  g_assert (DZL_IS_THEME_MANAGER (self));
  g_assert (plugin_info != NULL);
  g_assert (PEAS_IS_ENGINE (engine));

  g_hash_table_remove (self->plugin_providers,
                       peas_plugin_info_get_module_name (plugin_info));

  /* XXX: No opposite to gtk_icon_theme_add_resource_path() */
}

static void
dzl_theme_manager_finalize (GObject *object)
{
  DzlThemeManager *self = (DzlThemeManager *)object;

  g_clear_pointer (&self->app_provider, provider_destroy_notify);
  g_clear_pointer (&self->plugin_providers, g_hash_table_unref);

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
  PeasEngine *engine = peas_engine_get_default ();
  GdkScreen *screen = gdk_screen_get_default ();
  GApplication *app = g_application_get_default ();
  const GList *list;

  g_assert (PEAS_IS_ENGINE (engine));
  g_assert (GDK_IS_SCREEN (screen));
  g_assert (G_IS_APPLICATION (app));

  self->plugin_providers = g_hash_table_new_full (g_str_hash, g_str_equal, g_free,
                                                  provider_destroy_notify);

  self->app_provider = dzl_css_provider_new (g_application_get_resource_base_path (app));
  gtk_style_context_add_provider_for_screen (screen,
                                             GTK_STYLE_PROVIDER (self->app_provider),
                                             GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  gtk_icon_theme_add_resource_path (gtk_icon_theme_get_default (),
                                    "/org/gnome/builder/icons/");

  g_signal_connect_object (engine,
                           "load-plugin",
                           G_CALLBACK (dzl_theme_manager_load_plugin),
                           self,
                           G_CONNECT_AFTER | G_CONNECT_SWAPPED);

  g_signal_connect_object (engine,
                           "unload-plugin",
                           G_CALLBACK (dzl_theme_manager_unload_plugin),
                           self,
                           G_CONNECT_SWAPPED);

  list = peas_engine_get_plugin_list (engine);

  for (; list != NULL; list = list->next)
    {
      PeasPluginInfo *plugin_info = list->data;

      if (peas_plugin_info_is_loaded (plugin_info))
        dzl_theme_manager_load_plugin (self, plugin_info, engine);
    }
}

DzlThemeManager *
dzl_theme_manager_new (void)
{
  return g_object_new (DZL_TYPE_THEME_MANAGER, NULL);
}
