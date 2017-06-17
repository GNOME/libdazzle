# Contributing

## Licensing

Your work is considered a derivative work of the libdazzle codebase and therefore must be licensed as GPLv3+.
You may optionally grant other licensing in addition to GPLv3+ such as LGPLv2.1+ or MIT X11.
However, as part of libdazzle, which is GPLv3+, the combined work will be GPLv3+.

You do not need to assign us copyright attribution.
It is our belief that you should always retain copyright on your own work.

## Testing

When working on a new widget or tool, try to write unit tests to prove the implementation.
Not everything we have in the code base has tests, and ideally that will improve, not get worse.

## Troubleshooting

If you configure the meson project with `-Denable_tracing=true` then libdazzle with be built with tracing.
This allows various parts of the code to use `DZL_ENTRY`, `DZL_EXIT` and other tracing macros to log function calls.
You might find this useful in tracking down difficult re-entrancy or simply learn "how does this work".

If you need to add additional tracing macros to debug a problem, it is probably a good idea to submit a patch to add them.
Chances are someone else will need to debug stuff in the future.

## Code Style

We follow the GObject and Gtk coding style.
That is often unfamiliar to those who have not been hacking on GNU projects for the past couple of decades.
However, it is largely entrenched in our community, so we try to be consistent.

```c
static gboolean
this_is_a_function (GtkWidget    *param,
                    const gchar  *another_param,
                    guint         third_param,
                    GError      **error)
{
  g_return_val_if_fail (GTK_IS_WIDGET (param), FALSE);
  g_return_val_if_fail (third_param > 10, FALSE);

  if (another_param != NULL)
    {
      if (!do_some_more_work ())
        {
          g_set_error (error,
                       G_IO_ERROR,
                       G_IO_ERROR_FAILED,
                       "Something failed");
          return FALSE;
        }
    }

goto_labels_here:

  return TRUE;
}
```

```c
void      do_something_one   (GtkWidget  *widget);
void      do_something_two   (GtkWidget  *widget,
                              GError    **error);
gchar    *do_something_three (GtkWidget  *widget);
gboolean  do_something_four  (GtkWidget  *widget);
```

 * Notice that we use 2-space indention.
 * We indent new blocks {} with 2 spaces, and braces on their own line. We understand that this is confusing at first, but it is rather nice once it becomes familiar.
 * No tabs, spaces only.
 * Always compare against `NULL` rather than implicit comparisons. This eases ports to other languages and readability.
 * Use #define for constants. Try to avoid "magic constants".
 * goto labels are fully unindented.
 * Align function parameters.
 * Align blocks of function declarations in the header. This vastly improves readability and scanning to find what you want.

If in doubt, look for examples elsewhere in the codebase.

