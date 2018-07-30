/* dzl-shortcut-theme-load.c
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

#define G_LOG_DOMAIN "dzl-shortcut-theme"

#include "config.h"

#include <string.h>

#include "shortcuts/dzl-shortcut-context.h"
#include "shortcuts/dzl-shortcut-private.h"
#include "shortcuts/dzl-shortcut-theme.h"
#include "util/dzl-macros.h"

typedef enum
{
  LOAD_STATE_THEME = 1,
  LOAD_STATE_CONTEXT,
  LOAD_STATE_PROPERTY,
  LOAD_STATE_SHORTCUT,
  LOAD_STATE_SIGNAL,
  LOAD_STATE_PARAM,
  LOAD_STATE_ACTION,
} LoadStateType;

typedef struct _LoadStateFrame
{
  LoadStateType           type;

  /* Owned references */
  struct _LoadStateFrame *next;
  DzlShortcutContext     *context;
  gchar                  *accelerator;
  gchar                  *signal;
  GSList                 *params;

  /* Weak references */
  GObject                *object;
  GParamSpec             *pspec;

  guint                   translatable : 1;
} LoadStateFrame;

typedef struct
{
  DzlShortcutTheme *self;
  LoadStateFrame   *stack;
  GString          *text;
  const gchar      *translation_domain;
  guint             in_param : 1;
  guint             in_property : 1;
} LoadState;

static LoadStateFrame *
load_state_frame_new (LoadStateType type)
{
  LoadStateFrame *frm;

  frm = g_slice_new0 (LoadStateFrame);
  frm->type = type;

  return frm;
}

static void
load_state_frame_free (LoadStateFrame *frm)
{
  g_clear_object (&frm->context);
  g_clear_pointer (&frm->accelerator, g_free);
  g_clear_pointer (&frm->signal, g_free);

  g_slist_free_full (frm->params, g_free);
  frm->params = NULL;

  g_slice_free (LoadStateFrame, frm);
}

static void
load_state_push (LoadState      *state,
                 LoadStateFrame *frm)
{
  g_assert (state != NULL);
  g_assert (frm != NULL);
  g_assert (frm->next == NULL);

  frm->next = state->stack;
  state->stack = frm;
}

static gboolean
load_state_check_type (LoadState      *state,
                       LoadStateType   type,
                       GError        **error)
{
  if (state->stack != NULL)
    {
      if (state->stack->type == type)
        return TRUE;
    }

  g_set_error (error,
               G_IO_ERROR,
               G_IO_ERROR_FAILED,
               "Unexpected stack when unwinding elements");

  return FALSE;
}

static void
load_state_pop (LoadState *state)
{
  LoadStateFrame *frm = state->stack;

  if (frm != NULL)
    {
      state->stack = frm->next;
      load_state_frame_free (frm);
    }
}

static void
load_state_add_action (LoadState   *state,
                       const gchar *action)
{
  DzlShortcutContext *context = NULL;
  DzlShortcutTheme *theme = NULL;
  const gchar *accel = NULL;

  g_assert (state != NULL);
  g_assert (action != NULL);

  /* NOTE: Keep this in sync with load_state_add_command() */

  for (LoadStateFrame *iter = state->stack; iter != NULL; iter = iter->next)
    {
      if (iter->type == LOAD_STATE_SHORTCUT)
        accel = iter->accelerator;
      else if (iter->type == LOAD_STATE_CONTEXT)
        context = iter->context;
      else if (iter->type == LOAD_STATE_THEME)
        theme = state->self;

      if (accel && (context || theme))
        break;
    }

  if (accel != NULL)
    {
      if (context != NULL)
        dzl_shortcut_context_add_action (context, accel, action);
      else if (theme != NULL)
        dzl_shortcut_theme_set_accel_for_action (theme, action, accel, 0);
    }
}

static void
load_state_add_command (LoadState   *state,
                        const gchar *command)
{
  DzlShortcutContext *context = NULL;
  DzlShortcutTheme *theme = NULL;
  const gchar *accel = NULL;

  g_assert (state != NULL);
  g_assert (command != NULL);

  /* NOTE: Keep this in sync with load_state_add_action() */

  for (LoadStateFrame *iter = state->stack; iter != NULL; iter = iter->next)
    {
      if (iter->type == LOAD_STATE_SHORTCUT)
        accel = iter->accelerator;
      else if (iter->type == LOAD_STATE_CONTEXT)
        context = iter->context;
      else if (iter->type == LOAD_STATE_THEME)
        theme = state->self;

      if (accel && (context || theme))
        break;
    }

  if (accel != NULL)
    {
      if (context != NULL)
        dzl_shortcut_context_add_command (context, accel, command);
      else if (theme != NULL)
        dzl_shortcut_theme_set_accel_for_command (theme, command, accel, 0);
    }
}

