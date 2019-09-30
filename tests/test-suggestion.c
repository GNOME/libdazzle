/* test-suggestion.c
 *
 * Copyright (C) 2017 Christian Hergert <chergert@redhat.com>
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

#include <string.h>
#include <dazzle.h>

/*
 * Most of this is exactly how you SHOULD NOT write a web browser
 * shell. It's just dummy code to test the suggestions widget.
 * Think for yourself before copying any of this code.
 */

typedef struct
{
  const gchar *icon_name;
  const gchar *url;
  const gchar *title;
  const gchar *suffix;
} DemoData;

static DzlFuzzyMutableIndex *search_index;
static gulong notify_suggestion_handler;
static gulong button_handler;
static const DemoData demo_data[] = {
  { "web-browser-symbolic", "https://twitter.com", "Twitter", "twitter.com" },
  { "web-browser-symbolic", "https://facebook.com", "Facebook", "facebook.com" },
  { "web-browser-symbolic", "https://google.com", "Google", "google.com" },
  { "web-browser-symbolic", "https://images.google.com", "Google Images", "images.google.com" },
  { "web-browser-symbolic", "https://news.ycombinator.com", "Hacker News", "news.ycombinator.com" },
  { "web-browser-symbolic", "https://reddit.com/r/gnome", "GNOME Desktop Environment", "reddit.com/r/gnome" },
  { "web-browser-symbolic", "https://reddit.com/r/linux", "Linux, GNU/Linux, free software", "reddit.com/r/linux" },
  { "web-browser-symbolic", "https://wiki.gnome.org", "GNOME Wiki", "wiki.gnome.org" },
  { "web-browser-symbolic", "https://gnome.org", "GNOME", "gnome.org" },
  { "web-browser-symbolic", "https://planet.gnome.org", "Planet GNOME", "planet.gnome.org" },
  { "web-browser-symbolic", "https://wiki.gnome.org/Apps/Builder", "GNOME Builder", "wiki.gnome.org/Apps/Builder" },
};

static void
take_item (GListStore    *store,
           DzlSuggestion *suggestion)
{
  g_list_store_append (store, suggestion);
  g_object_unref (suggestion);
}

static gboolean
is_a_url (const gchar *str)
{
  /* you obviously want something better */

  if (strstr (str, ".com") ||
      strstr (str, ".net") ||
      strstr (str, ".org") ||
      strstr (str, ".io") ||
      strstr (str, ".ly"))
    return TRUE;

  return FALSE;
}

static gint
compare_match (gconstpointer a,
               gconstpointer b)
{
  const DzlFuzzyMutableIndexMatch *match_a = a;
  const DzlFuzzyMutableIndexMatch *match_b = b;

  if (match_a->score < match_b->score)
    return 1;
  else if (match_a->score > match_b->score)
    return -1;
  else
    return 0;
}

static gchar *
suggest_suffix (DzlSuggestion  *suggestion,
                const gchar    *typed_text,
                const DemoData *data)
{
  //g_print ("Suffix: Typed_Text=%s\n", typed_text);

  if (g_str_has_prefix (data->suffix, typed_text))
    return g_strdup (data->suffix + strlen (typed_text));

  return NULL;
}

static gchar *
replace_typed_text (DzlSuggestion  *suggestion,
                    const gchar    *typed_text,
                    const DemoData *data)
{
  return g_strdup (data->url);
}

typedef struct
{
  GListStore *store;
  gchar      *full_query;
  gchar      *query;
} AddSearchResults;

static void
add_search_results_free (AddSearchResults *r)
{
  g_clear_object (&r->store);
  g_clear_pointer (&r->query, g_free);
  g_clear_pointer (&r->full_query, g_free);
  g_slice_free (AddSearchResults, r);
}

