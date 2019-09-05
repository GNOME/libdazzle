/* dzl-shortcut-theme-editor.c
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

#define G_LOG_DOMAIN "dzl-shortcut-theme-editor"

#include "config.h"

#include <glib/gi18n.h>

#include "shortcuts/dzl-shortcut-accel-dialog.h"
#include "shortcuts/dzl-shortcut-model.h"
#include "shortcuts/dzl-shortcut-private.h"
#include "shortcuts/dzl-shortcut-theme-editor.h"
#include "util/dzl-util-private.h"

typedef struct
{
  GtkTreeView         *tree_view;
  GtkSearchEntry      *filter_entry;
  GtkTreeViewColumn   *shortcut_column;
  GtkCellRendererText *shortcut_cell;
  GtkTreeViewColumn   *title_column;
  GtkCellRendererText *title_cell;

  DzlShortcutTheme    *theme;
  GtkTreeModel        *model;
  GtkTreePath         *selected;
  PangoAttrList       *attrs;
} DzlShortcutThemeEditorPrivate;

enum {
  PROP_0,
  PROP_THEME,
  N_PROPS
};

enum {
  CHANGED,
  N_SIGNALS
};

G_DEFINE_TYPE_WITH_PRIVATE (DzlShortcutThemeEditor, dzl_shortcut_theme_editor, GTK_TYPE_BIN)

static GParamSpec *properties [N_PROPS];
static guint signals [N_SIGNALS];

static void
dzl_shortcut_theme_editor_dialog_response (DzlShortcutThemeEditor *self,
                                           gint                    response_code,
                                           DzlShortcutAccelDialog *dialog)
{
  DzlShortcutThemeEditorPrivate *priv = dzl_shortcut_theme_editor_get_instance_private (self);
  gboolean changed = FALSE;

  g_assert (DZL_IS_SHORTCUT_THEME_EDITOR (self));
  g_assert (DZL_SHORTCUT_ACCEL_DIALOG (dialog));

  if (response_code == GTK_RESPONSE_ACCEPT)
    {
      const DzlShortcutChord *chord = dzl_shortcut_accel_dialog_get_chord (dialog);

      if (priv->selected != NULL)
        {
          GtkTreePath *path;
          GtkTreeModel *model;
          GtkTreeIter iter;

          model = gtk_tree_view_get_model (priv->tree_view);

          if (GTK_IS_TREE_STORE (model))
            path = gtk_tree_path_copy (priv->selected);
          else
            path = gtk_tree_model_filter_convert_path_to_child_path (GTK_TREE_MODEL_FILTER (model), priv->selected);

          if (gtk_tree_model_get_iter (model, &iter, path))
            dzl_shortcut_model_set_chord (DZL_SHORTCUT_MODEL (priv->model), &iter, chord);
        }

      changed = TRUE;
    }

  gtk_widget_destroy (GTK_WIDGET (dialog));

  if (changed)
    g_signal_emit (self, signals [CHANGED], 0);
}

static gboolean
dzl_shortcut_theme_editor_visible_func (GtkTreeModel *model,
                                        GtkTreeIter  *iter,
                                        gpointer      user_data)
{
  const gchar *text = user_data;
  g_autofree gchar *keywords= NULL;
  GtkTreeIter parent;

  g_assert (GTK_IS_TREE_MODEL (model));
  g_assert (iter != NULL);
  g_assert (text != NULL);

  if (!gtk_tree_model_iter_parent (model, &parent, iter))
    return TRUE;

  gtk_tree_model_get (model, iter,
                      DZL_SHORTCUT_MODEL_COLUMN_KEYWORDS, &keywords,
                      -1);

  /* keywords and text are both casefolded */
  if (strstr (keywords, text) != NULL)
    return TRUE;

  return FALSE;
}

