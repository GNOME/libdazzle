/* dzl-cancellable.c
 *
 * Copyright © 2017 Christian Hergert <chergert@redhat.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define G_LOG_DOMAIN "dzl-cancellable"

#include <gio/gio.h>

#include "util/dzl-cancellable.h"

static void
dzl_cancellable_cancelled_cb (GCancellable *dep,
                              GCancellable *parent)
{
  g_assert (G_IS_CANCELLABLE (dep));
  g_assert (G_IS_CANCELLABLE (parent));

  if (!g_cancellable_is_cancelled (parent))
    g_cancellable_cancel (parent);
}

/**
 * dzl_cancellable_chain:
 * @self: (nullable): a #GCancellable or %NULL
 * @other: (nullable): a #GCancellable or %NULL
 *
 * If both @self and @other are not %NULL, then the cancellation of
 * @other will be propagated to @self if @other is cancelled.
 *
 * Since: 3.28
 */
void
dzl_cancellable_chain (GCancellable *self,
                       GCancellable *other)
{
  g_return_if_fail (!self || G_IS_CANCELLABLE (self));
  g_return_if_fail (!other || G_IS_CANCELLABLE (other));

  if (self == NULL || other == NULL)
    return;

  g_cancellable_connect (other,
                         G_CALLBACK (dzl_cancellable_cancelled_cb),
                         g_object_ref (self),
                         g_object_unref);
}
