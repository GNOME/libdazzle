#include <dazzle.h>

typedef struct
{
  const gchar *needle;
  const gchar *haystack;
  gint         expected_score;
} WordCheck;

static void
test_levenshtein_basic (void)
{
  static const WordCheck check[] = {
    { "gtk", "gkt", 2 },
    { "LibreFreeOpen", "Cromulent", 10 },
    { "Xorg", "Wayland", 7 },
    { "glib", "gobject", 6 },
    { "gbobject", "gobject", 1 },
    { "flip", "fliiiip", 3 },
    { "flip", "fliiiipper", 6 },
    { NULL }
  };

  for (guint i = 0; check[i].needle != NULL; i++)
    {
      gint score = dzl_levenshtein (check[i].needle, check[i].haystack);

      g_assert_cmpint (score, ==, check[i].expected_score);
    }
}

gint
main (gint argc,
      gchar *argv[])
{
  g_test_init (&argc, &argv, NULL);
  g_test_add_func ("/Dazzle/Levenshtein/basic", test_levenshtein_basic);
  return g_test_run ();
}
