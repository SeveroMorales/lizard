/*
 * pidgin
 *
 * Pidgin is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 *
 */

#include <glib/gi18n-lib.h>

#include <gtk/gtk.h>

#include <math.h>
#ifndef M_PI
#define M_PI (3.14159265358979323846264338327950288)
#endif

#include <purple.h>

#include "gtkwhiteboard.h"
#include "gtkutils.h"

#define UI_DATA "pidgin-ui-data"
#define PIDGIN_TYPE_WHITEBOARD (pidgin_whiteboard_get_type())
G_DECLARE_FINAL_TYPE(PidginWhiteboard, pidgin_whiteboard, PIDGIN, WHITEBOARD,
                     GtkWindow)

/**
 * PidginWhiteboard:
 * @cr:           Cairo context for drawing
 * @surface:      Cairo surface for drawing
 * @wb:           Backend data for this whiteboard
 * @drawing_area: Drawing area
 * @color_button: A color chooser widget
 * @width:        Canvas width
 * @height:       Canvas height
 * @brush_color:  Foreground color
 * @brush_size:   Brush size
 * @brush_state:  The @PidginWhiteboardBrushState state of the brush
 *
 * A PidginWhiteboard
 */
struct _PidginWhiteboard
{
	GtkWindow parent;

	cairo_t *cr;
	cairo_surface_t *surface;

	PurpleWhiteboard *wb;

	GtkWidget *drawing_area;
	GtkWidget *color_button;

	int width;
	int height;
	int brush_color;
	int brush_size;

	/* Tracks last position of the mouse when drawing */
	gdouble start_x;
	gdouble start_y;
	gdouble last_x;
	gdouble last_y;
};

G_DEFINE_TYPE(PidginWhiteboard, pidgin_whiteboard, GTK_TYPE_WINDOW)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
pidgin_whiteboard_rgb24_to_rgba(int color_rgb, GdkRGBA *color)
{
	color->red = ((color_rgb >> 16) & 0xFF) / 255.0f;
	color->green = ((color_rgb >> 8) & 0xFF) / 255.0f;
	color->blue = (color_rgb & 0xFF) / 255.0f;
	color->alpha = 1.0f;
}

static void
whiteboard_destroy_cb(GtkWidget *widget, G_GNUC_UNUSED gpointer data)
{
	PidginWhiteboard *gtkwb = PIDGIN_WHITEBOARD(widget);

	g_clear_object(&gtkwb->wb);
}

static void
pidgin_whiteboard_resize(G_GNUC_UNUSED GtkDrawingArea *self, gint width,
                         gint height, gpointer data)
{
	PidginWhiteboard *gtkwb = (PidginWhiteboard*)data;
	GdkRGBA white = {1.0f, 1.0f, 1.0f, 1.0f};
	cairo_t *cr;

	g_clear_pointer(&gtkwb->cr, cairo_destroy);
	g_clear_pointer(&gtkwb->surface, cairo_surface_destroy);

	gtkwb->surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, width,
	                                            height);
	gtkwb->cr = cr = cairo_create(gtkwb->surface);

	gdk_cairo_set_source_rgba(cr, &white);
	cairo_rectangle(cr, 0, 0, width, height);
	cairo_fill(cr);
}

static gboolean
pidgin_whiteboard_draw_event(G_GNUC_UNUSED GtkWidget *widget, cairo_t *cr,
                             gpointer _gtkwb)
{
	PidginWhiteboard *gtkwb = _gtkwb;

	cairo_set_source_surface(cr, gtkwb->surface, 0, 0);
	cairo_paint(cr);

	return FALSE;
}

