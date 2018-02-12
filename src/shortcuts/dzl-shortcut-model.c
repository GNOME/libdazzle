/* dzl-shortcut-model.c
 *
 * Copyright (C) 2017 Christian Hergert <chergert@redhat.com>
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

#define G_LOG_DOMAIN "dzl-shortcut-model"

#include "config.h"

#include "shortcuts/dzl-shortcut-model.h"
#include "shortcuts/dzl-shortcut-private.h"

struct _DzlShortcutModel
{
  GtkTreeStore        parent_instance;
  DzlShortcutManager *manager;
  DzlShortcutTheme   *theme;
};

G_DEFINE_TYPE (DzlShortcutModel, dzl_shortcut_model, GTK_TYPE_TREE_STORE)

enum {
  PROP_0,
  PROP_MANAGER,
  PROP_THEME,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

void
dzl_shortcut_model_rebuild (DzlShortcutModel *self)
{
  g_assert (DZL_IS_SHORTCUT_MODEL (self));

  gtk_tree_store_clear (GTK_TREE_STORE (self));

  if (self->manager != NULL && self->theme != NULL)
    {
      GNode *root;

      root = _dzl_shortcut_manager_get_root (self->manager);

      for (const GNode *iter = root->children; iter != NULL; iter = iter->next)
        {
          for (const GNode *groups = iter->children; groups != NULL; groups = groups->next)
            {
              DzlShortcutNodeData *group = groups->data;
              GtkTreeIter p;

              gtk_tree_store_append (GTK_TREE_STORE (self), &p, NULL);
              gtk_tree_store_set (GTK_TREE_STORE (self), &p,
                                  DZL_SHORTCUT_MODEL_COLUMN_TITLE, group->title,
                                  -1);

              for (const GNode *sc = groups->children; sc != NULL; sc = sc->next)
                {
                  DzlShortcutNodeData *shortcut = sc->data;
                  const DzlShortcutChord *chord = NULL;
                  g_autofree gchar *accel = NULL;
                  g_autofree gchar *down = NULL;
                  GtkTreeIter p2;

                  if (shortcut->type == DZL_SHORTCUT_NODE_ACTION)
                    chord = dzl_shortcut_theme_get_chord_for_action (self->theme, shortcut->name);
                  else if (shortcut->type == DZL_SHORTCUT_NODE_COMMAND)
                    chord = dzl_shortcut_theme_get_chord_for_command (self->theme, shortcut->name);

                  accel = dzl_shortcut_chord_get_label (chord);
                  down = g_utf8_casefold (shortcut->title, -1);

                  gtk_tree_store_append (GTK_TREE_STORE (self), &p2, &p);
                  gtk_tree_store_set (GTK_TREE_STORE (self), &p2,
                                      DZL_SHORTCUT_MODEL_COLUMN_TYPE, shortcut->type,
                                      DZL_SHORTCUT_MODEL_COLUMN_ID, shortcut->name,
                                      DZL_SHORTCUT_MODEL_COLUMN_TITLE, shortcut->title,
                                      DZL_SHORTCUT_MODEL_COLUMN_ACCEL, accel,
                                      DZL_SHORTCUT_MODEL_COLUMN_KEYWORDS, down,
                                      DZL_SHORTCUT_MODEL_COLUMN_CHORD, chord,
                                      -1);
                }
            }
        }
    }
}

static void
dzl_shortcut_model_constructed (GObject *object)
{
  DzlShortcutModel *self = (DzlShortcutModel *)object;

  g_assert (DZL_IS_SHORTCUT_MODEL (self));

  G_OBJECT_CLASS (dzl_shortcut_model_parent_class)->constructed (object);

  dzl_shortcut_model_rebuild (self);
}

static void
dzl_shortcut_model_finalize (GObject *object)
{
  DzlShortcutModel *self = (DzlShortcutModel *)object;

  g_clear_object (&self->manager);
  g_clear_object (&self->theme);

  G_OBJECT_CLASS (dzl_shortcut_model_parent_class)->finalize (object);
}

static void
dzl_shortcut_model_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  DzlShortcutModel *self = DZL_SHORTCUT_MODEL (object);

  switch (prop_id)
    {
    case PROP_MANAGER:
      g_value_set_object (value, dzl_shortcut_model_get_manager (self));
      break;

    case PROP_THEME:
      g_value_set_object (value, dzl_shortcut_model_get_theme (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_shortcut_model_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  DzlShortcutModel *self = DZL_SHORTCUT_MODEL (object);

  switch (prop_id)
    {
    case PROP_MANAGER:
      dzl_shortcut_model_set_manager (self, g_value_get_object (value));
      break;

    case PROP_THEME:
      dzl_shortcut_model_set_theme (self, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_shortcut_model_class_init (DzlShortcutModelClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = dzl_shortcut_model_constructed;
  object_class->finalize = dzl_shortcut_model_finalize;
  object_class->get_property = dzl_shortcut_model_get_property;
  object_class->set_property = dzl_shortcut_model_set_property;

  properties [PROP_MANAGER] =
    g_param_spec_object ("manager",
                         "Manager",
                         "Manager",
                         DZL_TYPE_SHORTCUT_MANAGER,
                         (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  properties [PROP_THEME] =
    g_param_spec_object ("theme",
                         "Theme",
                         "Theme",
                         DZL_TYPE_SHORTCUT_THEME,
                         (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
dzl_shortcut_model_init (DzlShortcutModel *self)
{
  GType element_types[] = {
    G_TYPE_INT,
    G_TYPE_STRING,
    G_TYPE_STRING,
    G_TYPE_STRING,
    G_TYPE_STRING,
    DZL_TYPE_SHORTCUT_CHORD,
  };

  G_STATIC_ASSERT (G_N_ELEMENTS (element_types) == DZL_SHORTCUT_MODEL_N_COLUMNS);

  self->manager = g_object_ref (dzl_shortcut_manager_get_default ());

  gtk_tree_store_set_column_types (GTK_TREE_STORE (self),
                                   G_N_ELEMENTS (element_types),
                                   element_types);
}

/**
 * dzl_shortcut_model_new:
 *
 * Returns: (transfer full): A #GtkTreeModel
 */
