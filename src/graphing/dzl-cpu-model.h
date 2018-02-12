/* dzl-cpu-model.h
 *
 * Copyright (C) 2015 Christian Hergert <christian@hergert.me>
 *
 * This file is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DZL_CPU_MODEL_H
#define DZL_CPU_MODEL_H

#include "dzl-version-macros.h"

#include "dzl-graph-model.h"

G_BEGIN_DECLS

#define DZL_TYPE_CPU_MODEL (dzl_cpu_model_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (DzlCpuModel, dzl_cpu_model, DZL, CPU_MODEL, DzlGraphModel)

DZL_AVAILABLE_IN_ALL
DzlGraphModel *dzl_cpu_model_new (void);

G_END_DECLS

#endif /* DZL_CPU_MODEL_H */
