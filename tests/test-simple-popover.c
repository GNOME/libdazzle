#include <dazzle.h>

static void
text_changed (DzlSimplePopover *popover,
              gpointer          unused)
{
  const gchar *text;

  text = dzl_simple_popover_get_text (popover);
  dzl_simple_popover_set_ready (popover, text && *text);
}

static void
activate_cb (DzlSimplePopover *popover,
             const gchar      *text,
             GtkButton        *button)
{
  g_assert (DZL_IS_SIMPLE_POPOVER (popover));
  g_assert (GTK_IS_BUTTON (button));

  gtk_button_set_label (button, text);
}

gint
main (gint argc,
      gchar *argv[])
{
  DzlSimplePopover *popover;
  GtkMenuButton *button;
  GtkWindow *window;

  gtk_init (&argc, &argv);

  window = g_object_new (GTK_TYPE_WINDOW,
                         "title", "Test Simple Popover",
                         "border-width", 24,
                         "visible", TRUE,
                         NULL);

  popover = g_object_new (DZL_TYPE_SIMPLE_POPOVER,
                          "title", "Change Label",
                          "message", "Type the new text for the label",
                          "button-text", "Change",
                          NULL);

  button = g_object_new (GTK_TYPE_MENU_BUTTON,
                         "label", "Click Me…",
                         "popover", popover,
                         "visible", TRUE,
                         NULL);
  gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (button));

  g_signal_connect (popover, "changed", G_CALLBACK (text_changed), NULL);
  g_signal_connect (popover, "activate", G_CALLBACK (activate_cb), button);
  g_signal_connect (window, "delete-event", gtk_main_quit, NULL);

  gtk_main ();

  return 0;
}