static void
pidgin_whiteboard_draw_brush_point(PurpleWhiteboard *wb, int x, int y,
                                   int color, int size)
{
	PidginWhiteboard *gtkwb = g_object_get_data(G_OBJECT(wb), UI_DATA);
	GtkWidget *widget = gtkwb->drawing_area;
	cairo_t *gfx_con = gtkwb->cr;
	GdkRGBA rgba;

	/* Interpret and convert color */
	pidgin_whiteboard_rgb24_to_rgba(color, &rgba);
	gdk_cairo_set_source_rgba(gfx_con, &rgba);

	/* Draw a circle */
	cairo_arc(gfx_con, x, y, size / 2.0, 0.0, 2.0 * M_PI);
	cairo_fill(gfx_con);

	gtk_widget_queue_draw(widget);
}

/* Uses Bresenham's algorithm (as provided by Wikipedia) */
static void
pidgin_whiteboard_draw_brush_line(PurpleWhiteboard *wb, int x0, int y0, int x1,
                                  int y1, int color, int size)
{
	int temp;

	int xstep;
	int ystep;

	int dx;
	int dy;

	int error;
	int derror;

	int x;
	int y;

	gboolean steep = abs(y1 - y0) > abs(x1 - x0);

	if (steep) {
		temp = x0;
		x0 = y0;
		y0 = temp;
		temp = x1;
		x1 = y1;
		y1 = temp;
	}

	dx = abs(x1 - x0);
	dy = abs(y1 - y0);

	error = 0;
	derror = dy;

	x = x0;
	y = y0;

	if (x0 < x1) {
		xstep = 1;
	} else {
		xstep = -1;
	}

	if (y0 < y1) {
		ystep = 1;
	} else {
		ystep = -1;
	}

	if (steep) {
		pidgin_whiteboard_draw_brush_point(wb, y, x, color, size);
	} else {
		pidgin_whiteboard_draw_brush_point(wb, x, y, color, size);
	}

	while (x != x1) {
		x += xstep;
		error += derror;

		if ((error * 2) >= dx) {
			y += ystep;
			error -= dx;
		}

		if (steep) {
			pidgin_whiteboard_draw_brush_point(wb, y, x, color,
			                                   size);
		} else {
			pidgin_whiteboard_draw_brush_point(wb, x, y, color,
			                                   size);
		}
	}
}

static void
pidgin_whiteboard_brush_down(G_GNUC_UNUSED GtkGestureDrag* self, gdouble x,
                             gdouble y, gpointer data)
{
	PidginWhiteboard *gtkwb = (PidginWhiteboard*)data;
	PurpleWhiteboard *wb = gtkwb->wb;
	GList *draw_list = purple_whiteboard_get_draw_list(wb);

	if(gtkwb->cr != NULL) {
		/* Check if draw_list has contents; if so, clear it */
		if(draw_list) {
			purple_whiteboard_draw_list_destroy(draw_list);
			draw_list = NULL;
		}

		/* Set tracking variables */
		gtkwb->start_x = x;
		gtkwb->start_y = y;

		gtkwb->last_x = 0;
		gtkwb->last_y = 0;

		draw_list = g_list_append(draw_list, GINT_TO_POINTER(gtkwb->start_x));
		draw_list = g_list_append(draw_list, GINT_TO_POINTER(gtkwb->start_y));

		pidgin_whiteboard_draw_brush_point(gtkwb->wb, gtkwb->start_x,
		                                   gtkwb->start_y, gtkwb->brush_color,
		                                   gtkwb->brush_size);
	}

	purple_whiteboard_set_draw_list(wb, draw_list);
}

static void
pidgin_whiteboard_brush_motion(G_GNUC_UNUSED GtkGestureDrag* self, gdouble x,
                               gdouble y, gpointer data)
{
	PidginWhiteboard *gtkwb = (PidginWhiteboard*)data;
	PurpleWhiteboard *wb = gtkwb->wb;
	GList *draw_list = purple_whiteboard_get_draw_list(wb);

	if (gtkwb->cr != NULL) {
		gdouble dx, dy;

		/* x and y are relative to the starting post, but we need to know where
		 * there are according to the last point, so we have to do the algebra.
		 */
		dx = (x + gtkwb->start_x - gtkwb->last_x);
		dy = (y + gtkwb->start_y - gtkwb->last_y);

		draw_list = g_list_append(draw_list, GINT_TO_POINTER(dx));
		draw_list = g_list_append(draw_list, GINT_TO_POINTER(dy));

		pidgin_whiteboard_draw_brush_line(gtkwb->wb,
		                                  gtkwb->start_x + gtkwb->last_x,
		                                  gtkwb->start_y + gtkwb->last_y,
		                                  gtkwb->start_x + x,
		                                  gtkwb->start_y + y,
		                                  gtkwb->brush_color,
		                                  gtkwb->brush_size);

		gtkwb->last_x = x;
		gtkwb->last_y = y;
	}

	purple_whiteboard_set_draw_list(wb, draw_list);
}