static void
dzl_shortcut_theme_editor_filter_changed (DzlShortcutThemeEditor *self,
                                          GtkSearchEntry         *entry)
{
  DzlShortcutThemeEditorPrivate *priv = dzl_shortcut_theme_editor_get_instance_private (self);
  g_autoptr(GtkTreeModel) filter = NULL;
  const gchar *text;

  g_assert (DZL_IS_SHORTCUT_THEME_EDITOR (self));
  g_assert (GTK_IS_SEARCH_ENTRY (entry));

  filter = gtk_tree_model_filter_new (priv->model, NULL);
  text = gtk_entry_get_text (GTK_ENTRY (entry));

  if (dzl_str_empty0 (text))
    {
      gtk_tree_view_set_model (priv->tree_view, priv->model);
      gtk_tree_view_expand_all (priv->tree_view);
      return;
    }

  gtk_tree_model_filter_set_visible_func (GTK_TREE_MODEL_FILTER (filter),
                                          dzl_shortcut_theme_editor_visible_func,
                                          g_utf8_casefold (text, -1),
                                          g_free);
  gtk_tree_view_set_model (priv->tree_view, GTK_TREE_MODEL (filter));
  gtk_tree_view_expand_all (priv->tree_view);
}

static void
dzl_shortcut_theme_editor_row_activated (DzlShortcutThemeEditor *self,
                                         GtkTreePath            *tree_path,
                                         GtkTreeViewColumn      *column,
                                         GtkTreeView            *tree_view)
{
  DzlShortcutThemeEditorPrivate *priv = dzl_shortcut_theme_editor_get_instance_private (self);
  GtkTreeModel *model;
  GtkTreeIter iter;

  g_assert (DZL_IS_SHORTCUT_THEME_EDITOR (self));
  g_assert (GTK_IS_TREE_VIEW (tree_view));
  g_assert (tree_path != NULL);
  g_assert (GTK_IS_TREE_VIEW_COLUMN (column));

  if (gtk_tree_path_get_depth (tree_path) == 1)
    return;

  model = gtk_tree_view_get_model (tree_view);

  if (gtk_tree_model_get_iter (model, &iter, tree_path))
    {
      g_autofree gchar *title = NULL;
      g_autofree gchar *accel = NULL;
      GtkDialog *dialog;
      GtkWidget *toplevel;

      g_clear_pointer (&priv->selected, gtk_tree_path_free);
      priv->selected = gtk_tree_path_copy (tree_path);

      gtk_tree_model_get (model, &iter,
                          DZL_SHORTCUT_MODEL_COLUMN_TITLE, &title,
                          DZL_SHORTCUT_MODEL_COLUMN_ACCEL, &accel,
                          -1);

      toplevel = gtk_widget_get_ancestor (GTK_WIDGET (self), GTK_TYPE_WINDOW);

      dialog = g_object_new (DZL_TYPE_SHORTCUT_ACCEL_DIALOG,
                             "modal", TRUE,
                             "resizable", FALSE,
                             "shortcut-title", title,
                             "title", _("Set Shortcut"),
                             "transient-for", toplevel,
                             "use-header-bar", TRUE,
                             NULL);

      g_signal_connect_object (dialog,
                               "response",
                               G_CALLBACK (dzl_shortcut_theme_editor_dialog_response),
                               self,
                               G_CONNECT_SWAPPED);

      gtk_window_present (GTK_WINDOW (dialog));
    }
}

static void
shortcut_cell_data_func (GtkCellLayout   *cell_layout,
                         GtkCellRenderer *renderer,
                         GtkTreeModel    *model,
                         GtkTreeIter     *iter,
                         gpointer         user_data)
{
  DzlShortcutThemeEditor *self = user_data;
  g_autofree gchar *accel = NULL;
  GtkTreeIter piter;

  g_assert (GTK_IS_TREE_VIEW_COLUMN (cell_layout));
  g_assert (GTK_IS_CELL_RENDERER (renderer));
  g_assert (GTK_IS_TREE_MODEL (model));
  g_assert (iter != NULL);
  g_assert (DZL_IS_SHORTCUT_THEME_EDITOR (self));

  gtk_tree_model_get (model, iter,
                      DZL_SHORTCUT_MODEL_COLUMN_ACCEL, &accel,
                      -1);

  if (accel && *accel)
    g_object_set (renderer, "text", accel, NULL);
  else if (gtk_tree_model_iter_parent (model, &piter, iter))
    g_object_set (renderer, "text", "Disabled", NULL);
  else
    g_object_set (renderer, "text", NULL, NULL);
}

