#include <dazzle.h>

#define PTR1 GINT_TO_POINTER(1234)
#define PTR2 GINT_TO_POINTER(4321)

static void
test_dzl_shortcut_chord_basic (void)
{
  DzlShortcutChord *chord;
  DzlShortcutChord *chord2;
  DzlShortcutChordTable *table;
  DzlShortcutMatch match;
  gboolean r;
  gpointer data = NULL;

  chord = dzl_shortcut_chord_new_from_string ("<control>p");
  g_assert (chord != NULL);

  table = dzl_shortcut_chord_table_new ();
  dzl_shortcut_chord_table_add (table, chord, PTR1);

  match = dzl_shortcut_chord_table_lookup (table, chord, &data);
  g_assert (PTR1 == data);
  g_assert_cmpint (match, ==, DZL_SHORTCUT_MATCH_EQUAL);

  r = dzl_shortcut_chord_table_remove (table, chord);
  g_assert_cmpint (r, ==, TRUE);

  r = dzl_shortcut_chord_table_remove (table, chord);
  g_assert_cmpint (r, ==, FALSE);

  match = dzl_shortcut_chord_table_lookup (table, chord, &data);
  g_assert (data == NULL);
  g_assert_cmpint (match, ==, DZL_SHORTCUT_MATCH_NONE);

  chord2 = dzl_shortcut_chord_new_from_string ("<control>p|<control>a");
  g_assert (chord2 != NULL);
  dzl_shortcut_chord_table_add (table, chord2, PTR2);
  g_assert (dzl_shortcut_chord_equal (chord2, dzl_shortcut_chord_table_lookup_data (table, PTR2)));

  match = dzl_shortcut_chord_table_lookup (table, chord, &data);
  g_assert (data == NULL);
  g_assert_cmpint (match, ==, DZL_SHORTCUT_MATCH_PARTIAL);

  match = dzl_shortcut_chord_table_lookup (table, chord2, &data);
  g_assert (data == PTR2);
  g_assert_cmpint (match, ==, DZL_SHORTCUT_MATCH_EQUAL);

  g_assert (dzl_shortcut_chord_equal (chord2, dzl_shortcut_chord_table_lookup_data (table, PTR2)));
  r = dzl_shortcut_chord_table_remove (table, chord2);
  g_assert_cmpint (r, ==, TRUE);
  g_assert (NULL == dzl_shortcut_chord_table_lookup_data (table, PTR2));

  r = dzl_shortcut_chord_table_remove (table, chord2);
  g_assert_cmpint (r, ==, FALSE);

  g_clear_pointer (&chord, dzl_shortcut_chord_free);
  g_clear_pointer (&chord2, dzl_shortcut_chord_free);
  g_clear_pointer (&table, dzl_shortcut_chord_table_free);
}

gint
main (gint   argc,
      gchar *argv[])
{
  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/Dazzle/Shortcut/Chord", test_dzl_shortcut_chord_basic);

  return g_test_run ();
}