static void
pidgin_whiteboard_brush_up(G_GNUC_UNUSED GtkGestureDrag *self,
                           G_GNUC_UNUSED gdouble x, G_GNUC_UNUSED gdouble y,
                           gpointer data)
{
	PidginWhiteboard *gtkwb = (PidginWhiteboard*)data;
	PurpleWhiteboard *wb = gtkwb->wb;
	GList *draw_list = purple_whiteboard_get_draw_list(wb);

	if(gtkwb->cr != NULL) {
		/* Send draw list to protocol draw_list handler */
		purple_whiteboard_send_draw_list(gtkwb->wb, draw_list);

		/* The brush stroke is finished, clear the list for another one
		 */
		if (draw_list) {
			purple_whiteboard_draw_list_destroy(draw_list);
		}

		purple_whiteboard_set_draw_list(wb, NULL);
	}
}

static void pidgin_whiteboard_set_dimensions(PurpleWhiteboard *wb, int width, int height)
{
	PidginWhiteboard *gtkwb = g_object_get_data(G_OBJECT(wb), UI_DATA);

	gtkwb->width = width;
	gtkwb->height = height;
}

static void pidgin_whiteboard_set_brush(PurpleWhiteboard *wb, int size, int color)
{
	PidginWhiteboard *gtkwb = g_object_get_data(G_OBJECT(wb), UI_DATA);

	gtkwb->brush_size = size;
	gtkwb->brush_color = color;
}

static void pidgin_whiteboard_clear(PurpleWhiteboard *wb)
{
	PidginWhiteboard *gtkwb = g_object_get_data(G_OBJECT(wb), UI_DATA);
	GtkWidget *drawing_area = gtkwb->drawing_area;
	cairo_t *cr = gtkwb->cr;
	GtkAllocation allocation;
	GdkRGBA white = {1.0f, 1.0f, 1.0f, 1.0f};

	gtk_widget_get_allocation(drawing_area, &allocation);

	gdk_cairo_set_source_rgba(cr, &white);
	cairo_rectangle(cr, 0, 0, allocation.width, allocation.height);
	cairo_fill(cr);

	gtk_widget_queue_draw(drawing_area);
}

static void
pidgin_whiteboard_clear_response(GtkDialog *self, gint response, gpointer data)
{
	PidginWhiteboard *gtkwb = (PidginWhiteboard *)data;

	if(response == GTK_RESPONSE_YES) {
		pidgin_whiteboard_clear(gtkwb->wb);

		/* Do protocol specific clearing procedures */
		purple_whiteboard_send_clear(gtkwb->wb);
	}

	gtk_window_destroy(GTK_WINDOW(self));
}

static void
pidgin_whiteboard_button_clear_press(G_GNUC_UNUSED GtkWidget *widget,
                                     gpointer data)
{
	PidginWhiteboard *gtkwb = (PidginWhiteboard*)(data);

	/* Confirm whether the user really wants to clear */
	GtkWidget *dialog = gtk_message_dialog_new(
	        GTK_WINDOW(gtkwb), GTK_DIALOG_DESTROY_WITH_PARENT,
	        GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, "%s",
	        _("Do you really want to clear?"));

	g_signal_connect(dialog, "response",
	                 G_CALLBACK(pidgin_whiteboard_clear_response), gtkwb);

	gtk_widget_show(dialog);
}