static void
load_state_commit_param (LoadState *state)
{
  gchar *text;

  g_assert (state->stack != NULL);
  g_assert (state->stack->type == LOAD_STATE_SIGNAL);
  g_assert (state->text != NULL);

  text = g_string_free (state->text, FALSE);
  state->text = NULL;
  state->stack->params = g_slist_append (state->stack->params, text);
}

static void
load_state_commit_property (LoadState  *state,
                            GError    **error)
{
  g_auto(GValue) value = G_VALUE_INIT;

  g_assert (state->stack != NULL);
  g_assert (state->stack->type == LOAD_STATE_PROPERTY);
  g_assert (state->stack->pspec != NULL);
  g_assert (state->text != NULL);

  /* XXX: Note this isn't super safe, since we are passing a NULL
   *      GtkBuilder, but it does work for the cases we need to support.
   *      But there is the chance for a NULL dereference that we should
   *      probably protect against.
   */
  if (gtk_builder_value_from_string_type (NULL,
                                          G_PARAM_SPEC_VALUE_TYPE (state->stack->pspec),
                                          state->text->str,
                                          &value,
                                          error))
    g_object_set_property (state->stack->object,
                           state->stack->pspec->name,
                           &value);

  g_string_free (state->text, TRUE);
  state->text = NULL;
}

static void
parse_into_value (const gchar *str,
                  GValue      *value)
{
  g_autofree gchar *lower = NULL;

  /*
   * We don't know the type at this point, so we rely on various
   * GValueTransform to convert types at runtime upon signal emission. It adds
   * some runtime overhead but allows more flexibility in where we emit
   * signals from shortcuts.
   */

  if (!str || !*str)
    {
      g_value_init (value, G_TYPE_STRING);
      return;
    }

  if (g_ascii_isdigit (*str) || *str == '-' || *str == '+')
    {
      if (strchr (str, '.') != NULL)
        {
          g_value_init (value, G_TYPE_DOUBLE);
          g_value_set_double (value, g_ascii_strtod (str, NULL));
        }
      else
        {
          gint64 v = g_ascii_strtoll (str, NULL, 10);

          if (ABS (v) <= G_MAXINT)
            {
              g_value_init (value, G_TYPE_INT);
              g_value_set_int (value, v);
            }
          else
            {
              g_value_init (value, G_TYPE_INT64);
              g_value_set_int64 (value, v);
            }
        }

      return;
    }

  lower = g_utf8_strdown (str, -1);

  if (g_str_equal (lower, "false"))
    {
      g_value_init (value, G_TYPE_BOOLEAN);
      return;
    }

  if (g_str_equal (lower, "true"))
    {
      g_value_init (value, G_TYPE_BOOLEAN);
      g_value_set_boolean (value, TRUE);
      return;
    }

  g_value_init (value, G_TYPE_STRING);
  g_value_set_string (value, str);
}

static void
load_state_add_signal (LoadState *state)
{
  LoadStateFrame *signal;
  LoadStateFrame *shortcut;
  LoadStateFrame *context;
  g_autoptr(GArray) values = NULL;

  g_assert (state->stack != NULL);
  g_assert (state->stack->type == LOAD_STATE_SIGNAL);
  g_assert (state->stack->next != NULL);
  g_assert (state->stack->next->type == LOAD_STATE_SHORTCUT);
  g_assert (state->stack->next->accelerator != NULL);
  g_assert (state->stack->next->next->type == LOAD_STATE_CONTEXT);
  g_assert (state->stack->next->next->context != NULL);

  signal = state->stack;
  shortcut = signal->next;
  context = shortcut->next;

  g_assert (signal->type == LOAD_STATE_SIGNAL);
  g_assert (shortcut->type == LOAD_STATE_SHORTCUT);
  g_assert (context->type == LOAD_STATE_CONTEXT);

  values = g_array_sized_new (FALSE, FALSE, sizeof (GValue), g_slist_length (signal->params));
  g_array_set_clear_func (values, (GDestroyNotify)g_value_unset);

  for (const GSList *iter = signal->params; iter != NULL; iter = iter->next)
    {
      const gchar *str = iter->data;
      GValue value = G_VALUE_INIT;

      parse_into_value (str, &value);

      g_array_append_val (values, value);
    }

#if 0
  g_print ("Adding signal %s to %s via %s\n",
           signal->signal,
           dzl_shortcut_context_get_name (context->context),
           shortcut->accelerator);
#endif

  dzl_shortcut_context_add_signalv (context->context,
                                    shortcut->accelerator,
                                    signal->signal,
                                    values);
}

