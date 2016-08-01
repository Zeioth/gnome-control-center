/* um-arrow-frame.c
 *
 * Copyright (C) 2016 Red Hat, Inc,
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
 *
 * Writen by: Felipe Borges <felipeborges@gnome.org>,
 *
 */

#ifndef UM_ARROW_FRAME_H
#define UM_ARROW_FRAME_H

#include <gtk/gtk.h>

#include "um-carousel.h"

G_BEGIN_DECLS

#define UM_TYPE_ARROW_FRAME (um_arrow_frame_get_type())

G_DECLARE_FINAL_TYPE (UmArrowFrame, um_arrow_frame, UM, ARROW_FRAME, GtkFrame)

GtkWidget*                      um_arrow_frame_new               (void);

void                            um_arrow_frame_set_item          (UmArrowFrame *frame,
                                                                  GtkWidget    *item);

G_END_DECLS

#endif /* UM_ARROW_FRAME_H */