static void
pidgin_whiteboard_save_response(GtkNativeDialog *self, gint response_id,
                                gpointer data)
{
	PidginWhiteboard *gtkwb = (PidginWhiteboard *)data;
	GdkPixbuf *pixbuf;

	if(response_id == GTK_RESPONSE_ACCEPT) {
		gboolean success;
		GFile *file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(self));
		gchar *filename = g_file_get_path(file);

		pixbuf = gdk_pixbuf_get_from_surface(gtkwb->surface, 0, 0,
		                                     gtkwb->width, gtkwb->height);

		success = gdk_pixbuf_save(pixbuf, filename, "png", NULL,
		                          "compression", "9", NULL);
		g_object_unref(pixbuf);

		if (success) {
			purple_debug_info("gtkwhiteboard", "whiteboard saved to \"%s\"",
			                  filename);
		} else {
			purple_notify_error(NULL, _("Whiteboard"),
			                    _("Unable to save the file"), NULL, NULL);
			purple_debug_error("gtkwhiteboard", "whiteboard "
			                   "couldn't be saved to \"%s\"", filename);
		}

		g_free(filename);
		g_object_unref(file);
	}

	g_object_unref(self);
}


static void
pidgin_whiteboard_button_save_press(G_GNUC_UNUSED GtkWidget *widget,
                                    gpointer _gtkwb)
{
	PidginWhiteboard *gtkwb = _gtkwb;
	GtkFileChooserNative *chooser;

	chooser = gtk_file_chooser_native_new(_("Save File"), GTK_WINDOW(gtkwb),
	                                      GTK_FILE_CHOOSER_ACTION_SAVE,
	                                      _("_Save"), _("_Cancel"));

	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(chooser),
	                                  "whiteboard.png");

	g_signal_connect(chooser, "response",
	                 G_CALLBACK(pidgin_whiteboard_save_response), gtkwb);

	gtk_native_dialog_show(GTK_NATIVE_DIALOG(chooser));
}

static void
color_selected(GtkColorButton *button, PidginWhiteboard *gtkwb)
{
	GdkRGBA color;
	PurpleWhiteboard *wb = gtkwb->wb;
	int old_size, old_color;
	int new_color;

	gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(button), &color);

	new_color = (unsigned int)(color.red * 255) << 16;
	new_color |= (unsigned int)(color.green * 255) << 8;
	new_color |= (unsigned int)(color.blue * 255);

	purple_whiteboard_get_brush(wb, &old_size, &old_color);
	purple_whiteboard_send_brush(wb, old_size, new_color);
}

static void
pidgin_whiteboard_create(PurpleWhiteboard *wb)
{
	PidginWhiteboard *gtkwb;
	PurpleBuddy *buddy;
	GdkRGBA color;

	gtkwb = PIDGIN_WHITEBOARD(g_object_new(PIDGIN_TYPE_WHITEBOARD, NULL));
	gtkwb->wb = wb;
	g_object_set_data_full(G_OBJECT(wb), UI_DATA, gtkwb, g_object_unref);

	/* Get dimensions (default?) for the whiteboard canvas */
	if (!purple_whiteboard_get_dimensions(wb, &gtkwb->width,
	                                      &gtkwb->height)) {
		/* Give some initial board-size */
		gtkwb->width = 300;
		gtkwb->height = 250;
	}

	if (!purple_whiteboard_get_brush(wb, &gtkwb->brush_size,
	                                 &gtkwb->brush_color)) {
		/* Give some initial brush-info */
		gtkwb->brush_size = 2;
		gtkwb->brush_color = 0xff0000;
	}

	/* Try and set window title as the name of the buddy, else just use
	 * their username
	 */
	buddy = purple_blist_find_buddy(purple_whiteboard_get_account(wb),
	                                purple_whiteboard_get_id(wb));

	gtk_window_set_title(GTK_WINDOW(gtkwb),
	                     buddy != NULL
	                             ? purple_buddy_get_contact_alias(buddy)
	                             : purple_whiteboard_get_id(wb));
	gtk_widget_set_name(GTK_WIDGET(gtkwb), purple_whiteboard_get_id(wb));

	gtk_widget_set_size_request(GTK_WIDGET(gtkwb->drawing_area),
	                            gtkwb->width, gtkwb->height);

	pidgin_whiteboard_rgb24_to_rgba(gtkwb->brush_color, &color);
	gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(gtkwb->color_button),
	                           &color);

	/* Make all this (window) visible */
	gtk_widget_show(GTK_WIDGET(gtkwb));

	/* TODO Specific protocol/whiteboard assignment here? Needs a UI Op? */
	/* Set default brush size and color */
	/*
	ds->brush_size = DOODLE_BRUSH_MEDIUM;
	ds->brush_color = 0;
	*/
}

