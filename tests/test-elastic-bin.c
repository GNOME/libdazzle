#include <dazzle.h>

static GtkWidget *list_box;

static void
add_row (void)
{
  static guint counter;
  g_autofree gchar *text = g_strdup_printf ("%u", ++counter);
  GtkWidget *row;

  row = g_object_new (GTK_TYPE_LABEL,
                      "label", text,
                      "visible", TRUE,
                      NULL);
  gtk_container_add (GTK_CONTAINER (list_box), row);
}

gint
main (gint argc,
      gchar *argv[])
{
  GtkWidget *window;
  GtkWidget *box;
  GtkWidget *button;
  GtkWidget *scroller;
  GtkWidget *bin;

  gtk_init (&argc, &argv);

  window = g_object_new (GTK_TYPE_WINDOW,
                         "title", "test-elastic",
                         "default-width", 300,
                         "resizable", FALSE,
                         NULL);

  box = g_object_new (GTK_TYPE_BOX,
                      "orientation", GTK_ORIENTATION_VERTICAL,
                      "visible", TRUE,
                      NULL);
  gtk_container_add (GTK_CONTAINER (window), box);

  button = g_object_new (GTK_TYPE_BUTTON,
                         "label", "Add row",
                         "visible", TRUE,
                         NULL);
  g_signal_connect (button, "clicked", add_row, NULL);
  gtk_container_add (GTK_CONTAINER (box), button);

  bin = g_object_new (DZL_TYPE_ELASTIC_BIN,
                      "vexpand", TRUE,
                      "visible", TRUE,
                      NULL);
  gtk_container_add (GTK_CONTAINER (box), bin);

  scroller = g_object_new (GTK_TYPE_SCROLLED_WINDOW,
                           "propagate-natural-height", TRUE,
                           "visible", TRUE,
                           NULL);
  gtk_container_add (GTK_CONTAINER (bin), scroller);

  list_box = g_object_new (GTK_TYPE_LIST_BOX,
                           "visible", TRUE,
                           NULL);
  gtk_container_add (GTK_CONTAINER (scroller), list_box);

  g_signal_connect (window, "delete-event", gtk_main_quit, NULL);
  gtk_window_present (GTK_WINDOW (window));
  gtk_main ();

  return 0;
}
