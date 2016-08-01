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

#include "um-carousel.h"

#include <act/act.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#define ITEMS_PER_PAGE 3

struct _UmCarousel {
    GtkGrid parent;

    GSequence *pages;
    GSequence *children;
    gint current_page;

    GList *model;
    UmCarouselCreateWidgetFunc create_widget_func;
    gpointer create_widget_func_data;
    GDestroyNotify create_widget_func_data_destroy;

    /* Widgets */
    GtkStack *stack;
    GtkRadioButton *current_button;
    GtkWidget *go_back_button;
    GtkWidget *go_next_button;
};

G_DEFINE_TYPE (UmCarousel, um_carousel, GTK_TYPE_GRID)

enum {
    ITEM_ACTIVATED,
    NUM_SIGNALS
};

static guint signals[NUM_SIGNALS] = { 0, };

static GtkWidget *
get_item_at_index (UmCarousel *self,
                   gint        index)
{
    GSequenceIter *iter;

    iter = g_sequence_get_iter_at_pos (self->children, index);
    if (!g_sequence_iter_is_end (iter))
        return g_sequence_get (iter);

    return NULL;
}

static GtkWidget *
get_page (UmCarousel *self)
{
    GSequenceIter *iter = NULL;
    GtkWidget *box;
    gint num_of_children;

    num_of_children = g_sequence_get_length (self->children);
    if (num_of_children % ITEMS_PER_PAGE == 0) {
        gchar *page_name;

        box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        page_name = g_strdup_printf ("%d", g_sequence_get_length (self->pages));
        gtk_stack_add_named (self->stack, box, page_name);

        g_sequence_append (self->pages, box);
    } else {
        iter = g_sequence_get_end_iter (self->pages);
        iter = g_sequence_iter_prev (iter);
        box = g_sequence_get (iter);
    }

    return box;
}

static void
set_visible_page (UmCarousel *self,
                  gint        page)
{
    gchar *page_name;

    self->current_page = page;
    page_name = g_strdup_printf ("%d", self->current_page);
    gtk_stack_set_visible_child_name (self->stack, page_name);

    g_free (page_name);
}

static void
on_item_toggled (GtkToggleButton *item,
                 gpointer         user_data)
{
    g_signal_emit (user_data, signals[ITEM_ACTIVATED], 0, item);
}

static GtkWidget *
create_item (UmCarousel *self,
             GtkWidget  *child,
             gint        position,
             gint        item_id)
{
    GtkStyleProvider *provider;
    GtkWidget *item;

    item = gtk_radio_button_new (NULL);

    provider = GTK_STYLE_PROVIDER (gtk_css_provider_new ());
    gtk_css_provider_load_from_data (GTK_CSS_PROVIDER (provider), "* {border: 0; background: none }", -1, NULL);
    gtk_style_context_add_provider (gtk_widget_get_style_context (item), provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    g_object_unref (provider);

    g_object_set_data (G_OBJECT (item), "item-ref", GINT_TO_POINTER (item_id));
    g_signal_connect (item, "toggled", G_CALLBACK (on_item_toggled), self);

    if (self->current_button == NULL) {
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (item), TRUE);
    } else {
        gtk_radio_button_join_group (GTK_RADIO_BUTTON (item), self->current_button);
    }
    self->current_button = GTK_RADIO_BUTTON (item);

    gtk_container_add (GTK_CONTAINER (item), child);
    gtk_widget_show (GTK_WIDGET (child));

    gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (item), FALSE);
    gtk_style_context_add_class (gtk_widget_get_style_context (item), "flat");
    gtk_widget_set_valign (item, GTK_ALIGN_CENTER);

    return item;
}

static void
forall_items (GtkContainer *container,
              GtkCallback   callback,
              gpointer      callback_target)
{
    UmCarousel *self = UM_CAROUSEL (container);
    GSequenceIter *iter;
    GtkWidget *item;

    iter = g_sequence_get_begin_iter (self->children);
    while (!g_sequence_iter_is_end (iter)) {
        item = g_sequence_get (iter);
        iter = g_sequence_iter_next (iter);
        callback (GTK_WIDGET (item), callback_target);
    }
}

static void
insert_item (UmCarousel *self,
             GtkWidget  *child,
             gint        position,
             gint        item_id)
{
    GtkWidget *item, *box;

    box = get_page (self);
    item = create_item (self, child, position, item_id);

    if (position == 0) {
        g_sequence_prepend (self->children, item);
        gtk_box_pack_start (GTK_BOX (box), item, TRUE, FALSE, 10);
    } else if (position == -1) {
        g_sequence_append (self->children, item);
        gtk_box_pack_end (GTK_BOX (box), item, TRUE, FALSE, 10);
    } else {
         GSequenceIter *iter;

         iter = g_sequence_get_iter_at_pos (self->children, position);
         iter = g_sequence_insert_before (iter, item);

         gtk_box_pack_start (GTK_BOX (box), item, TRUE, FALSE, 10);
         gtk_box_reorder_child (GTK_BOX (box), item, position);
    }

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (item), TRUE);
    gtk_widget_show_all (box);
}

