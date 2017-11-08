/* dzl-pattern-spec.h
 *
 * Copyright (C) 2015-2017 Christian Hergert <christian@hergert.me>
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

#ifndef DZL_MATCH_PATTERN_H
#define DZL_MATCH_PATTERN_H

#include <glib-object.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

typedef struct _DzlPatternSpec DzlPatternSpec;

#define DZL_TYPE_PATTERN_SPEC (dzl_pattern_spec_get_type())

DZL_AVAILABLE_IN_ALL
GType           dzl_pattern_spec_get_type (void);
DZL_AVAILABLE_IN_ALL
DzlPatternSpec *dzl_pattern_spec_new      (const gchar    *keywords);
DZL_AVAILABLE_IN_ALL
DzlPatternSpec *dzl_pattern_spec_ref      (DzlPatternSpec *self);
DZL_AVAILABLE_IN_ALL
void            dzl_pattern_spec_unref    (DzlPatternSpec *self);
DZL_AVAILABLE_IN_ALL
gboolean        dzl_pattern_spec_match    (DzlPatternSpec *self,
                                           const gchar     *haystack);
DZL_AVAILABLE_IN_ALL
const gchar    *dzl_pattern_spec_get_text (DzlPatternSpec *self);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (DzlPatternSpec, dzl_pattern_spec_unref)

G_END_DECLS

#endif /* DZL_MATCH_PATTERN_H */
