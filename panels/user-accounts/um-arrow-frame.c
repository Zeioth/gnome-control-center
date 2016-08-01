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
 *            Georges Basile Stavracas Neto <georges.stavracas@gmail.com>
 *
 */

#include "um-arrow-frame.h"

#define ARROW_HEIGHT 16
#define ARROW_WIDTH 36
#define HANDLE_GAP (ARROW_HEIGHT + 5)

struct _UmArrowFrame {
  GtkFrame parent;

  UmCarousel *carousel;
  GtkWidget *item;

  GdkWindow *handle_window;
  gint margin_top;
};

G_DEFINE_TYPE (UmArrowFrame, um_arrow_frame, GTK_TYPE_FRAME)

static gint
um_arrow_frame_get_row_x (UmArrowFrame *frame)
{
  GtkWidget *widget;
  GtkWidget *row;
  gint row_width;
  gint dest_x;

  widget = GTK_WIDGET (frame);

  if (!frame->item)
    return gtk_widget_get_allocated_width (widget) / 2;

  row = GTK_WIDGET (frame->item);
  row_width = gtk_widget_get_allocated_width (row);

  gtk_widget_translate_coordinates (row,
                                    widget,
                                    row_width / 2,
                                    0,
                                    &dest_x,
                                    NULL);

  return CLAMP (dest_x,
                0,
                gtk_widget_get_allocated_width (widget));
}

static void
um_arrow_frame_draw_arrow (UmArrowFrame *frame,
                           cairo_t      *cr)
{
  GtkWidget *widget = GTK_WIDGET (frame);
  GtkStyleContext *context;
  GtkAllocation alloc;
  GtkStateFlags state;
  GtkBorder border;
  GdkRGBA border_color;
  gint border_width;
  gint start_x;
  gint start_y;
  gint tip_x;
  gint tip_y;
  gint end_x;
  gint end_y;

  context = gtk_widget_get_style_context (widget);
  state = gtk_style_context_get_state (context);

  gtk_style_context_get_border (context,
                                state,
                                &border);

  gtk_style_context_get (context,
                         state,
                         GTK_STYLE_PROPERTY_BORDER_COLOR, &border_color,
                         NULL);

  /* widget size */
  gtk_widget_get_allocation (widget, &alloc);

  tip_x = um_arrow_frame_get_row_x (frame);
  start_x = tip_x - (ARROW_WIDTH / 2);
  end_x = tip_x + (ARROW_WIDTH / 2);

  start_y = end_y = border.top + ARROW_HEIGHT;
  tip_y = 0;
  border_width = border.top;

  /* draw arrow */
  cairo_save (cr);

  cairo_set_line_width (cr, 1.0);
  cairo_move_to (cr, start_x, start_y);
  cairo_line_to (cr, tip_x,   tip_y);
  cairo_line_to (cr, end_x,   end_y);

  /*
   * Don't allow that gtk_render_background renders
   * anything out of (tip_x, start_y) (end_x, end_y).
   */
  cairo_clip (cr);

  /* render the arrow background */
  gtk_render_background (context,
                         cr,
                         0,
                         0,
                         alloc.width,
                         alloc.height);

  /* draw the border */
  if (border_width > 0)
    {
G_GNUC_BEGIN_IGNORE_DEPRECATIONS

      gtk_style_context_get_border_color (context,
                                          state,
                                          &border_color);

G_GNUC_END_IGNORE_DEPRECATIONS

      gdk_cairo_set_source_rgba (cr, &border_color);

      cairo_set_line_width (cr, 1);
      cairo_move_to (cr, start_x, start_y);
      cairo_line_to (cr, tip_x,   tip_y);
      cairo_line_to (cr, end_x,   end_y);

      cairo_set_line_width (cr, border_width + 1);
      cairo_stroke (cr);
    }

  cairo_restore (cr);
}

