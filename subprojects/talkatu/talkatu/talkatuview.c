/*
 * Talkatu - GTK widgets for chat applications
 * Copyright (C) 2017-2020 Gary Kramlich <grim@reaperworld.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, see <https://www.gnu.org/licenses/>.
 */

#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <gumbo.h>
#include <stdio.h>

#include "talkatu/talkatuactiongroup.h"
#include "talkatu/talkatuattachment.h"
#include "talkatu/talkatuattachmentdialog.h"
#include "talkatu/talkatubuffer.h"
#include "talkatu/talkatumarkup.h"
#include "talkatu/talkatutag.h"
#include "talkatu/talkatuview.h"

/**
 * TalkatuViewClass:
 * @open_url: The class handler for the #TalkatuView::open_url signal.
 *
 * The backing class to #TalkatuView instances.
 */

/**
 * TalkatuView:
 *
 * A #GtkTextView subclass that's preconfigured with a #TalkatuBuffer.
 */

typedef struct {
	GSimpleActionGroup *action_group;

	GtkWidget *menu;
	gchar *url;
} TalkatuViewPrivate;

enum {
	SIG_OPEN_URL,
	LAST_SIGNAL,
};
static guint signals[LAST_SIGNAL] = {0, };

G_DEFINE_TYPE_WITH_PRIVATE(TalkatuView, talkatu_view, GTK_TYPE_TEXT_VIEW)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static const gchar *
talkatu_view_url_from_iter(G_GNUC_UNUSED TalkatuView *view, GtkTextIter *iter) {
	GSList *tag = NULL;
	gchar *url = NULL;

	tag = gtk_text_iter_get_tags(iter);
	for(; tag != NULL; tag = g_slist_delete_link(tag, tag)) {
		if(tag->data == NULL) {
			continue;
		}

		url = g_object_get_data(G_OBJECT(tag->data), "talkatu-anchor-url");
		if(url != NULL) {
			break;
		}
	}
	g_slist_free(tag);

	return url;
}

/******************************************************************************
 * Actions
 *****************************************************************************/
static void
talkatu_view_link_open_cb(GtkWidget *widget,
                          G_GNUC_UNUSED const gchar *action_name,
                          G_GNUC_UNUSED GVariant *parameter)
{
	TalkatuView *view = TALKATU_VIEW(widget);
	TalkatuViewPrivate *priv = talkatu_view_get_instance_private(view);

	g_signal_emit(view, signals[SIG_OPEN_URL], 0, priv->url);
}

static void
talkatu_view_link_copy_cb(GtkWidget *widget,
                          G_GNUC_UNUSED const gchar *action_name,
                          G_GNUC_UNUSED GVariant *parameter)
{
	TalkatuView *view = TALKATU_VIEW(widget);
	TalkatuViewPrivate *priv = talkatu_view_get_instance_private(view);

	if(priv->url != NULL) {
		GdkClipboard *clipboard = NULL;

		clipboard = gtk_widget_get_clipboard(widget);

		gdk_clipboard_set_text(clipboard, priv->url);
	}
}

/******************************************************************************
 * Signal Handlers
 *****************************************************************************/
static void
talkatu_view_pressed_cb(GtkGestureClick *gesture,
                        G_GNUC_UNUSED guint n_press, double wx,
                        double wy, gpointer data)
{
	TalkatuView *view = TALKATU_VIEW(data);
	GtkTextIter iter;
	const gchar *url = NULL;
	gint x, y;

	gtk_text_view_window_to_buffer_coords(
		GTK_TEXT_VIEW(view),
		GTK_TEXT_WINDOW_TEXT,
		(gint)wx, (gint)wy,
		&x, &y
	);

	gtk_text_view_get_iter_at_location(GTK_TEXT_VIEW(view), &iter, x, y);

	url = talkatu_view_url_from_iter(view, &iter);
	if(url != NULL) {
		gint button = gtk_gesture_single_get_current_button(GTK_GESTURE_SINGLE(gesture));

		if(button == 1) {
			g_signal_emit(view, signals[SIG_OPEN_URL], 0, url);
		} else if(button == 2) {
			TalkatuViewPrivate *priv = talkatu_view_get_instance_private(view);

			g_clear_pointer(&priv->url, g_free);
			priv->url = g_strdup(url);

			gtk_popover_set_pointing_to(GTK_POPOVER(priv->menu),
			                            &(const GdkRectangle){ x, y, 1, 1});
			gtk_popover_popup(GTK_POPOVER(priv->menu));
		}
	}
}

