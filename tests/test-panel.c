#include <dazzle.h>

typedef enum
{
   COMMAND_SHOW,
   COMMAND_HIDE,
   COMMAND_EXIT
} Command;

#define INTERVAL 500

static struct {
  gint interval;
  Command command;
} commands[] = {
  { 1000, COMMAND_SHOW },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { 300, COMMAND_EXIT },
};
static gint current_command;
static GtkWidget *dockbin;
static GtkWidget *class_entry;
static GTimer *timer;

static void
toggle_all (gboolean show)
{
  DzlDockRevealer *edge;
  DzlDockBin *dock = DZL_DOCK_BIN (dockbin);

  edge = DZL_DOCK_REVEALER (dzl_dock_bin_get_left_edge (dock));
  dzl_dock_revealer_set_reveal_child (edge, show);

  edge = DZL_DOCK_REVEALER (dzl_dock_bin_get_right_edge (dock));
  dzl_dock_revealer_set_reveal_child (edge, show);

  edge = DZL_DOCK_REVEALER (dzl_dock_bin_get_bottom_edge (dock));
  dzl_dock_revealer_set_reveal_child (edge, show);
}

static void
adjust_sizes (void)
{
  DzlDockBin *dock = DZL_DOCK_BIN (dockbin);
  GtkWidget *edge;

  edge = dzl_dock_bin_get_left_edge (dock);
  dzl_dock_revealer_set_position (DZL_DOCK_REVEALER (edge), 300);

  edge = dzl_dock_bin_get_right_edge (dock);
  dzl_dock_revealer_set_position (DZL_DOCK_REVEALER (edge), 300);

  edge = dzl_dock_bin_get_bottom_edge (dock);
  dzl_dock_revealer_set_position (DZL_DOCK_REVEALER (edge), 300);
}

static void
grab_class_entry (GtkWidget *button)
{
  gtk_widget_grab_focus (class_entry);
}

static gboolean
process_command (gpointer data)
{
  switch (commands [current_command].command)
    {
    case COMMAND_SHOW:
      toggle_all (TRUE);
      break;

    case COMMAND_HIDE:
      toggle_all (FALSE);
      break;

    case COMMAND_EXIT:
    default:
      gtk_main_quit ();
      return G_SOURCE_REMOVE;
    }

  g_timeout_add (commands [++current_command].interval,
                 process_command,
                 NULL);

  return G_SOURCE_REMOVE;
}

static void
log_handler (const gchar    *domain,
             GLogLevelFlags  flags,
             const gchar    *message,
             gpointer        user_data)
{
  gdouble t = g_timer_elapsed (timer, NULL);

  g_print ("%s: time=%0.5lf %s\n", domain, t, message);
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

gint
main (gint   argc,
      gchar *argv[])
{
  GtkBuilder *builder = NULL;
  GtkWindow *window = NULL;
  GActionGroup *group;
  GError *error = NULL;
  g_autofree gchar *ui_path = g_build_filename (TEST_DATA_DIR, "test-panel.ui", NULL);

  gtk_init (&argc, &argv);

  load_css ();

  timer = g_timer_new ();

  builder = gtk_builder_new ();
  gtk_builder_add_from_file (builder, ui_path, &error);
  g_assert_no_error (error);

  window = GTK_WINDOW (gtk_builder_get_object (builder, "window"));
  g_signal_connect (window, "delete-event", gtk_main_quit, NULL);

  dockbin = GTK_WIDGET (gtk_builder_get_object (builder, "dockbin"));
  /* _dzl_dock_item_printf (DZL_DOCK_ITEM (dockbin)); */

  group = gtk_widget_get_action_group (dockbin, "dockbin");
  gtk_widget_insert_action_group (GTK_WIDGET (window), "dockbin", group);

  adjust_sizes ();

  class_entry = GTK_WIDGET (gtk_builder_get_object (builder, "class_entry"));

#if 0
  gtk_builder_add_callback_symbol (builder, "toggle_all", G_CALLBACK (toggle_all));
#endif
  gtk_builder_add_callback_symbol (builder, "grab_class_entry", G_CALLBACK (grab_class_entry));
  gtk_builder_connect_signals (builder, dockbin);

  if (0)
  g_timeout_add (commands [0].interval,
                 process_command,
                 NULL);

  g_log_set_default_handler (log_handler, NULL);

  gtk_widget_grab_focus (GTK_WIDGET (gtk_builder_get_object (builder, "text_view")));

  gtk_window_maximize (window);
  gtk_window_present (window);
  gtk_main ();

  g_clear_object (&builder);

  return 0;
}
