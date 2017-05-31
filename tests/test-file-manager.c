#include <dazzle.h>
#include <stdlib.h>

int main (int argc, char *argv[])
{
  g_autoptr(GFile) file = NULL;
  g_autoptr(GError) error = NULL;

  if (argc == 1)
    {
      g_printerr ("usage: %s FILENAME\n", argv[0]);
      return EXIT_FAILURE;
    }

  file = g_file_new_for_commandline_arg (argv [1]);

  if (!dzl_file_manager_show (file, &error))
    {
      g_printerr ("%s\n", error->message);
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