static void
talkatu_view_motion_cb(G_GNUC_UNUSED GtkEventControllerMotion *controller,
                       gdouble wx, gdouble wy, gpointer data)
{
	TalkatuView *view = TALKATU_VIEW(data);
	GtkTextIter iter;
	const gchar *url = NULL;
	gint x, y;

	gtk_text_view_window_to_buffer_coords(
		GTK_TEXT_VIEW(view),
		GTK_TEXT_WINDOW_TEXT,
		(gint)wx, (gint)wy,
		&x, &y
	);

	gtk_text_view_get_iter_at_location(GTK_TEXT_VIEW(view), &iter, x, y);

	url = talkatu_view_url_from_iter(view, &iter);
	if(url != NULL) {
		gtk_widget_set_cursor_from_name(GTK_WIDGET(view), "pointer");
	} else {
		gtk_widget_set_cursor_from_name(GTK_WIDGET(view), "text");
	}
}

static void
talkatu_view_buffer_set_cb(GObject *view,
                           G_GNUC_UNUSED GParamSpec *pspec,
                           G_GNUC_UNUSED gpointer data) {
	TalkatuViewPrivate *priv = talkatu_view_get_instance_private(TALKATU_VIEW(view));
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));

	if(TALKATU_IS_BUFFER(buffer)) {
		priv->action_group = talkatu_buffer_get_action_group(TALKATU_BUFFER(buffer));
	}
}

/******************************************************************************
 * Default Signal Handlers
 *****************************************************************************/
static gboolean
talkatu_view_query_tooltip(GtkWidget *widget,
                           gint x,
                           gint y,
                           gboolean keyboard,
                           GtkTooltip *tooltip)
{
	GtkTextIter iter;
	const gchar *url = NULL;
	gint adj_x, adj_y;

	if(keyboard) {
		return GTK_WIDGET_CLASS(talkatu_view_parent_class)->query_tooltip(widget, x, y, keyboard, tooltip);
	}

	/* convert the window coordinates to match what's visible */
	gtk_text_view_window_to_buffer_coords(
		GTK_TEXT_VIEW(widget),
		GTK_TEXT_WINDOW_TEXT,
		x, y,
		&adj_x, &adj_y
	);

	/* now find the iter for what we're at */
	gtk_text_view_get_iter_at_location(
		GTK_TEXT_VIEW(widget),
		&iter,
		adj_x,
		adj_y
	);

	/* look for a url, if we have one, add it to tooltip */
	url = talkatu_view_url_from_iter(TALKATU_VIEW(widget), &iter);
	if(url != NULL) {
		gtk_tooltip_set_text(tooltip, url);

		return TRUE;
	}

	return GTK_WIDGET_CLASS(talkatu_view_parent_class)->query_tooltip(widget, x, y, keyboard, tooltip);
}

/******************************************************************************
 * GtkTextViewClass overrides
 *****************************************************************************/
static GtkTextBuffer *
talkatu_view_create_buffer(G_GNUC_UNUSED GtkTextView *view) {
	return talkatu_buffer_new(NULL);
}

/******************************************************************************
 * GObject Stuff
 *****************************************************************************/
static void
talkatu_view_finalize(GObject *obj) {
	TalkatuViewPrivate *priv = talkatu_view_get_instance_private(TALKATU_VIEW(obj));

	g_clear_pointer(&priv->url, g_free);

	G_OBJECT_CLASS(talkatu_view_parent_class)->finalize(obj);
}

static void
talkatu_view_dispose(GObject *obj) {
	TalkatuView *view = TALKATU_VIEW(obj);
	TalkatuViewPrivate *priv = talkatu_view_get_instance_private(view);

	g_clear_pointer(&priv->menu, gtk_widget_unparent);

	G_OBJECT_CLASS(talkatu_view_parent_class)->dispose(obj);
}

