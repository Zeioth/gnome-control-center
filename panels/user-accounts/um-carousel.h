/* um-carousel.c
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
 * Writen by: Felipe Borges <felipeborges@gnome.org>
 *
 */

#ifndef UM_CAROUSEL_H
#define UM_CAROUSEL_H

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define UM_TYPE_CAROUSEL (um_carousel_get_type())

G_DECLARE_FINAL_TYPE (UmCarousel, um_carousel, UM, CAROUSEL, GtkGrid)

typedef GtkWidget * (*UmCarouselCreateWidgetFunc) (gpointer item,
						   gpointer user_data);

UmCarousel *um_carousel_new                (void);
void        um_carousel_bind_model         (UmCarousel    *self,
                                            GListModel    *model,
                                            UmCarouselCreateWidgetFunc create_widget_func,
                                            gpointer user_data,
                                            GDestroyNotify user_data_free_func);

void        um_carousel_select_item        (UmCarousel *self,
                                            gint        position);

G_END_DECLS

#endif /* UM_CAROUSEL_H */
