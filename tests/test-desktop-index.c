#include <dazzle.h>
#include <stdlib.h>

static GTimer *timer;
static gchar *last_query;

static void
query_cb (GObject      *object,
          GAsyncResult *result,
          gpointer      user_data)
{
  g_autoptr(GListModel) model = NULL;
  DzlFuzzyIndex *index = (DzlFuzzyIndex *)object;
  g_autoptr(GError) error = NULL;
  g_autoptr(GListStore) suggestions = NULL;
  g_autoptr(DzlSuggestionEntry) entry = user_data;
  g_autoptr(GHashTable) hash = NULL;
  guint n_items;

  model = dzl_fuzzy_index_query_finish (index, result, &error);

  if (error)
    {
      g_printerr ("%s\n", error->message);
      return;
    }

  suggestions = g_list_store_new (DZL_TYPE_SUGGESTION);

  n_items = g_list_model_get_n_items (model);

  hash = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

  for (guint i = 0; i < n_items; i++)
    {
      g_autoptr(DzlFuzzyIndexMatch) match = g_list_model_get_item (model, i);
      GVariant *doc = dzl_fuzzy_index_match_get_document (match);
      g_autoptr(DzlSuggestion) suggestion = NULL;
      g_autoptr(GVariantDict) dict = g_variant_dict_new (doc);
      const gchar *id = NULL;
      const gchar *title = NULL;
      const gchar *icon_name = NULL;
      g_autofree gchar *highlight = NULL;
      g_autofree gchar *escaped = NULL;
      g_autofree gchar *subtitle = NULL;
      g_autofree gchar *escape_keyword = NULL;

      if (!g_variant_dict_lookup (dict, "id", "&s", &id))
        id = NULL;
      if (!g_variant_dict_lookup (dict, "title", "&s", &title))
        title = NULL;
      if (!g_variant_dict_lookup (dict, "icon-name", "&s", &icon_name))
        icon_name = NULL;

      if (g_hash_table_contains (hash, id))
        continue;
      else
        g_hash_table_insert (hash, g_strdup (id), NULL);

      escaped = g_markup_escape_text (title, -1);
      highlight = dzl_fuzzy_highlight (escaped, last_query, FALSE);
      escape_keyword = g_markup_escape_text (dzl_fuzzy_index_match_get_key (match), -1);
      subtitle = g_strdup_printf ("%lf (%s) (priority %u)",
                                  dzl_fuzzy_index_match_get_score (match),
                                  escape_keyword,
                                  dzl_fuzzy_index_match_get_priority (match));

      suggestion = g_object_new (DZL_TYPE_SUGGESTION,
                                 "id", id,
                                 "icon-name", icon_name,
                                 "title", highlight,
                                 "subtitle", subtitle,
                                 NULL);

      g_list_store_append (suggestions, suggestion);
    }

  dzl_suggestion_entry_set_model (entry, G_LIST_MODEL (suggestions));
}

static void
entry_changed (DzlSuggestionEntry *entry,
               DzlFuzzyIndex      *index)
{
  const gchar *typed_text;
  GString *str = g_string_new (NULL);

  g_assert (DZL_IS_SUGGESTION_ENTRY (entry));
  g_assert (DZL_IS_FUZZY_INDEX (index));

  typed_text = dzl_suggestion_entry_get_typed_text (entry);

  g_timer_reset (timer);

  for (; *typed_text; typed_text = g_utf8_next_char (typed_text))
    {
      gunichar ch = g_utf8_get_char (typed_text);

      if (!g_unichar_isspace (ch))
        g_string_append_unichar (str, ch);
    }

  dzl_fuzzy_index_query_async (index, str->str, 25, NULL, query_cb, g_object_ref (entry));

  g_free (last_query);
  last_query = g_string_free (str, FALSE);
}

