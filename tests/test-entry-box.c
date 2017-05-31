#include <dazzle.h>

int main (int argc, char *argv[])
{
  GtkWidget *window;
  GtkWidget *entry_box;
  GtkWidget *label;
  GtkWidget *icon;

  gtk_init (&argc, &argv);

  window = g_object_new (GTK_TYPE_WINDOW,
                         "border-width", 24,
                         "title", "Test Entry Box",
                         NULL);

  entry_box = g_object_new (DZL_TYPE_ENTRY_BOX,
                            "max-width-chars", 55,
                            "spacing", 12,
                            "valign", GTK_ALIGN_START,
                            "visible", TRUE,
                            NULL);
  gtk_container_add (GTK_CONTAINER (window), entry_box);

  label = g_object_new (GTK_TYPE_LABEL,
                        "label", "Frobnicate",
                        "visible", TRUE,
                        NULL);
  gtk_container_add (GTK_CONTAINER (entry_box), label);

  icon = g_object_new (DZL_TYPE_PROGRESS_ICON,
                       "progress", .33,
                       "visible", TRUE,
                       NULL);
  gtk_container_add_with_properties (GTK_CONTAINER (entry_box), icon,
                                     "pack-type", GTK_PACK_END,
                                     NULL);

  g_signal_connect (window, "delete-event", gtk_main_quit, NULL);
  gtk_window_present (GTK_WINDOW (window));
  gtk_main ();

  return 0;
}
