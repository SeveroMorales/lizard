/*
  GNOME stroke implementation
  Copyright (c) 2000, 2001 Dan Nicolaescu
  See the file COPYING for distribution information.
*/

#include "config.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include "gstroke.h"
#include "gstroke-internal.h"

static gboolean gstroke_draw_cb(GtkWidget *widget, cairo_t *cr,
                                gpointer user_data);
/*FIXME: Maybe these should be put in a structure, and not static...*/
static int mouse_button = 2;
static gboolean draw_strokes = FALSE;

#define GSTROKE_TIMEOUT_DURATION 10

#define GSTROKE_SIGNALS "gstroke_signals"

struct gstroke_func_and_data {
	void (*func)(GtkWidget *, void *);
	gpointer data;
};


/*FIXME: maybe it's better to just make 2 static variables, not a
  structure */
struct mouse_position {
	struct s_point last_point;
	gboolean invalid;
};


static struct mouse_position last_mouse_position;
static guint timer_id;

static void gstroke_execute (GtkWidget *widget, const gchar *name);

static void
record_stroke_segment(GtkWidget *widget)
{
	gint x, y;
	struct gstroke_metrics *metrics;
	GdkSeat *seat;
	GdkDevice *dev;

	g_return_if_fail(widget != NULL);

	seat = gdk_display_get_default_seat(gtk_widget_get_display(widget));
	dev = gdk_seat_get_pointer(seat);
	gdk_window_get_device_position(gtk_widget_get_window(widget),
		dev, &x, &y, NULL);

	last_mouse_position.invalid = FALSE;

	if (last_mouse_position.last_point.x != x ||
		last_mouse_position.last_point.y != y)
	{
		last_mouse_position.last_point.x = x;
		last_mouse_position.last_point.y = y;
		metrics = g_object_get_data(G_OBJECT(widget), GSTROKE_METRICS);
		_gstroke_record (x, y, metrics);
	}

	if (gstroke_draw_strokes()) {
		gtk_widget_queue_draw(widget);
	}
}

static gint
gstroke_timeout (gpointer data)
{
	GtkWidget *widget;

	g_return_val_if_fail(data != NULL, FALSE);

	widget = GTK_WIDGET (data);
	record_stroke_segment (widget);

	return TRUE;
}

static void
gstroke_cancel(GtkWidget *widget, GdkEvent *event)
{
	last_mouse_position.invalid = TRUE;

	if (timer_id > 0)
	    g_source_remove (timer_id);

	timer_id = 0;

	if (event != NULL) {
		gdk_seat_ungrab(gdk_event_get_seat(event));
	}

	if (gstroke_draw_strokes()) {
		gtk_widget_queue_draw(widget);
	}
}

static gint
process_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
  static GtkWidget *original_widget = NULL;
  static GdkCursor *cursor = NULL;

  switch (event->type) {
    case GDK_BUTTON_PRESS:
		if (event->button.button != gstroke_get_mouse_button()) {
			/* Similar to the bug below catch when any other button is
			 * clicked after the middle button is clicked (but possibly
			 * not released)
			 */
				gstroke_cancel(widget, event);
			original_widget = NULL;
			break;
		}

      original_widget = widget; /* remember the widget where
                                   the stroke started */

      record_stroke_segment (widget);

	  if (cursor == NULL) {
		  GdkDisplay *display = gtk_widget_get_display(widget);
		  cursor = gdk_cursor_new_for_display(display, GDK_PENCIL);
	  }

      gdk_seat_grab(gdk_event_get_seat(event), gtk_widget_get_window(widget),
                    GDK_SEAT_CAPABILITY_ALL_POINTING, FALSE, cursor, event,
                    NULL, NULL);
      timer_id = g_timeout_add (GSTROKE_TIMEOUT_DURATION,
				  gstroke_timeout, widget);
      return TRUE;

    case GDK_BUTTON_RELEASE:
      if ((event->button.button != gstroke_get_mouse_button())
	  || (original_widget == NULL)) {

		/* Nice bug when you hold down one button and press another. */
		/* We'll just cancel the gesture instead. */
				gstroke_cancel(widget, event);
		original_widget = NULL;
		break;
	  }

      last_mouse_position.invalid = TRUE;
      original_widget = NULL;
      g_source_remove (timer_id);
	  gdk_seat_ungrab(gdk_event_get_seat(event));
      timer_id = 0;

			{
				GtkWidget *history = data;
	char result[GSTROKE_MAX_SEQUENCE];
	struct gstroke_metrics *metrics;

	metrics = (struct gstroke_metrics *)g_object_get_data(G_OBJECT (widget),
														  GSTROKE_METRICS);
		if (gstroke_draw_strokes()) {
					gtk_widget_queue_draw(widget);
		}

				_gstroke_canonical(result, metrics);
				gstroke_execute(history, result);
			}
      return FALSE;

    default:
      break;
  }

  return FALSE;
}

void
gstroke_set_draw_strokes(gboolean draw)
{
	draw_strokes = draw;
}

gboolean
gstroke_draw_strokes(void)
{
	return draw_strokes;
}

void
gstroke_set_mouse_button(gint button)
{
	mouse_button = button;
}

guint
gstroke_get_mouse_button(void)
{
	return mouse_button;
}