static GListModel *
add_search_results (GListStore  *store,
                    const gchar *full_query,
                    const gchar *query)
{
  g_autoptr(GArray) matches = NULL;
  g_autofree gchar *search_url = NULL;
  g_autofree gchar *with_slashes = g_strdup_printf ("://%s", query);
  gboolean exact = FALSE;

  matches = dzl_fuzzy_mutable_index_match (search_index, query, 20);

  g_array_sort (matches, compare_match);

  for (guint i = 0; i < matches->len; i++)
    {
      const DzlFuzzyMutableIndexMatch *match = &g_array_index (matches, DzlFuzzyMutableIndexMatch, i);
      const DemoData *data = match->value;
      g_autofree gchar *markup = NULL;
      DzlSuggestion *item;

      markup = dzl_fuzzy_highlight (data->url, query, FALSE);

      if (g_str_has_suffix (data->url, with_slashes))
        exact = TRUE;

      item = g_object_new (DZL_TYPE_SUGGESTION,
                           "id", data->url,
                           "icon-name", data->icon_name,
                           "title", markup,
                           "subtitle", data->title,
                           NULL);
      g_signal_connect (item, "suggest-suffix", G_CALLBACK (suggest_suffix), (gpointer)data);
      g_signal_connect (item, "replace-typed-text", G_CALLBACK (replace_typed_text), (gpointer)data);
      take_item (store, item);
    }

  if (!exact && is_a_url (full_query))
    {
      g_autofree gchar *url = g_strdup_printf ("http://%s", full_query);
      take_item (store,
                 g_object_new (DZL_TYPE_SUGGESTION,
                               "id", url,
                               "icon-name", NULL,
                               "title", query,
                               "subtitle", NULL,
                               NULL));
    }

  search_url = g_strdup_printf ("https://www.google.com/search?q=%s", full_query);
  take_item (store,
             g_object_new (DZL_TYPE_SUGGESTION,
                           "id", search_url,
                           "title", full_query,
                           "subtitle", "Google Search",
                           "icon-name", "edit-find-symbolic",
                           NULL));

  return G_LIST_MODEL (store);
}

static gboolean
key_press (GtkWidget   *widget,
           GdkEventKey *key,
           GtkEntry    *param)
{
  if (key->keyval == GDK_KEY_l && (key->state & GDK_CONTROL_MASK) == GDK_CONTROL_MASK)
    {
      gtk_widget_grab_focus (GTK_WIDGET (param));
      gtk_editable_select_region (GTK_EDITABLE (param), 0, -1);
    }
  else if (key->keyval == GDK_KEY_w && (key->state & GDK_CONTROL_MASK) == GDK_CONTROL_MASK)
    gtk_window_close (GTK_WINDOW (widget));

  return FALSE;
}

static gboolean
do_add_search_results (gpointer data)
{
  AddSearchResults *r = data;
  g_list_store_remove_all (r->store);
  add_search_results (r->store, r->full_query, r->query);
  return G_SOURCE_REMOVE;
}

static void
search_changed (DzlSuggestionEntry *entry,
                gpointer            user_data)
{
  g_autoptr(GListModel) model = NULL;
  GString *str = g_string_new (NULL);
  AddSearchResults *res = NULL;
  const gchar *text;
  gulong *handler = user_data;

  g_assert (DZL_IS_SUGGESTION_ENTRY (entry));
  g_assert (handler);

  text = dzl_suggestion_entry_get_typed_text (entry);

  for (const gchar *iter = text; *iter; iter = g_utf8_next_char (iter))
    {
      gunichar ch = g_utf8_get_char (iter);

      if (!g_unichar_isspace (ch))
        g_string_append_unichar (str, ch);
    }

  if (str->len)
    {
      GListModel *old_model = dzl_suggestion_entry_get_model (entry);

      if (old_model == NULL)
        {
          model = G_LIST_MODEL (g_list_store_new (DZL_TYPE_SUGGESTION));

          /* Update the model, but ignore selection events while
           * that happens so that we don't update the entry box.
           */
          g_signal_handler_block (entry, *handler);
          dzl_suggestion_entry_set_model (entry, model);
          g_signal_handler_unblock (entry, *handler);

          old_model = G_LIST_MODEL (model);
        }

      res = g_slice_new0 (AddSearchResults);
      res->store = g_object_ref (G_LIST_STORE (old_model));
      res->full_query = g_strdup (text);
      res->query = g_strdup (str->str);
    }

  /* Update the model asynchonrously to ensure we test that use case */
  if (res != NULL)
    g_timeout_add_full (G_PRIORITY_HIGH,
                        1,
                        do_add_search_results,
                        res,
                        (GDestroyNotify)add_search_results_free);

  g_string_free (str, TRUE);
}