static void
pidgin_whiteboard_destroy(PurpleWhiteboard *wb)
{
	PidginWhiteboard *gtkwb;

	g_return_if_fail(wb != NULL);
	gtkwb = g_object_get_data(G_OBJECT(wb), UI_DATA);
	g_return_if_fail(gtkwb != NULL);

	/* TODO Ask if user wants to save picture before the session is closed
	 */

	gtkwb->wb = NULL;
}

/******************************************************************************
 * GObject implementation
 *****************************************************************************/
static void
pidgin_whiteboard_init(PidginWhiteboard *self) {
	gtk_widget_init_template(GTK_WIDGET(self));
}

static void
pidgin_whiteboard_finalize(GObject *obj) {
	PidginWhiteboard *gtkwb = PIDGIN_WHITEBOARD(obj);

	/* Clear graphical memory */
	g_clear_pointer(&gtkwb->cr, cairo_destroy);
	g_clear_pointer(&gtkwb->surface, cairo_surface_destroy);

	G_OBJECT_CLASS(pidgin_whiteboard_parent_class)->finalize(obj);
}

static void
pidgin_whiteboard_class_init(PidginWhiteboardClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	obj_class->finalize = pidgin_whiteboard_finalize;

	gtk_widget_class_set_template_from_resource(
	        widget_class, "/im/pidgin/Pidgin3/Whiteboard/whiteboard.ui");

	gtk_widget_class_bind_template_child(widget_class, PidginWhiteboard,
	                                     drawing_area);
	gtk_widget_class_bind_template_child(widget_class, PidginWhiteboard,
	                                     color_button);

	gtk_widget_class_bind_template_callback(
	        widget_class, whiteboard_destroy_cb);
	gtk_widget_class_bind_template_callback(
	        widget_class, pidgin_whiteboard_draw_event);
	gtk_widget_class_bind_template_callback(
	        widget_class, pidgin_whiteboard_resize);
	gtk_widget_class_bind_template_callback(
	        widget_class, pidgin_whiteboard_brush_down);
	gtk_widget_class_bind_template_callback(
	        widget_class, pidgin_whiteboard_brush_motion);
	gtk_widget_class_bind_template_callback(
	        widget_class, pidgin_whiteboard_brush_up);
	gtk_widget_class_bind_template_callback(
	        widget_class, pidgin_whiteboard_button_clear_press);
	gtk_widget_class_bind_template_callback(
	        widget_class, pidgin_whiteboard_button_save_press);
	gtk_widget_class_bind_template_callback(
	        widget_class, color_selected);
}

/******************************************************************************
 * API
 *****************************************************************************/
static PurpleWhiteboardUiOps ui_ops = {
	.create = pidgin_whiteboard_create,
	.destroy = pidgin_whiteboard_destroy,
	.set_dimensions = pidgin_whiteboard_set_dimensions,
	.set_brush = pidgin_whiteboard_set_brush,
	.draw_point = pidgin_whiteboard_draw_brush_point,
	.draw_line = pidgin_whiteboard_draw_brush_line,
	.clear = pidgin_whiteboard_clear,
};

PurpleWhiteboardUiOps *
pidgin_whiteboard_get_ui_ops(void)
{
	return &ui_ops;
}