static void
title_cell_data_func (GtkCellLayout   *cell_layout,
                      GtkCellRenderer *renderer,
                      GtkTreeModel    *model,
                      GtkTreeIter     *iter,
                      gpointer         user_data)
{
  DzlShortcutThemeEditor *self = user_data;
  DzlShortcutThemeEditorPrivate *priv = dzl_shortcut_theme_editor_get_instance_private (self);
  g_autofree gchar *title = NULL;
  GtkTreeIter piter;

  g_assert (GTK_IS_TREE_VIEW_COLUMN (cell_layout));
  g_assert (GTK_IS_CELL_RENDERER (renderer));
  g_assert (GTK_IS_TREE_MODEL (model));
  g_assert (iter != NULL);
  g_assert (DZL_IS_SHORTCUT_THEME_EDITOR (self));

  gtk_tree_model_get (model, iter,
                      DZL_SHORTCUT_MODEL_COLUMN_TITLE, &title,
                      -1);

  g_object_set (renderer, "text", title, NULL);

  if (!gtk_tree_model_iter_parent (model, &piter, iter))
    g_object_set (renderer, "attributes", priv->attrs, NULL);
  else
    g_object_set (renderer, "attributes", NULL, NULL);
}

static void
dzl_shortcut_theme_editor_changed (DzlShortcutThemeEditor *self,
                                   DzlShortcutManager     *manager)
{
  DzlShortcutThemeEditorPrivate *priv = dzl_shortcut_theme_editor_get_instance_private (self);

  g_assert (DZL_IS_SHORTCUT_THEME_EDITOR (self));
  g_assert (DZL_IS_SHORTCUT_MANAGER (manager));

  dzl_shortcut_model_rebuild (DZL_SHORTCUT_MODEL (priv->model));
  gtk_tree_view_expand_all (priv->tree_view);
}

static void
dzl_shortcut_theme_editor_finalize (GObject *object)
{
  DzlShortcutThemeEditor *self = (DzlShortcutThemeEditor *)object;
  DzlShortcutThemeEditorPrivate *priv = dzl_shortcut_theme_editor_get_instance_private (self);

  g_clear_object (&priv->model);
  g_clear_object (&priv->theme);
  g_clear_pointer (&priv->selected, gtk_tree_path_free);
  g_clear_pointer (&priv->attrs, pango_attr_list_unref);

  G_OBJECT_CLASS (dzl_shortcut_theme_editor_parent_class)->finalize (object);
}