static void
suggestion_activated (DzlSuggestionEntry *entry,
                      DzlSuggestion      *suggestion,
                      gpointer            user_data)
{
  const gchar *uri = dzl_suggestion_get_id (suggestion);

  g_print ("Activated selection: %s\n", uri);

  g_signal_handlers_block_by_func (entry, G_CALLBACK (search_changed), user_data);
  gtk_entry_set_text (GTK_ENTRY (entry), uri);
  g_signal_handlers_unblock_by_func (entry, G_CALLBACK (search_changed), user_data);

  g_signal_stop_emission_by_name (entry, "suggestion-activated");

  dzl_suggestion_entry_hide_suggestions (entry);
}

static void
suggestion_selected (DzlSuggestionEntry *entry,
                     DzlSuggestion      *suggestion,
                     gpointer            user_data)
{
  const gchar *uri = dzl_suggestion_get_id (suggestion);

  g_print ("Selected suggestion: %s\n", uri);

  g_signal_handlers_block_by_func (entry, G_CALLBACK (search_changed), user_data);
  gtk_entry_set_text (GTK_ENTRY (entry), uri);
  gtk_editable_set_position (GTK_EDITABLE (entry), -1);
  g_signal_handlers_unblock_by_func (entry, G_CALLBACK (search_changed), user_data);
}