static void
um_arrow_frame_draw_background (UmArrowFrame *frame,
                                cairo_t      *cr)
{
  GtkWidget *widget = GTK_WIDGET (frame);
  GtkStyleContext *context;
  GtkAllocation alloc;
  GtkStateFlags state;
  GtkBorder margin;
  gint start_x;
  gint start_y;
  gint start_gap;
  gint end_x;
  gint end_y;
  gint end_gap;

  context = gtk_widget_get_style_context (widget);
  state = gtk_style_context_get_state (context);

  /* widget size */
  gtk_widget_get_allocation (widget, &alloc);

  /* margin */
  gtk_style_context_get_margin (context,
                                state,
                                &margin);

  start_x = margin.left;
  end_x = alloc.width + margin.right;

  start_y = margin.top + ARROW_HEIGHT;
  end_y = alloc.height + margin.bottom;

  start_gap = ((end_y - start_y + ARROW_WIDTH) / 2);
  end_gap = ((end_y - start_y + ARROW_WIDTH) / 2);

  gtk_render_background (context,
                         cr,
                         start_x,
                         start_y,
                         end_x,
                         end_y);
  gtk_render_frame_gap (context,
                        cr,
                        start_x,
                        start_y,
                        end_x,
                        end_y,
                        GTK_POS_TOP,
                        start_gap,
                        end_gap);
}

static gboolean
um_arrow_frame_draw (GtkWidget *widget,
                     cairo_t   *cr)
{
  UmArrowFrame *frame = UM_ARROW_FRAME (widget);
  GtkWidget *child;

  um_arrow_frame_draw_background (frame, cr);
  if (frame->item)
    um_arrow_frame_draw_arrow (frame, cr);

  child = gtk_bin_get_child (GTK_BIN (widget));

  if (child)
    {
      gtk_container_propagate_draw (GTK_CONTAINER (widget), child, cr);
    }

  return TRUE;
}

static void
um_arrow_frame_compute_child_allocation (GtkFrame      *frame,
                                         GtkAllocation *allocation)
{
  allocation->height += HANDLE_GAP;
  GTK_FRAME_CLASS (um_arrow_frame_parent_class)->compute_child_allocation (frame, allocation);
}

static void
um_arrow_frame_get_preferred_height (GtkWidget *widget,
                                     gint      *minimum_height,
                                     gint      *natural_height)
{
  GTK_WIDGET_CLASS (um_arrow_frame_parent_class)->get_preferred_height (widget,
                                                                        minimum_height,
                                                                        natural_height);

  *minimum_height += ARROW_HEIGHT;
  *natural_height += ARROW_HEIGHT;
  *natural_height = MAX (*minimum_height, *natural_height + HANDLE_GAP);
}

static void
um_arrow_frame_realize (GtkWidget *widget)
{
  UmArrowFrame *self = UM_ARROW_FRAME (widget);
  GtkAllocation allocation;
  GdkWindowAttr attributes = { 0 };
  GdkDisplay *display;
  GdkWindow *parent_window;
  gint attributes_mask;

  display = gtk_widget_get_display (widget);
  parent_window = gtk_widget_get_parent_window (widget);

  gtk_widget_set_realized (widget, TRUE);

  gtk_widget_set_window (widget, parent_window);
  g_object_ref (parent_window);

  gtk_widget_get_allocation (widget, &allocation);

  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.wclass = GDK_INPUT_ONLY;
  attributes.x = allocation.x;
  attributes.y = allocation.y;
  attributes.width = allocation.width;
  attributes.height = allocation.height;
  attributes.visual = gtk_widget_get_visual (widget);
  attributes.cursor = gdk_cursor_new_for_display (display, GDK_SB_H_DOUBLE_ARROW);
  attributes.event_mask = gtk_widget_get_events (widget);
  attributes.event_mask |= (GDK_BUTTON_PRESS_MASK |
                            GDK_BUTTON_RELEASE_MASK |
                            GDK_ENTER_NOTIFY_MASK |
                            GDK_LEAVE_NOTIFY_MASK |
                            GDK_POINTER_MOTION_MASK);

  attributes_mask = GDK_WA_CURSOR | GDK_WA_X | GDK_WA_Y;

  self->handle_window = gdk_window_new (parent_window,
                                        &attributes,
                                        attributes_mask);

  gtk_widget_register_window (widget, self->handle_window);

  g_clear_object (&attributes.cursor);
}