void
gstroke_enable (GtkWidget *widget)
{
	GtkWidget *event = gtk_widget_get_parent(widget);
	struct gstroke_metrics *metrics = NULL;

	if (GTK_IS_EVENT_BOX(event)) {
		metrics = (struct gstroke_metrics *)g_object_get_data(G_OBJECT(event),
		                                                      GSTROKE_METRICS);
	}

	if (metrics == NULL) {
		GtkWidget *parent;

		metrics = g_new0(struct gstroke_metrics, 1);
		metrics->pointList = NULL;
		metrics->min_x = 10000;
		metrics->min_y = 10000;
		metrics->max_x = 0;
		metrics->max_y = 0;
		metrics->point_count = 0;

		event = gtk_event_box_new();
		gtk_event_box_set_above_child(GTK_EVENT_BOX(event), TRUE);
		gtk_widget_set_events(event, GDK_BUTTON_PRESS_MASK |
		                                     GDK_BUTTON_RELEASE_MASK |
		                                     GDK_BUTTON2_MOTION_MASK);
		gtk_widget_set_app_paintable(event, TRUE);
		gtk_widget_show(event);

		parent = gtk_widget_get_parent(widget);
		g_object_ref(widget);
		gtk_container_remove(GTK_CONTAINER(parent), widget);
		gtk_container_add(GTK_CONTAINER(event), widget);
		g_object_unref(widget);
		gtk_container_add(GTK_CONTAINER(parent), event);

		g_object_set_data(G_OBJECT(event), GSTROKE_METRICS, metrics);

		g_signal_connect(G_OBJECT(event), "event", G_CALLBACK(process_event),
		                 widget);
		g_signal_connect_after(G_OBJECT(event), "draw",
		                       G_CALLBACK(gstroke_draw_cb), NULL);
	} else {
		_gstroke_init(metrics);
	}

	last_mouse_position.invalid = TRUE;
}

void
gstroke_disable(GtkWidget *widget)
{
	GtkWidget *event = gtk_widget_get_parent(widget);

	g_return_if_fail(GTK_IS_EVENT_BOX(event));

	g_signal_handlers_disconnect_by_func(G_OBJECT(event),
	                                     G_CALLBACK(process_event), widget);
	g_signal_handlers_disconnect_by_func(G_OBJECT(event),
	                                     G_CALLBACK(gstroke_draw_cb), NULL);
}

void
gstroke_signal_connect(GtkWidget *widget, const gchar *name,
                       void (*func)(GtkWidget *widget, void *data),
                       gpointer data)
{
	struct gstroke_func_and_data *func_and_data;
	GHashTable *hash_table =
	        (GHashTable *)g_object_get_data(G_OBJECT(widget), GSTROKE_SIGNALS);

	if (!hash_table) {
		hash_table =
		        g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
		g_object_set_data(G_OBJECT(widget), GSTROKE_SIGNALS, hash_table);
	}

	func_and_data = g_new0(struct gstroke_func_and_data, 1);
	func_and_data->func = func;
	func_and_data->data = data;
	g_hash_table_insert(hash_table, g_strdup(name), func_and_data);
}

static void
gstroke_execute (GtkWidget *widget, const gchar *name)
{

  GHashTable *hash_table =
    (GHashTable*)g_object_get_data(G_OBJECT(widget), GSTROKE_SIGNALS);

#if 0
  purple_debug_misc("gestures", "gstroke %s", name);
#endif

  if (hash_table)
    {
      struct gstroke_func_and_data *fd =
	(struct gstroke_func_and_data*)g_hash_table_lookup (hash_table, name);
      if (fd)
	(*fd->func)(widget, fd->data);
    }
}

void
gstroke_cleanup (GtkWidget *widget)
{
	struct gstroke_metrics *metrics;
	GHashTable *hash_table = (GHashTable *)g_object_steal_data(G_OBJECT(widget),
	                                                           GSTROKE_SIGNALS);
	if (hash_table) {
		g_hash_table_destroy(hash_table);
	}

	metrics = (struct gstroke_metrics *)g_object_steal_data(G_OBJECT(widget),
	                                                        GSTROKE_METRICS);
	g_free(metrics);
}

static gboolean
gstroke_draw_cb(GtkWidget *widget, cairo_t *cr,
                G_GNUC_UNUSED gpointer user_data)
{
	struct gstroke_metrics *metrics =
	        (struct gstroke_metrics *)g_object_get_data(G_OBJECT(widget),
	                                                    GSTROKE_METRICS);
	GSList *iter = NULL;
	p_point point;

	if (last_mouse_position.invalid) {
		return FALSE;
	}

	if (!metrics) {
		return FALSE;
	}

	iter = metrics->pointList;
	if (!iter) {
		return FALSE;
	}

	cairo_save(cr);

	cairo_set_line_width(cr, 2.0);
	cairo_set_dash(cr, NULL, 0, 0.0);
	cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);
	cairo_set_line_join(cr, CAIRO_LINE_JOIN_MITER);

	point = (p_point)iter->data;
	iter = iter->next;
	cairo_move_to(cr, point->x, point->y);

	while (iter) {
		point = (p_point)iter->data;
		iter = iter->next;

		cairo_line_to(cr, point->x, point->y);
	}

	cairo_stroke(cr);

	cairo_restore(cr);

	return FALSE;
}