static void
theme_start_element (GMarkupParseContext  *context,
                     const gchar          *element_name,
                     const gchar         **attr_names,
                     const gchar         **attr_values,
                     gpointer              user_data,
                     GError              **error)
{
  LoadState *state = user_data;

  g_assert (state != NULL);
  g_assert (DZL_IS_SHORTCUT_THEME (state->self));
  g_assert (context != NULL);
  g_assert (element_name != NULL);

  if (g_strcmp0 (element_name, "keytheme") == 0)
    {
      const gchar *name = NULL;
      const gchar *parent = NULL;
      const gchar *domain = NULL;

      if (state->stack != NULL)
        {
          g_set_error (error,
                       G_IO_ERROR,
                       G_IO_ERROR_INVALID_DATA,
                       "Got theme element in location other than root");
          return;
        }

      if (!g_markup_collect_attributes (element_name, attr_names, attr_values, error,
                                        G_MARKUP_COLLECT_STRING, "name", &name,
                                        G_MARKUP_COLLECT_STRING | G_MARKUP_COLLECT_OPTIONAL, "parent", &parent,
                                        G_MARKUP_COLLECT_STRING | G_MARKUP_COLLECT_OPTIONAL, "translation-domain", &domain,
                                        G_MARKUP_COLLECT_INVALID))
        return;

      if (domain != NULL)
        state->translation_domain = g_intern_string (domain);

      _dzl_shortcut_theme_set_name (state->self, name);

      if (parent != NULL)
        dzl_shortcut_theme_set_parent_name (state->self, parent);

      load_state_push (state, load_state_frame_new (LOAD_STATE_THEME));
    }
  else if (g_strcmp0 (element_name, "property") == 0)
    {
      LoadStateFrame *frm;
      const gchar *translatable = NULL;
      const gchar *name = NULL;
      GParamSpec *pspec;
      GObject *obj = NULL;

      if (!load_state_check_type (state, LOAD_STATE_CONTEXT, NULL) &&
          !load_state_check_type (state, LOAD_STATE_THEME, NULL))
        {
          g_set_error (error,
                       G_IO_ERROR,
                       G_IO_ERROR_INVALID_DATA,
                       "property only valid in theme or context");
          return;
        }

      if (state->stack->type == LOAD_STATE_CONTEXT)
        obj = G_OBJECT (state->stack->context);
      else if (state->stack->type == LOAD_STATE_THEME)
        obj = G_OBJECT (state->self);
      else { g_assert_not_reached (); }

      if (!g_markup_collect_attributes (element_name, attr_names, attr_values, error,
                                        G_MARKUP_COLLECT_STRING, "name", &name,
                                        G_MARKUP_COLLECT_STRING | G_MARKUP_COLLECT_OPTIONAL, "translatable", &translatable,
                                        G_MARKUP_COLLECT_INVALID))
        return;

      pspec = g_object_class_find_property (G_OBJECT_GET_CLASS (obj), name);

      if (pspec == NULL)
        {
          g_set_error (error,
                       G_MARKUP_ERROR,
                       G_MARKUP_ERROR_INVALID_CONTENT,
                       "Failed to locate “%s” property",
                       name);
          return;
        }

      frm = load_state_frame_new (LOAD_STATE_PROPERTY);
      frm->pspec = pspec;
      frm->object = obj;
      frm->translatable = translatable && (*translatable == 'y' || *translatable == 'Y');

      load_state_push (state, frm);

      state->in_property = TRUE;
    }
  else if (g_strcmp0 (element_name, "context") == 0)
    {
      LoadStateFrame *frm;
      const gchar *name = NULL;

      if (!load_state_check_type (state, LOAD_STATE_THEME, error))
        return;

      if (!g_markup_collect_attributes (element_name, attr_names, attr_values, error,
                                        G_MARKUP_COLLECT_STRING, "name", &name,
                                        G_MARKUP_COLLECT_INVALID))
        return;

      frm = load_state_frame_new (LOAD_STATE_CONTEXT);
      frm->context = dzl_shortcut_context_new (name);

      load_state_push (state, frm);
    }
  else if (g_strcmp0 (element_name, "shortcut") == 0)
    {
      LoadStateFrame *frm;
      const gchar *accelerator = NULL;
      const gchar *action = NULL;
      const gchar *signal = NULL;
      const gchar *command = NULL;

      if (!load_state_check_type (state, LOAD_STATE_CONTEXT, NULL) &&
          !load_state_check_type (state, LOAD_STATE_THEME, NULL))
        {
          g_set_error (error,
                       G_MARKUP_ERROR,
                       G_MARKUP_ERROR_INVALID_CONTENT,
                       "shortcut only allowed in context or theme elements");
          return;
        }

      if (!g_markup_collect_attributes (element_name, attr_names, attr_values, error,
                                        G_MARKUP_COLLECT_STRING, "accelerator", &accelerator,
                                        G_MARKUP_COLLECT_STRING | G_MARKUP_COLLECT_OPTIONAL, "action", &action,
                                        G_MARKUP_COLLECT_STRING | G_MARKUP_COLLECT_OPTIONAL, "signal", &signal,
                                        G_MARKUP_COLLECT_STRING | G_MARKUP_COLLECT_OPTIONAL, "command", &command,
                                        G_MARKUP_COLLECT_INVALID))
        return;

      frm = load_state_frame_new (LOAD_STATE_SHORTCUT);
      frm->accelerator = g_strdup (accelerator);
      load_state_push (state, frm);

      if (action != NULL)
        load_state_add_action (state, action);

      if (command != NULL)
        load_state_add_command (state, command);

      if (signal != NULL)
        {
          frm = load_state_frame_new (LOAD_STATE_SIGNAL);
          frm->signal = g_strdup (signal);
          load_state_push (state, frm);
          load_state_add_signal (state);
          load_state_pop (state);
        }
    }
  else if (g_strcmp0 (element_name, "signal") == 0)
    {
      LoadStateFrame *frm;
      const gchar *name = NULL;

      if (!load_state_check_type (state, LOAD_STATE_SHORTCUT, error))
        return;

      if (!g_markup_collect_attributes (element_name, attr_names, attr_values, error,
                                        G_MARKUP_COLLECT_STRING, "name", &name,
                                        G_MARKUP_COLLECT_INVALID))
        return;

      frm = load_state_frame_new (LOAD_STATE_SIGNAL);
      frm->signal = g_strdup (name);

      load_state_push (state, frm);
    }
  else if (g_strcmp0 (element_name, "param") == 0)
    {
      if (!load_state_check_type (state, LOAD_STATE_SIGNAL, error))
        return;

      state->in_param = TRUE;
    }
  else if (g_strcmp0 (element_name, "action") == 0)
    {
      const gchar *name = NULL;

      if (!load_state_check_type (state, LOAD_STATE_SHORTCUT, error))
        return;

      if (!g_markup_collect_attributes (element_name, attr_names, attr_values, error,
                                        G_MARKUP_COLLECT_STRING, "name", &name,
                                        G_MARKUP_COLLECT_INVALID))
        return;

      load_state_add_action (state, name);
    }
  else if (g_strcmp0 (element_name, "resource") == 0)
    {
      const gchar *path = NULL;
      g_autofree gchar *full_path = NULL;

      if (!load_state_check_type (state, LOAD_STATE_THEME, error))
        return;

      if (!g_markup_collect_attributes (element_name, attr_names, attr_values, error,
                                        G_MARKUP_COLLECT_STRING, "path", &path,
                                        G_MARKUP_COLLECT_INVALID))
        return;

      g_assert (state->self != NULL);

      if (!g_str_has_prefix (path, "resource://"))
        path = full_path = g_strdup_printf ("resource://%s", path);

      dzl_shortcut_theme_add_css_resource (state->self, path);
    }
}