static void
create_ui (void)
{
  GtkWindow *window;
  GtkHeaderBar *header;
  GtkWidget *entry;
  g_autoptr(DzlFuzzyIndex) index = dzl_fuzzy_index_new ();
  g_autoptr(GFile) file = g_file_new_for_path ("desktop.index");
  g_autoptr(GError) error = NULL;
  g_autoptr(GtkCssProvider) provider = NULL;
  gboolean r;

  provider = dzl_css_provider_new ("resource:///org/gnome/dazzle/themes");
  gtk_style_context_add_provider_for_screen (gdk_screen_get_default (),
                                             GTK_STYLE_PROVIDER (provider),
                                             GTK_STYLE_PROVIDER_PRIORITY_APPLICATION-1);

  timer = g_timer_new ();

  r = dzl_fuzzy_index_load_file (index, file, NULL, &error);
  g_assert_no_error (error);
  g_assert (r);

  window = g_object_new (GTK_TYPE_WINDOW,
                         "title", "Title Window",
                         "default-width", 800,
                         "default-height", 600,
                         NULL);

  header = g_object_new (GTK_TYPE_HEADER_BAR,
                         "show-close-button", TRUE,
                         "visible", TRUE,
                         NULL);
  gtk_window_set_titlebar (window, GTK_WIDGET (header));

  entry = g_object_new (DZL_TYPE_SUGGESTION_ENTRY,
                        "visible", TRUE,
                        "max-width-chars", 55,
                        NULL);
  gtk_header_bar_set_custom_title (header, entry);

  g_signal_connect_data (entry,
                         "changed",
                         G_CALLBACK (entry_changed),
                         g_steal_pointer (&index),
                         (GClosureNotify)g_object_unref,
                         0);

  gtk_widget_grab_focus (entry);
  g_signal_connect (window, "delete-event", gtk_main_quit, NULL);
  gtk_window_present (window);

}

static gchar **
tokenize (const gchar *input)
{
  return g_strsplit_set (input, ",;: -_./\t\n", 0);
}

static void
test_desktop_index_key (DzlFuzzyIndexBuilder *builder,
                        GKeyFile             *key_file,
                        const gchar          *group,
                        const gchar          *key,
                        GVariant             *document,
                        gboolean              can_tokenize,
                        gint                  priority)
{
  g_autofree gchar *str = NULL;
  g_auto(GStrv) words = NULL;

  g_assert (DZL_IS_FUZZY_INDEX_BUILDER (builder));
  g_assert (group);
  g_assert (key);
  g_assert (document);

  if (NULL == (str = g_key_file_get_string (key_file, group, key, NULL)))
    return;

  if (!can_tokenize)
    {
      dzl_fuzzy_index_builder_insert (builder, str, document, priority);
      return;
    }

  words = tokenize (str);

  for (guint i = 0; words[i] != NULL; i++)
    {
      if (*words[i] && !g_ascii_isspace (*words[i]))
        dzl_fuzzy_index_builder_insert (builder, words[i], document, priority);
    }
}

static gboolean
test_desktop_index_file (DzlFuzzyIndexBuilder  *builder,
                         const gchar           *path,
                         const gchar           *name,
                         GError               **error)
{
  g_autoptr(GKeyFile) key_file = NULL;
  g_autoptr(GVariant) document = NULL;
  g_autofree gchar *icon = NULL;
  g_autofree gchar *entry_name = NULL;
  GVariantDict dict;

  g_assert (DZL_IS_FUZZY_INDEX_BUILDER (builder));
  g_assert (path != NULL);

  key_file = g_key_file_new ();

  if (!g_key_file_load_from_file (key_file, path, 0, error))
    return FALSE;

  if (!g_key_file_has_group (key_file, "Desktop Entry"))
    {
      g_set_error (error,
                   G_IO_ERROR,
                   G_IO_ERROR_NOT_SUPPORTED,
                   "%s is not a desktop file",
                   path);
      return FALSE;
    }

  icon = g_key_file_get_string (key_file, "Desktop Entry", "Icon", NULL);
  entry_name = g_key_file_get_string (key_file, "Desktop Entry", "Name", NULL);

  g_variant_dict_init (&dict, NULL);
  g_variant_dict_insert (&dict, "id", "s", name ?: "");
  g_variant_dict_insert (&dict, "icon-name", "s", icon ?: "");
  g_variant_dict_insert (&dict, "title", "s", entry_name ?: "");
  document = g_variant_take_ref (g_variant_dict_end (&dict));

  test_desktop_index_key (builder, key_file, "Desktop Entry", "Name", document, FALSE, 0);
  test_desktop_index_key (builder, key_file, "Desktop Entry", "Name", document, TRUE, 1);
  test_desktop_index_key (builder, key_file, "Desktop Entry", "GenericName", document, TRUE, 2);
  test_desktop_index_key (builder, key_file, "Desktop Entry", "Keywords", document, TRUE, 3);
  test_desktop_index_key (builder, key_file, "Desktop Entry", "Comment", document, TRUE, 4);
  test_desktop_index_key (builder, key_file, "Desktop Entry", "Categories", document, TRUE, 5);

  return TRUE;
}

