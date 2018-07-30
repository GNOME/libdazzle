/* dzl-preferences-page.c
 *
 * Copyright (C) 2015-2017 Christian Hergert <chergert@redhat.com>
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

#include "config.h"

#define G_LOG_DOMAIN "dzl-preferences-page"

#include <glib/gi18n.h>

#include "prefs/dzl-preferences-group.h"
#include "prefs/dzl-preferences-group-private.h"
#include "prefs/dzl-preferences-page.h"
#include "prefs/dzl-preferences-page-private.h"
#include "util/dzl-macros.h"

enum {
  PROP_0,
  PROP_PRIORITY,
  LAST_PROP
};

G_DEFINE_TYPE (DzlPreferencesPage, dzl_preferences_page, GTK_TYPE_BIN)

static GParamSpec *properties [LAST_PROP];

static void
dzl_preferences_page_finalize (GObject *object)
{
  DzlPreferencesPage *self = (DzlPreferencesPage *)object;

  g_clear_pointer (&self->groups_by_name, g_hash_table_unref);

  G_OBJECT_CLASS (dzl_preferences_page_parent_class)->finalize (object);
}

static void
dzl_preferences_page_get_property (GObject    *object,
                                   guint       prop_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  DzlPreferencesPage *self = DZL_PREFERENCES_PAGE (object);

  switch (prop_id)
    {
    case PROP_PRIORITY:
      g_value_set_int (value, self->priority);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_preferences_page_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  DzlPreferencesPage *self = DZL_PREFERENCES_PAGE (object);

  switch (prop_id)
    {
    case PROP_PRIORITY:
      self->priority = g_value_get_int (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_preferences_page_class_init (DzlPreferencesPageClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = dzl_preferences_page_finalize;
  object_class->get_property = dzl_preferences_page_get_property;
  object_class->set_property = dzl_preferences_page_set_property;

  properties [PROP_PRIORITY] =
    g_param_spec_int ("priority",
                      "Priority",
                      "Priority",
                      G_MININT,
                      G_MAXINT,
                      0,
                      (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, LAST_PROP, properties);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/dazzle/ui/dzl-preferences-page.ui");
  gtk_widget_class_bind_template_child (widget_class, DzlPreferencesPage, box);
}

static void
dzl_preferences_page_init (DzlPreferencesPage *self)
{
  self->groups_by_name = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

  gtk_widget_init_template (GTK_WIDGET (self));
}

void
dzl_preferences_page_add_group (DzlPreferencesPage  *self,
                                DzlPreferencesGroup *group)
{
  gchar *name = NULL;

  g_return_if_fail (DZL_IS_PREFERENCES_PAGE (self));
  g_return_if_fail (DZL_IS_PREFERENCES_GROUP (group));

  g_object_get (group, "name", &name, NULL);

  if (g_hash_table_contains (self->groups_by_name, name))
    {
      g_free (name);
      return;
    }

  g_hash_table_insert (self->groups_by_name, name, group);

  gtk_container_add_with_properties (GTK_CONTAINER (self->box), GTK_WIDGET (group),
                                     "priority", dzl_preferences_group_get_priority (group),
                                     NULL);
}

/**
 * dzl_preferences_page_get_group:
 *
 * Returns: (transfer none) (nullable): An #DzlPreferencesGroup or %NULL.
 */
DzlPreferencesGroup *
dzl_preferences_page_get_group (DzlPreferencesPage *self,
                                const gchar        *name)
{
  g_return_val_if_fail (DZL_IS_PREFERENCES_PAGE (self), NULL);
  g_return_val_if_fail (name != NULL, NULL);

  return g_hash_table_lookup (self->groups_by_name, name);
}

void
dzl_preferences_page_set_map (DzlPreferencesPage *self,
                              GHashTable         *map)
{
  DzlPreferencesGroup *group;
  GHashTableIter iter;

  g_return_if_fail (DZL_IS_PREFERENCES_PAGE (self));

  g_hash_table_iter_init (&iter, self->groups_by_name);

  while (g_hash_table_iter_next (&iter, NULL, (gpointer *)&group))
    dzl_preferences_group_set_map (group, map);
}

void
dzl_preferences_page_refilter (DzlPreferencesPage *self,
                               DzlPatternSpec     *spec)
{
  DzlPreferencesGroup *group;
  GHashTableIter iter;
  guint count = 0;

  g_return_if_fail (DZL_IS_PREFERENCES_PAGE (self));

  g_hash_table_iter_init (&iter, self->groups_by_name);
  while (g_hash_table_iter_next (&iter, NULL, (gpointer *)&group))
    count += dzl_preferences_group_refilter (group, spec);
  gtk_widget_set_visible (GTK_WIDGET (self), count > 0);
}