static void
theme_end_element (GMarkupParseContext  *context,
                   const gchar          *element_name,
                   gpointer              user_data,
                   GError              **error)
{
  LoadState *state = user_data;

  g_assert (context != NULL);
  g_assert (element_name != NULL);

  if (g_strcmp0 (element_name, "keytheme") == 0)
    {
      if (!load_state_check_type (state, LOAD_STATE_THEME, error))
        return;
    }
  else if (g_strcmp0 (element_name, "resource") == 0)
    {
      /* nothing to pop, but we want to propagate any errors */
      load_state_check_type (state, LOAD_STATE_THEME, error);
      return;
    }
  else if (g_strcmp0 (element_name, "property") == 0)
    {
      if (!load_state_check_type (state, LOAD_STATE_PROPERTY, error))
        return;

      if (state->text)
        load_state_commit_property (state, error);

      state->in_property = FALSE;
    }
  else if (g_strcmp0 (element_name, "context") == 0)
    {
      if (!load_state_check_type (state, LOAD_STATE_CONTEXT, error))
        return;

      dzl_shortcut_theme_add_context (state->self, state->stack->context);
    }
  else if (g_strcmp0 (element_name, "shortcut") == 0)
    {
      if (!load_state_check_type (state, LOAD_STATE_SHORTCUT, error))
        return;
    }
  else if (g_strcmp0 (element_name, "signal") == 0)
    {
      if (!load_state_check_type (state, LOAD_STATE_SIGNAL, error))
        return;

      load_state_add_signal (state);
    }
  else if (g_strcmp0 (element_name, "param") == 0)
    {
      if (!load_state_check_type (state, LOAD_STATE_SIGNAL, error))
        return;

      g_assert (state->in_param);

      if (state->text)
        load_state_commit_param (state);

      state->in_param = FALSE;

      return;
    }
  else if (g_strcmp0 (element_name, "action") == 0)
    {
      load_state_check_type (state, LOAD_STATE_SHORTCUT, error);
      return;
    }
  else
    {
      g_set_error (error,
                   G_IO_ERROR,
                   G_IO_ERROR_INVALID_DATA,
                   "Unexpected close element %s",
                   element_name);
      return;
    }

  load_state_pop (state);
}