static void
fsck_index (void)
{
  g_autofree gchar *contents = NULL;
  g_autoptr(GVariant) variant = NULL;
  g_autoptr(GVariantIter) iter = NULL;
  const gchar *key;
  GVariant *value;
  GVariantDict dict;
  gsize len;
  GError *error = NULL;
  gboolean r;

  r = g_file_get_contents ("desktop.index", &contents, &len, &error);
  g_assert_no_error (error);
  g_assert (r);

  variant = g_variant_new_from_data (G_VARIANT_TYPE_VARDICT, contents, len, FALSE, NULL, NULL);
  g_assert (variant != NULL);
  g_variant_take_ref (variant);

  g_variant_dict_init (&dict, variant);

  r = g_variant_dict_lookup (&dict, "tables", "a{sv}" , &iter);
  g_assert (r);

  while (g_variant_iter_loop (iter, "{sv}", &key, &value))
    {
      g_autoptr(GVariantIter) aiter = NULL;
      guint last_key_id = 0;
      gint last_offset = -1;
      guint key_id;
      guint offset;

      aiter = g_variant_iter_new (value);
      g_assert (aiter != NULL);

      while (g_variant_iter_loop (aiter, "(uu)", &offset, &key_id))
        {
          g_assert_cmpint (key_id, >=, last_key_id);
          if (key_id == last_key_id)
            g_assert_cmpint (offset, >, last_offset);
          last_key_id = key_id;
          last_offset = offset;
        }
    }

  g_variant_dict_clear (&dict);
}

gint
main (gint   argc,
      gchar *argv[])
{
  g_autoptr(GOptionContext) context = NULL;
  g_autoptr(DzlFuzzyIndexBuilder) builder = NULL;
  g_autoptr(GFile) outfile = NULL;
  g_autoptr(GError) error = NULL;
  g_autoptr(GPtrArray) ar = NULL;

  context = g_option_context_new ("[DIRECTORIES...] - Index desktop info directories");
  g_option_context_add_group (context, gtk_get_option_group (TRUE));

  if (!g_option_context_parse (context, &argc, &argv, &error))
    {
      g_printerr ("%s\n", error->message);
      return EXIT_FAILURE;
    }

  builder = dzl_fuzzy_index_builder_new ();

  dzl_fuzzy_index_builder_set_case_sensitive (builder, FALSE);

  ar = g_ptr_array_new ();
  for (guint i = 1; i < argc; i++)
    g_ptr_array_add (ar, argv[i]);
  if (ar->len == 0)
    g_ptr_array_add (ar, (gchar *)"/usr/share/applications");

  for (guint i = 0; i < ar->len; i++)
    {
      const gchar *name;
      const gchar *directory = g_ptr_array_index (ar, i);
      g_autoptr(GDir) dir = g_dir_open (directory, 0, &error);

      if (dir == NULL)
        {
          g_printerr ("%s\n", error->message);
          g_clear_error (&error);
          continue;
        }

      while (NULL != (name = g_dir_read_name (dir)))
        {
          g_autofree gchar *path = NULL;

          if (!g_str_has_suffix (name, ".desktop"))
            continue;

          path = g_build_filename (directory, name, NULL);

          if (!test_desktop_index_file (builder, path, name, &error))
            {
              g_printerr ("%s\n", error->message);
              g_clear_error (&error);
              continue;
            }
        }
    }

  outfile = g_file_new_for_path ("desktop.index");

  if (!dzl_fuzzy_index_builder_write (builder, outfile, G_PRIORITY_DEFAULT, NULL, &error))
    g_printerr ("%s\n", error->message);

  g_print ("desktop.index written\n");

  fsck_index ();
  create_ui ();
  gtk_main ();

  return EXIT_SUCCESS;
}