static void
dzl_shortcut_theme_editor_get_property (GObject    *object,
                                        guint       prop_id,
                                        GValue     *value,
                                        GParamSpec *pspec)
{
  DzlShortcutThemeEditor *self = DZL_SHORTCUT_THEME_EDITOR (object);

  switch (prop_id)
    {
    case PROP_THEME:
      g_value_set_object (value, dzl_shortcut_theme_editor_get_theme (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_shortcut_theme_editor_set_property (GObject      *object,
                                        guint         prop_id,
                                        const GValue *value,
                                        GParamSpec   *pspec)
{
  DzlShortcutThemeEditor *self = DZL_SHORTCUT_THEME_EDITOR (object);

  switch (prop_id)
    {
    case PROP_THEME:
      dzl_shortcut_theme_editor_set_theme (self, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_shortcut_theme_editor_class_init (DzlShortcutThemeEditorClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = dzl_shortcut_theme_editor_finalize;
  object_class->get_property = dzl_shortcut_theme_editor_get_property;
  object_class->set_property = dzl_shortcut_theme_editor_set_property;

  properties [PROP_THEME] =
    g_param_spec_object ("theme",
                         "Theme",
                         "The theme for editing",
                         DZL_TYPE_SHORTCUT_THEME,
                         (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);

  /**
   * DzlShortcutThemeEditor::changed:
   *
   * The "changed" signal is emitted when one of the rows within the editor
   * has been changed.
   *
   * You might want to use this signal to save your theme changes to your
   * configured storage backend.
   */
  signals [CHANGED] =
    g_signal_new ("changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0, NULL, NULL, NULL,
                  G_TYPE_NONE, 0);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/dazzle/ui/dzl-shortcut-theme-editor.ui");

  gtk_widget_class_bind_template_child_private (widget_class, DzlShortcutThemeEditor, tree_view);
  gtk_widget_class_bind_template_child_private (widget_class, DzlShortcutThemeEditor, filter_entry);
  gtk_widget_class_bind_template_child_private (widget_class, DzlShortcutThemeEditor, shortcut_cell);
  gtk_widget_class_bind_template_child_private (widget_class, DzlShortcutThemeEditor, shortcut_column);
  gtk_widget_class_bind_template_child_private (widget_class, DzlShortcutThemeEditor, title_cell);
  gtk_widget_class_bind_template_child_private (widget_class, DzlShortcutThemeEditor, title_column);
}

static void
dzl_shortcut_theme_editor_init (DzlShortcutThemeEditor *self)
{
  DzlShortcutThemeEditorPrivate *priv = dzl_shortcut_theme_editor_get_instance_private (self);
  PangoAttrList *list = NULL;

  gtk_widget_init_template (GTK_WIDGET (self));

  priv->model = dzl_shortcut_model_new ();
  gtk_tree_view_set_model (priv->tree_view, priv->model);

  g_signal_connect_object (dzl_shortcut_manager_get_default (),
                           "changed",
                           G_CALLBACK (dzl_shortcut_theme_editor_changed),
                           self,
                           G_CONNECT_SWAPPED);

  g_signal_connect_object (priv->filter_entry,
                           "changed",
                           G_CALLBACK (dzl_shortcut_theme_editor_filter_changed),
                           self,
                           G_CONNECT_SWAPPED);

  g_signal_connect (priv->filter_entry,
                    "stop-search",
                    G_CALLBACK (gtk_entry_set_text),
                    (gpointer) "");

  g_signal_connect_object (priv->tree_view,
                           "row-activated",
                           G_CALLBACK (dzl_shortcut_theme_editor_row_activated),
                           self,
                           G_CONNECT_SWAPPED);

  /* Set "dim-label" like alpha on the shortcut label */
  list = pango_attr_list_new ();
  pango_attr_list_insert (list, pango_attr_foreground_alpha_new (0.55 * G_MAXUSHORT));
  g_object_set (priv->shortcut_cell,
                "attributes", list,
                NULL);
  pango_attr_list_unref (list);

  /* Setup cell data funcs */
  gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT (priv->title_column),
                                      GTK_CELL_RENDERER (priv->title_cell),
                                      title_cell_data_func,
                                      self,
                                      NULL);
  gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT (priv->shortcut_column),
                                      GTK_CELL_RENDERER (priv->shortcut_cell),
                                      shortcut_cell_data_func,
                                      self,
                                      NULL);

  /* diable selections on the treeview */
  gtk_tree_selection_set_mode (gtk_tree_view_get_selection (priv->tree_view),
                               GTK_SELECTION_NONE);

  priv->attrs = pango_attr_list_new ();
  pango_attr_list_insert (priv->attrs, pango_attr_foreground_alpha_new (0.55 * 0xFFFF));
}

GtkWidget *
dzl_shortcut_theme_editor_new (void)
{
  return g_object_new (DZL_TYPE_SHORTCUT_THEME_EDITOR, NULL);
}

/**
 * dzl_shortcut_theme_editor_get_theme:
 * @self: a #DzlShortcutThemeEditor
 *
 * Gets the shortcut theme if one hsa been set.
 *
 * Returns: (transfer none) (nullable): An #DzlShortcutTheme or %NULL
 */
DzlShortcutTheme *
dzl_shortcut_theme_editor_get_theme (DzlShortcutThemeEditor *self)
{
  DzlShortcutThemeEditorPrivate *priv = dzl_shortcut_theme_editor_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_SHORTCUT_THEME_EDITOR (self), NULL);

  return priv->theme;
}

void
dzl_shortcut_theme_editor_set_theme (DzlShortcutThemeEditor *self,
                                     DzlShortcutTheme       *theme)
{
  DzlShortcutThemeEditorPrivate *priv = dzl_shortcut_theme_editor_get_instance_private (self);

  g_return_if_fail (DZL_IS_SHORTCUT_THEME_EDITOR (self));
  g_return_if_fail (!theme || DZL_IS_SHORTCUT_THEME (theme));

  if (g_set_object (&priv->theme, theme))
    {
      dzl_shortcut_model_set_theme (DZL_SHORTCUT_MODEL (priv->model), theme);
      gtk_tree_view_expand_all (priv->tree_view);
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_THEME]);
    }
}
