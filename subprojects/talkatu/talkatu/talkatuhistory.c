/*
 * Talkatu - GTK widgets for chat applications
 * Copyright (C) 2017-2022 Gary Kramlich <grim@reaperworld.com>
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

#include <gtk/gtk.h>

#include <talkatu/talkatuhistory.h>

#include <talkatu/talkatuhistoryrow.h>
#include <talkatu/talkatumessage.h>

/**
 * TalkatuHistory:
 *
 * A #TalkatuView subclass that is used to display a conversation.
 */
struct _TalkatuHistory {
	GtkWidget parent;

	GtkWidget *list_box;
};

G_DEFINE_TYPE(TalkatuHistory, talkatu_history, GTK_TYPE_WIDGET)

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
talkatu_history_dispose(GObject *obj) {
	TalkatuHistory *history = TALKATU_HISTORY(obj);

	g_clear_pointer(&history->list_box, gtk_widget_unparent);

	G_OBJECT_CLASS(talkatu_history_parent_class)->dispose(obj);
}

static void
talkatu_history_init(TalkatuHistory *history) {
	gtk_widget_init_template(GTK_WIDGET(history));
}

static void
talkatu_history_class_init(TalkatuHistoryClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	obj_class->dispose = talkatu_history_dispose;

	gtk_widget_class_set_template_from_resource(
		widget_class,
		"/org/imfreedom/keep/talkatu/talkatu/ui/history.ui"
	);

	gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BIN_LAYOUT);

	gtk_widget_class_bind_template_child(widget_class, TalkatuHistory,
	                                     list_box);
}

/******************************************************************************
 * Public API
 *****************************************************************************/

/**
 * talkatu_history_new:
 *
 * Creates a new #TalkatuHistory for displaying chat history.
 *
 * Returns: (transfer full): The new #TalkatuHistory instance.
 */
GtkWidget *talkatu_history_new(void) {
	return g_object_new(TALKATU_TYPE_HISTORY, NULL);
}

/**
 * talkatu_history_write_message:
 * @history: The #TalkatuHistory instance.
 * @message: The #TalkatuMessage to add to @history.
 *
 * Adds @message to @history.  Messages are sorted by timestamp so make sure
 * it is set correctly.
 */
void
talkatu_history_write_message(TalkatuHistory *history,
                              TalkatuMessage *message)
{
	GtkWidget *row = NULL;

	g_return_if_fail(TALKATU_IS_HISTORY(history));
	g_return_if_fail(TALKATU_IS_MESSAGE(message));

	row = talkatu_history_row_new(message);
	gtk_list_box_append(GTK_LIST_BOX(history->list_box), row);
}
