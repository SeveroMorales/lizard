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

#include <talkatu/talkatuhistoryrow.h>
#include <talkatu/talkatuhtmlpangorenderer.h>
#include <talkatu/talkatuhtmlrenderer.h>

/**
 * TalkatuHistoryRow:
 *
 * A #GtkListBoxRow subclass that is used to display a #TalkatuMessage.
 */
struct _TalkatuHistoryRow {
	GtkListBoxRow parent;

	TalkatuMessage *message;

	GtkWidget *avatar_image;
	GtkWidget *author;
	GtkWidget *timestamp;
	GtkWidget *edited;
	GtkWidget *content;
};

enum {
	PROP_0 = 0,
	PROP_MESSAGE,
	N_PROPERTIES,
};
static GParamSpec *properties[N_PROPERTIES] = {NULL, };

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
talkatu_history_row_update(TalkatuHistoryRow *row) {
	if(!TALKATU_IS_MESSAGE(row->message)) {
		gtk_label_set_text(GTK_LABEL(row->author), _("Unknown"));
		gtk_label_set_text(GTK_LABEL(row->timestamp), _("Unknown"));
		gtk_label_set_text(GTK_LABEL(row->content),
		                   _("This message was lost."));
	} else {
		GDateTime *timestamp = NULL;
		TalkatuContentType content_type;
		GdkRGBA *author_name_color = NULL;
		PangoAttrList *author_attrs = NULL;
		PangoAttribute *attr = NULL;
		gchar *author = NULL, *contents = NULL, *datetime = NULL;
		gboolean edited = FALSE;

		/* g_object_get will stop getting properties at the first error. So when
		 * a new property is added, it needs to be appended to the end so that
		 * we don't break implementations that haven't added the latest
		 * properties yet.
		 */
		g_object_get(
			G_OBJECT(row->message),
			"author", &author,
			"content-type", &content_type,
			"contents", &contents,
			"edited", &edited,
			"timestamp", &timestamp,
			"author-name-color", &author_name_color,
			NULL
		);

		author_attrs = pango_attr_list_new();
		if(author_name_color != NULL) {
			attr = pango_attr_foreground_new(0xFFFF * author_name_color->red,
			                                 0xFFFF * author_name_color->green,
			                                 0xFFFF * author_name_color->blue);
			pango_attr_list_insert(author_attrs, attr);
		}
		gtk_label_set_attributes(GTK_LABEL(row->author), author_attrs);
		pango_attr_list_unref(author_attrs);
		gtk_label_set_text(GTK_LABEL(row->author), author);

		datetime = g_date_time_format(timestamp, "%I:%M %P");
		gtk_label_set_text(GTK_LABEL(row->timestamp), datetime);

		if(edited) {
			gtk_widget_show(row->edited);
		} else {
			gtk_widget_hide(row->edited);
		}

		if(content_type == TALKATU_CONTENT_TYPE_PANGO) {
			gtk_label_set_markup(GTK_LABEL(row->content), contents);
		} else if(content_type == TALKATU_CONTENT_TYPE_HTML) {
			TalkatuHtmlRenderer *renderer = NULL;
			TalkatuHtmlPangoRenderer *pr = NULL;

			renderer = talkatu_html_pango_renderer_new();
			pr = TALKATU_HTML_PANGO_RENDERER(renderer);

			talkatu_html_renderer_render(renderer, contents);

			gtk_label_set_markup(GTK_LABEL(row->content),
			                     talkatu_html_pango_renderer_get_string(pr));

			g_object_unref(G_OBJECT(renderer));
		} else {
			gtk_label_set_text(GTK_LABEL(row->content), contents);
		}

		g_free(author);
		g_free(datetime);
		g_free(contents);
		g_clear_pointer(&timestamp, g_date_time_unref);
		g_clear_pointer(&author_name_color, gdk_rgba_free);
	}
}

static TalkatuMessage *
talkatu_history_row_get_message(TalkatuHistoryRow *row) {
	if(row->message == NULL) {
		return NULL;
	}

	return TALKATU_MESSAGE(g_object_ref(G_OBJECT(row->message)));
}

static void
talkatu_history_row_set_message(TalkatuHistoryRow *row,
                                TalkatuMessage *message)
{
	if(g_set_object(&row->message, message)) {
		talkatu_history_row_update(row);

		g_object_notify_by_pspec(G_OBJECT(row), properties[PROP_MESSAGE]);
	}
}

/******************************************************************************
 * GObject Stuff
 *****************************************************************************/
G_DEFINE_TYPE(TalkatuHistoryRow, talkatu_history_row, GTK_TYPE_LIST_BOX_ROW)

static void
talkatu_history_row_get_property(GObject *obj, guint param_id, GValue *value,
                                 GParamSpec *pspec)
{
	TalkatuHistoryRow *row = TALKATU_HISTORY_ROW(obj);

	switch(param_id) {
		case PROP_MESSAGE:
			g_value_set_object(value, talkatu_history_row_get_message(row));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
talkatu_history_row_set_property(GObject *obj, guint param_id,
                                 const GValue *value, GParamSpec *pspec)
{
	TalkatuHistoryRow *row = TALKATU_HISTORY_ROW(obj);

	switch(param_id) {
		case PROP_MESSAGE:
			talkatu_history_row_set_message(row, g_value_get_object(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
talkatu_history_row_finalize(GObject *obj) {
	TalkatuHistoryRow *row = TALKATU_HISTORY_ROW(obj);

	g_clear_object(&row->message);

	G_OBJECT_CLASS(talkatu_history_row_parent_class)->finalize(obj);
}

static void
talkatu_history_row_init(TalkatuHistoryRow *row) {
	gtk_widget_init_template(GTK_WIDGET(row));
}

static void
talkatu_history_row_class_init(TalkatuHistoryRowClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	obj_class->get_property = talkatu_history_row_get_property;
	obj_class->set_property = talkatu_history_row_set_property;
	obj_class->finalize = talkatu_history_row_finalize;

	/**
	 * TalkatuHistoryRow::message:
	 *
	 * The message that this row is displaying.
	 */
	properties[PROP_MESSAGE] = g_param_spec_object(
		"message", "message", "The message to display",
		G_TYPE_OBJECT,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS
	);

	gtk_widget_class_set_template_from_resource(
		widget_class,
		"/org/imfreedom/keep/talkatu/talkatu/ui/historyrow.ui"
	);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);

	gtk_widget_class_bind_template_child(widget_class, TalkatuHistoryRow,
	                                     avatar_image);
	gtk_widget_class_bind_template_child(widget_class, TalkatuHistoryRow,
	                                     author);
	gtk_widget_class_bind_template_child(widget_class, TalkatuHistoryRow,
	                                     timestamp);
	gtk_widget_class_bind_template_child(widget_class, TalkatuHistoryRow,
	                                     edited);
	gtk_widget_class_bind_template_child(widget_class, TalkatuHistoryRow,
	                                     content);
}

/******************************************************************************
 * Public API
 *****************************************************************************/

/**
 * talkatu_history_row_new:
 * @message: The #TalkatuMessage instance that will be displayed.
 *
 * Creates a new #TalkatuHistoryRow for displaying @message.
 *
 * Returns: (transfer full): The new #TalkatuHistoryRow instance.
 */
GtkWidget *talkatu_history_row_new(TalkatuMessage *message) {
	g_return_val_if_fail(TALKATU_IS_MESSAGE(message), NULL);

	return GTK_WIDGET(g_object_new(
		TALKATU_TYPE_HISTORY_ROW,
		"message", message,
		NULL
	));
}
