#include <dazzle.h>

static void
toggle_bold (GtkToggleButton *button,
             DzlBoldingLabel *label)
{
  gboolean is_bold = gtk_toggle_button_get_active (button);

  dzl_bolding_label_set_weight (label, is_bold ? PANGO_WEIGHT_BOLD : PANGO_WEIGHT_NORMAL);
}

int main (int argc, char *argv[])
{
  GtkWidget *window;
  GtkWidget *box;
  GtkWidget *label;
  GtkWidget *button;

  gtk_init (&argc, &argv);

  window = g_object_new (GTK_TYPE_WINDOW,
                         "border-width", 24,
                         "title", "Test Entry Box",
                         NULL);

  box = g_object_new (GTK_TYPE_BOX,
                      "spacing", 12,
                      "valign", GTK_ALIGN_START,
                      "visible", TRUE,
                      NULL);
  gtk_container_add (GTK_CONTAINER (window), box);

  label = g_object_new (DZL_TYPE_BOLDING_LABEL,
                        "label", "toggle to bold",
                        "visible", TRUE,
                        NULL);
  gtk_container_add (GTK_CONTAINER (box), label);

  button = g_object_new (GTK_TYPE_TOGGLE_BUTTON,
                         "child", g_object_new (GTK_TYPE_IMAGE,
                                                "icon-name", "format-text-bold-symbolic",
                                                "visible", TRUE,
                                                NULL),
                         "visible", TRUE,
                         NULL);
  gtk_container_add_with_properties (GTK_CONTAINER (box), button,
                                     "pack-type", GTK_PACK_END,
                                     NULL);

  g_signal_connect (button, "clicked", G_CALLBACK (toggle_bold), label);
  g_signal_connect (window, "delete-event", gtk_main_quit, NULL);
  gtk_window_present (GTK_WINDOW (window));
  gtk_main ();

  return 0;
}