GtkTreeModel *
dzl_shortcut_model_new (void)
{
  return g_object_new (DZL_TYPE_SHORTCUT_MODEL, NULL);
}

/**
 * dzl_shortcut_model_get_manager:
 * @self: a #DzlShortcutModel
 *
 * Gets the manager to be edited.
 *
 * Returns: (transfer none): A #DzlShortcutManager
 */
DzlShortcutManager *
dzl_shortcut_model_get_manager (DzlShortcutModel *self)
{
  g_return_val_if_fail (DZL_IS_SHORTCUT_MODEL (self), NULL);

  return self->manager;
}

void
dzl_shortcut_model_set_manager (DzlShortcutModel   *self,
                                DzlShortcutManager *manager)
{
  g_return_if_fail (DZL_IS_SHORTCUT_MODEL (self));
  g_return_if_fail (!manager || DZL_IS_SHORTCUT_MANAGER (manager));

  if (g_set_object (&self->manager, manager))
    {
      dzl_shortcut_model_rebuild (self);
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_MANAGER]);
    }
}

/**
 * dzl_shortcut_model_get_theme:
 * @self: a #DzlShortcutModel
 *
 * Get the theme to be edited.
 *
 * Returns: (transfer none): A #DzlShortcutTheme
 */
DzlShortcutTheme *
dzl_shortcut_model_get_theme (DzlShortcutModel *self)
{
  g_return_val_if_fail (DZL_IS_SHORTCUT_MODEL (self), NULL);

  return self->theme;
}

void
dzl_shortcut_model_set_theme (DzlShortcutModel *self,
                              DzlShortcutTheme *theme)
{
  g_return_if_fail (DZL_IS_SHORTCUT_MODEL (self));
  g_return_if_fail (!theme || DZL_IS_SHORTCUT_THEME (theme));

  if (g_set_object (&self->theme, theme))
    {
      dzl_shortcut_model_rebuild (self);
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_THEME]);
    }
}

static void
dzl_shortcut_model_apply (DzlShortcutModel *self,
                          GtkTreeIter      *iter)
{
  g_autoptr(DzlShortcutChord) chord = NULL;
  g_autofree gchar *id = NULL;
  gint type = 0;

  g_assert (DZL_IS_SHORTCUT_MODEL (self));
  g_assert (DZL_IS_SHORTCUT_THEME (self->theme));
  g_assert (iter != NULL);

  gtk_tree_model_get (GTK_TREE_MODEL (self), iter,
                      DZL_SHORTCUT_MODEL_COLUMN_TYPE, &type,
                      DZL_SHORTCUT_MODEL_COLUMN_ID, &id,
                      DZL_SHORTCUT_MODEL_COLUMN_CHORD, &chord,
                      -1);

  if (type == DZL_SHORTCUT_NODE_ACTION)
    dzl_shortcut_theme_set_chord_for_action (self->theme, id, chord, 0);
  else if (type == DZL_SHORTCUT_NODE_COMMAND)
    dzl_shortcut_theme_set_chord_for_command (self->theme, id, chord, 0);
  else
    g_warning ("Unknown type: %d", type);
}

void
dzl_shortcut_model_set_chord (DzlShortcutModel       *self,
                              GtkTreeIter            *iter,
                              const DzlShortcutChord *chord)
{
  g_autofree gchar *accel = NULL;

  g_return_if_fail (DZL_IS_SHORTCUT_MODEL (self));
  g_return_if_fail (iter != NULL);
  g_return_if_fail (gtk_tree_store_iter_is_valid (GTK_TREE_STORE (self), iter));

  accel = dzl_shortcut_chord_get_label (chord);

  gtk_tree_store_set (GTK_TREE_STORE (self), iter,
                      DZL_SHORTCUT_MODEL_COLUMN_ACCEL, accel,
                      DZL_SHORTCUT_MODEL_COLUMN_CHORD, chord,
                      -1);

  dzl_shortcut_model_apply (self, iter);
}