static void
talkatu_view_init(TalkatuView *view) {
	gtk_widget_init_template(GTK_WIDGET(view));
}

static void
talkatu_view_class_init(TalkatuViewClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
	GtkTextViewClass *text_view_class = GTK_TEXT_VIEW_CLASS(klass);

	obj_class->dispose = talkatu_view_dispose;
	obj_class->finalize = talkatu_view_finalize;

	widget_class->query_tooltip = talkatu_view_query_tooltip;

	gtk_widget_class_set_template_from_resource(
		widget_class,
		"/org/imfreedom/keep/talkatu/talkatu/ui/view.ui"
	);

	text_view_class->create_buffer = talkatu_view_create_buffer;

	/* add our signals */

	/**
	 * TalkatuView::open-url:
	 * @talkatutextview: The #TalkatuView instances.
	 * @url: The URL to open.
	 * @user_data: User supplied data.
	 *
	 * Emitted when a user clicks on a link to open the url
	 */
	signals[SIG_OPEN_URL] = g_signal_new(
		"open-url",
		G_TYPE_FROM_CLASS(klass),
		G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		G_STRUCT_OFFSET(TalkatuViewClass, open_url),
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		1,
		G_TYPE_STRING
	);

	gtk_widget_class_bind_template_callback(widget_class,
	                                        talkatu_view_pressed_cb);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        talkatu_view_motion_cb);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        talkatu_view_buffer_set_cb);

	gtk_widget_class_bind_template_child_private(widget_class, TalkatuView, menu);

	/* add our actions */
	gtk_widget_class_install_action(widget_class, "link.open", NULL,
	                                talkatu_view_link_open_cb);
	gtk_widget_class_install_action(widget_class, "link.copy", NULL,
	                                talkatu_view_link_copy_cb);

	/* add our custom keybindings */
	gtk_widget_class_add_binding_action(widget_class, GDK_KEY_b, GDK_CONTROL_MASK, TALKATU_ACTION_FORMAT_BOLD, NULL);
	gtk_widget_class_add_binding_action(widget_class, GDK_KEY_i, GDK_CONTROL_MASK, TALKATU_ACTION_FORMAT_ITALIC, NULL);
	gtk_widget_class_add_binding_action(widget_class, GDK_KEY_slash, GDK_CONTROL_MASK, TALKATU_ACTION_FORMAT_STRIKETHROUGH, NULL);
	gtk_widget_class_add_binding_action(widget_class, GDK_KEY_plus, GDK_CONTROL_MASK, TALKATU_ACTION_FORMAT_GROW, NULL);
	gtk_widget_class_add_binding_action(widget_class, GDK_KEY_equal, GDK_CONTROL_MASK, TALKATU_ACTION_FORMAT_GROW, NULL);
	gtk_widget_class_add_binding_action(widget_class, GDK_KEY_minus, GDK_CONTROL_MASK, TALKATU_ACTION_FORMAT_SHRINK, NULL);
	gtk_widget_class_add_binding_action(widget_class, GDK_KEY_r, GDK_CONTROL_MASK, TALKATU_ACTION_FORMAT_RESET, NULL);
	gtk_widget_class_add_binding_signal(widget_class, GDK_KEY_Insert, GDK_META_MASK | GDK_SHIFT_MASK, "insert-at-cursor", "s", "🐣");
}

/******************************************************************************
 * Public API
 *****************************************************************************/

/**
 * talkatu_view_new:
 *
 * Creates a new #TalkatuView with a #TalkatuBuffer.
 *
 * Returns: (transfer full): The new #TalkatuView.
 */
GtkWidget *talkatu_view_new(void) {
	return talkatu_view_new_with_buffer(talkatu_buffer_new(NULL));
}

/**
 * talkatu_view_new_with_buffer:
 * @buffer: A #GtkTextBuffer.
 *
 * Creates a new #TalkatuView with @buffer.
 *
 * Returns: (transfer full): The new #TalkatuView.
 */
GtkWidget *talkatu_view_new_with_buffer(GtkTextBuffer *buffer) {
	return g_object_new(TALKATU_TYPE_VIEW, "buffer", buffer, NULL);
}