static void
um_arrow_frame_unrealize (GtkWidget *widget)
{
  UmArrowFrame *self = UM_ARROW_FRAME (widget);

  if (self->handle_window)
    {
      gdk_window_hide (self->handle_window);
      gtk_widget_unregister_window (widget, self->handle_window);
      g_clear_pointer (&self->handle_window, gdk_window_destroy);
    }

  GTK_WIDGET_CLASS (um_arrow_frame_parent_class)->unrealize (widget);
}

static void
um_arrow_frame_size_allocate (GtkWidget     *widget,
                              GtkAllocation *allocation)
{
  UmArrowFrame *self = UM_ARROW_FRAME (widget);

  GTK_WIDGET_CLASS (um_arrow_frame_parent_class)->size_allocate (widget, allocation);

  allocation->y = self->margin_top - ARROW_HEIGHT;
  gtk_widget_set_allocation (widget, allocation);

  if (gtk_widget_get_realized (widget))
    {
      gdk_window_move_resize (self->handle_window,
                              allocation->x,
                              allocation->y,
                              allocation->width,
                              allocation->height);

      gdk_window_raise (self->handle_window);
    }
}

GtkWidget *
um_arrow_frame_new (void)
{
  return g_object_new (UM_TYPE_ARROW_FRAME, NULL);
}

static void
um_arrow_frame_finalize (GObject *object)
{
  G_OBJECT_CLASS (um_arrow_frame_parent_class)->finalize (object);
}

static void
um_arrow_frame_class_init (UmArrowFrameClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GtkFrameClass *frame_class = GTK_FRAME_CLASS (klass);

  object_class->finalize = um_arrow_frame_finalize;

  widget_class->draw = um_arrow_frame_draw;
  widget_class->get_preferred_height = um_arrow_frame_get_preferred_height;
  widget_class->realize = um_arrow_frame_realize;
  widget_class->unrealize = um_arrow_frame_unrealize;
  widget_class->size_allocate = um_arrow_frame_size_allocate;

  frame_class->compute_child_allocation = um_arrow_frame_compute_child_allocation;
}

static void
um_arrow_frame_init (UmArrowFrame *self)
{
  GtkStyleProvider *provider;

  provider = GTK_STYLE_PROVIDER (gtk_css_provider_new ());
  gtk_css_provider_load_from_data (GTK_CSS_PROVIDER (provider), "* {border-top: solid 1px; border-color: @borders }", -1, NULL);
  gtk_style_context_add_provider (gtk_widget_get_style_context (GTK_WIDGET (self)),
                                  provider,
				  GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  g_object_unref (provider);

}

static void
um_arrow_frame_set_margin_top (GtkWidget     *widget,
                               GtkAllocation *allocation,
                               UmArrowFrame  *frame)
{
  frame->margin_top = allocation->y*2 + allocation->height;
}

void
um_arrow_frame_set_item (UmArrowFrame *frame,
                         GtkWidget    *item)
{
  g_return_if_fail (UM_IS_ARROW_FRAME (frame));

  if (item == NULL) {
    frame->margin_top = 0;
    frame->item = NULL;
  } else {
    frame->item = GTK_WIDGET (item);
    g_signal_connect (item, "size-allocate", G_CALLBACK (um_arrow_frame_set_margin_top), frame);
  }

  gtk_widget_queue_draw (GTK_WIDGET (frame));
}