static void
theme_text (GMarkupParseContext  *context,
            const gchar          *text,
            gsize                 text_len,
            gpointer              user_data,
            GError              **error)
{
  LoadState *state = user_data;

  g_assert (context != NULL);
  g_assert (text != NULL);
  g_assert (state != NULL);

  if (state->in_param || state->in_property)
    {
      if ((state->in_param && !load_state_check_type (state, LOAD_STATE_SIGNAL, error)) ||
          (state->in_property && !load_state_check_type (state, LOAD_STATE_PROPERTY, error)))
        return;

      if (state->text == NULL)
        state->text = g_string_new (NULL);

      g_string_append_len (state->text, text, text_len);
    }
}

static const GMarkupParser theme_parser = {
  .start_element = theme_start_element,
  .end_element = theme_end_element,
  .text = theme_text,
};

gboolean
dzl_shortcut_theme_load_from_data (DzlShortcutTheme  *self,
                                   const gchar       *data,
                                   gssize             len,
                                   GError           **error)
{
  g_autoptr(GMarkupParseContext) context = NULL;
  LoadState state = { 0 };
  gboolean ret;

  g_return_val_if_fail (DZL_IS_SHORTCUT_THEME (self), FALSE);
  g_return_val_if_fail (data != NULL, FALSE);

  state.self = self;

  context = g_markup_parse_context_new (&theme_parser, 0, &state, NULL);
  ret = g_markup_parse_context_parse (context, data, len, error);

  while (state.stack != NULL)
    {
      LoadStateFrame *frm = state.stack;
      state.stack = frm->next;
      load_state_frame_free (frm);
    }

  if (state.text)
    g_string_free (state.text, TRUE);

  return ret;
}

gboolean
dzl_shortcut_theme_load_from_file (DzlShortcutTheme  *self,
                                   GFile             *file,
                                   GCancellable      *cancellable,
                                   GError           **error)
{
  g_autofree gchar *contents = NULL;
  gsize len = 0;

  g_return_val_if_fail (DZL_IS_SHORTCUT_THEME (self), FALSE);
  g_return_val_if_fail (G_IS_FILE (file), FALSE);
  g_return_val_if_fail (!cancellable || G_IS_CANCELLABLE (cancellable), FALSE);

  if (!g_file_load_contents (file, cancellable, &contents, &len, NULL, error))
    return FALSE;

  return dzl_shortcut_theme_load_from_data (self, contents, len, error);
}

gboolean
dzl_shortcut_theme_load_from_path (DzlShortcutTheme  *self,
                                   const gchar       *path,
                                   GCancellable      *cancellable,
                                   GError           **error)
{
  g_autoptr(GFile) file = NULL;

  g_return_val_if_fail (DZL_IS_SHORTCUT_THEME (self), FALSE);
  g_return_val_if_fail (path != NULL, FALSE);
  g_return_val_if_fail (!cancellable || G_IS_CANCELLABLE (cancellable), FALSE);

  file = g_file_new_for_path (path);

  return dzl_shortcut_theme_load_from_file (self, file, cancellable, error);
}
