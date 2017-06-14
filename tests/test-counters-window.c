#include <dazzle.h>
#include <stdlib.h>
#include <errno.h>

static gboolean
int_parse_with_range (gint        *value,
                      gint         lower,
                      gint         upper,
                      const gchar *str)
{
  gint64 v64;

  g_assert (value);
  g_assert (lower <= upper);

  v64 = g_ascii_strtoll (str, NULL, 10);

  if (((v64 == G_MININT64) || (v64 == G_MAXINT64)) && (errno == ERANGE))
    return FALSE;

  if ((v64 < lower) || (v64 > upper))
    return FALSE;

  *value = (gint)v64;

  return TRUE;
}

gint
main (gint argc,
      gchar *argv[])
{
  GtkWidget *window;
  DzlCounterArena *arena;
  gint pid;

  gtk_init (&argc, &argv);

  if (argc < 2)
    {
      g_printerr ("usage: %s [PID]\n", argv[0]);
      return EXIT_FAILURE;
    }

  if (!int_parse_with_range (&pid, 1, G_MAXUSHORT, argv[1]))
    {
      g_printerr ("usage: %s [PID]\n", argv[0]);
      return EXIT_FAILURE;
    }

  arena = dzl_counter_arena_new_for_pid (pid);

  if (arena == NULL)
    {
      g_printerr ("Failed to access counters for process %u.\n", pid);
      return EXIT_FAILURE;
    }

  window = dzl_counters_window_new ();
  dzl_counters_window_set_arena (DZL_COUNTERS_WINDOW (window), arena);
  g_signal_connect (window, "delete-event", gtk_main_quit, NULL);
  gtk_window_present (GTK_WINDOW (window));

  gtk_main ();

  return EXIT_SUCCESS;
}