static void
load_css (void)
{
  g_autoptr(GtkCssProvider) provider = NULL;

  provider = dzl_css_provider_new ("resource:///org/gnome/dazzle/themes");
  gtk_style_context_add_provider_for_screen (gdk_screen_get_default (),
                                             GTK_STYLE_PROVIDER (provider),
                                             GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

static void
notify_suggestion_cb (DzlSuggestionEntry *entry,
                      GParamSpec         *pspec,
                      gpointer            unused)
{
  DzlSuggestion *suggestion;

  g_assert (DZL_IS_SUGGESTION_ENTRY (entry));

  if ((suggestion = dzl_suggestion_entry_get_suggestion (entry)))
    {
      g_print  ("Suggestion changed to %s\n", dzl_suggestion_get_id (suggestion));
    }
  else
    {
      g_print ("Suggestion cleared\n");
    }
}

int
main (gint   argc,
      gchar *argv[])
{
  GtkWidget *window;
  GtkWidget *header;
  GtkWidget *entry;
  GtkWidget *main_entry;
  GtkWidget *box;
  GtkWidget *button;
  GtkWidget *scroller;
  GtkWidget *switch_;

  gtk_init (&argc, &argv);

  load_css ();

  search_index = dzl_fuzzy_mutable_index_new (FALSE);

  for (guint i = 0; i < G_N_ELEMENTS (demo_data); i++)
    {
      const DemoData *data = &demo_data[i];

      dzl_fuzzy_mutable_index_insert (search_index, data->url, (gpointer)data);
    }

  window = g_object_new (GTK_TYPE_WINDOW,
                         "default-width", 1100,
                         "default-height", 600,
                         NULL);
  header = g_object_new (GTK_TYPE_HEADER_BAR,
                         "show-close-button", TRUE,
                         "visible", TRUE,
                         NULL);

  scroller = g_object_new (GTK_TYPE_SCROLLED_WINDOW,
                           "child", g_object_new (GTK_TYPE_TEXT_VIEW,
                                                  "visible", TRUE,
                                                  NULL),
                           "expand", TRUE,
                           "visible", TRUE,
                           NULL);
  gtk_container_add (GTK_CONTAINER (window), scroller);

  box = g_object_new (GTK_TYPE_BOX,
                      "orientation", GTK_ORIENTATION_HORIZONTAL,
                      "visible", TRUE,
                      "spacing", 0,
                      NULL);
  gtk_style_context_add_class (gtk_widget_get_style_context (GTK_WIDGET (box)), "linked");

  entry = g_object_new (DZL_TYPE_SUGGESTION_ENTRY,
                        "activate-on-single-click", TRUE,
                        "halign", GTK_ALIGN_CENTER,
                        "hexpand", FALSE,
                        "max-width-chars", 55,
                        "visible", TRUE,
                        "width-chars", 30,
                        NULL);
  main_entry = entry;
  notify_suggestion_handler =
    g_signal_connect (entry,
                      "notify::suggestion",
                      G_CALLBACK (notify_suggestion_cb),
                      &notify_suggestion_handler);
#if 0
  dzl_suggestion_entry_set_position_func (DZL_SUGGESTION_ENTRY (entry),
                                          dzl_suggestion_entry_window_position_func,
                                          NULL, NULL);
#endif
  gtk_box_set_center_widget (GTK_BOX (box), entry);
  g_signal_connect (entry, "changed", G_CALLBACK (search_changed), &notify_suggestion_handler);
  g_signal_connect (entry, "suggestion-activated", G_CALLBACK (suggestion_activated), &notify_suggestion_handler);
  g_signal_connect (entry, "suggestion-selected", G_CALLBACK (suggestion_selected), &notify_suggestion_handler);

  button = g_object_new (GTK_TYPE_BUTTON,
                         "halign", GTK_ALIGN_START,
                         "hexpand", FALSE,
                         "visible", TRUE,
                         "child", g_object_new (GTK_TYPE_IMAGE,
                                                "visible", TRUE,
                                                "icon-name", "web-browser-symbolic",
                                                NULL),
                         NULL);
  gtk_box_pack_end (GTK_BOX (box), button, FALSE, FALSE, 0);

  gtk_window_set_titlebar (GTK_WINDOW (window), header);
  gtk_header_bar_set_custom_title (GTK_HEADER_BAR (header), box);

  g_signal_connect (window, "delete-event", gtk_main_quit, NULL);
  g_signal_connect (window, "key-press-event", G_CALLBACK (key_press), entry);

  button = dzl_suggestion_button_new ();
  entry = GTK_WIDGET (dzl_suggestion_button_get_entry (DZL_SUGGESTION_BUTTON (button)));
  button_handler = g_signal_connect (entry, "changed", G_CALLBACK (search_changed), &button_handler);
  g_signal_connect (entry, "suggestion-activated", G_CALLBACK (suggestion_activated), &button_handler);
  g_signal_connect (entry, "suggestion-selected", G_CALLBACK (suggestion_selected), &button_handler);
  g_signal_connect (entry, "notify::suggestion", G_CALLBACK (notify_suggestion_cb), &button_handler);
  gtk_container_add (GTK_CONTAINER (header), button);
  gtk_widget_show (button);

  box = g_object_new (GTK_TYPE_BOX,
                      "orientation", GTK_ORIENTATION_HORIZONTAL,
                      "visible", TRUE,
                      "spacing", 6,
                      NULL);
  gtk_container_add (GTK_CONTAINER (header), box);
  switch_ = g_object_new (GTK_TYPE_SWITCH,
                          "visible", TRUE,
                          NULL);
  g_object_bind_property (G_OBJECT (switch_), "active", main_entry, "compact",
                          G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);
  gtk_container_add (GTK_CONTAINER (box),
                     g_object_new (GTK_TYPE_LABEL,
                                   "label", "Compact Mode:",
                                   "visible", TRUE,
                                   NULL));
  gtk_container_add (GTK_CONTAINER (box), switch_);

  gtk_window_present (GTK_WINDOW (window));

  gtk_main ();

  return 0;
}