void
um_carousel_select_item (UmCarousel *self,
                         gint        position)
{
    self->current_button = GTK_RADIO_BUTTON (get_item_at_index (self, position));
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (self->current_button), TRUE);

    self->current_page = floor (position / ITEMS_PER_PAGE);
    set_visible_page (self, self->current_page);
}

static void
model_changed (GListModel *list,
               guint       position,
               guint       removed,
               guint       added,
               gpointer    user_data)
{
    UmCarousel *self = user_data;
    gboolean have_more_pages;
    gint i;

    while (removed--) {
        GtkWidget *item;

        um_carousel_select_item (self, 0);

        item = get_item_at_index (self, position);
        gtk_widget_destroy (GTK_WIDGET (item));
    }

    for (i = 0; i < added; i++) {
        GObject *item;
        GtkWidget *widget;
        uid_t user_uid;

        item = g_list_model_get_object (list, position + i);
        user_uid = act_user_get_uid (ACT_USER (item));
        widget = self->create_widget_func (item, self->create_widget_func_data);

        if (g_object_is_floating (widget)) {
            g_object_ref_sink (widget);

            gtk_widget_show (widget);
            insert_item (self, widget, position + i, user_uid);

            g_object_unref (widget);
            g_object_unref (item);

            /* Jump to last page when a new item gets appended. */
            set_visible_page (self, g_sequence_get_length (self->pages) -1);
	}
    }

    have_more_pages = (g_sequence_get_length (self->pages) > 1);
    gtk_widget_set_sensitive (self->go_back_button, have_more_pages);
    gtk_widget_set_sensitive (self->go_next_button, have_more_pages);
}

void
um_carousel_bind_model (UmCarousel                 *self,
                        GListModel                 *model,
                        UmCarouselCreateWidgetFunc  create_widget_func,
                        gpointer                    user_data,
                        GDestroyNotify              user_data_free_func)
{
    if (self->model) {
        if (self->create_widget_func_data_destroy) {
            self->create_widget_func_data_destroy (self->create_widget_func_data);
        }

        g_signal_handlers_disconnect_by_func (self->model, model_changed, self);
        g_clear_object (&self->model);
    }

    forall_items (GTK_CONTAINER (self), (GtkCallback) gtk_widget_destroy, NULL);

    if (model == NULL)
        return;

    self->model = g_object_ref (model);
    self->create_widget_func = create_widget_func;
    self->create_widget_func_data = user_data;
    self->create_widget_func_data_destroy = user_data_free_func;

    g_signal_connect (self->model, "items-changed", G_CALLBACK (model_changed), self);
    model_changed (model, 0, 0, g_list_model_get_n_items (model), self);
}

static void
um_carousel_page_changed (GtkButton *button,
                          gpointer   user_data)
{
    UmCarousel *self = UM_CAROUSEL (user_data);

    set_visible_page (self, self->current_page);
    um_carousel_select_item (self, self->current_page * ITEMS_PER_PAGE);

}

static void
um_carousel_go_back_button_clicked (GtkButton *button,
                                    gpointer   user_data)
{
    UmCarousel *self = UM_CAROUSEL (user_data);

    self->current_page--;
    if (self->current_page < 0) {
        self->current_page = g_sequence_get_length (self->pages) - 1;
    }
}

static void
um_carousel_go_next_button_clicked (GtkButton *button,
                                    gpointer   user_data)
{
    UmCarousel *self = UM_CAROUSEL (user_data);

    self->current_page++;
    if (self->current_page >= g_sequence_get_length (self->pages)) {
        self->current_page = 0;
    }
}

UmCarousel *
um_carousel_new (void)
{
    return g_object_new (UM_TYPE_CAROUSEL, NULL);
}

static void
um_carousel_finalize (GObject *object)
{
    G_OBJECT_CLASS (um_carousel_parent_class)->finalize (object);
}

static void
um_carousel_class_init (UmCarouselClass *klass)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = um_carousel_finalize;

    gtk_widget_class_set_template_from_resource (widget_class,
                                                 "/org/gnome/control-center/user-accounts/carousel.ui");

    gtk_widget_class_bind_template_child (widget_class, UmCarousel, stack);
    gtk_widget_class_bind_template_child (widget_class, UmCarousel, go_back_button);
    gtk_widget_class_bind_template_child (widget_class, UmCarousel, go_next_button);

    gtk_widget_class_bind_template_callback (widget_class, um_carousel_go_back_button_clicked);
    gtk_widget_class_bind_template_callback (widget_class, um_carousel_go_next_button_clicked);
    gtk_widget_class_bind_template_callback (widget_class, um_carousel_page_changed);

    signals[ITEM_ACTIVATED] = g_signal_new ("item-activated",
                                            UM_TYPE_CAROUSEL,
                                            G_SIGNAL_RUN_LAST,
                                            0,
                                            NULL, NULL,
                                            g_cclosure_marshal_VOID__OBJECT,
                                            G_TYPE_NONE, 1,
                                            GTK_TYPE_TOGGLE_BUTTON);
}

static void
um_carousel_init (UmCarousel *self)
{
    self->pages = g_sequence_new (NULL);
    self->current_page = 0;
    self->children = g_sequence_new (NULL);

    gtk_widget_init_template (GTK_WIDGET (self));

    self->current_button = NULL;
}
