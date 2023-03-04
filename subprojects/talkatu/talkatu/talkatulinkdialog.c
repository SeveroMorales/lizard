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

#include <gtk/gtk.h>

#include "talkatu/talkatulinkdialog.h"

/**
 * TalkatuLinkDialog:
 *
 * A simple #GtkDialog subclass to allow the user to create a link.
 */
struct _TalkatuLinkDialog {
	GtkDialog parent;

	GtkWidget *url;
	GtkWidget *display;
};

G_DEFINE_TYPE(TalkatuLinkDialog, talkatu_link_dialog, GTK_TYPE_DIALOG)

/******************************************************************************
 * GObject Stuff
 *****************************************************************************/
static void
talkatu_link_dialog_init(TalkatuLinkDialog *dialog) {
	gtk_widget_init_template(GTK_WIDGET(dialog));
}

static void
talkatu_link_dialog_class_init(TalkatuLinkDialogClass *klass) {
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	gtk_widget_class_set_template_from_resource(
		widget_class,
		"/org/imfreedom/keep/talkatu/talkatu/ui/linkdialog.ui"
	);

	gtk_widget_class_bind_template_child(widget_class, TalkatuLinkDialog, url);
	gtk_widget_class_bind_template_child(widget_class, TalkatuLinkDialog, display);
}

/******************************************************************************
 * Public API
 *****************************************************************************/

/**
 * talkatu_link_dialog_new:
 *
 * Creates a new #TalkatuLinkDialog with a #TalkatuBuffer.
 *
 * Returns: (transfer full): The new #TalkatuLinkDialog.
 */
GtkWidget *
talkatu_link_dialog_new(void) {
	return GTK_WIDGET(g_object_new(
		TALKATU_TYPE_LINK_DIALOG,
		NULL
	));
}

/**
 * talkatu_link_dialog_get_url:
 * @dialog: The #TalkatuLinkDialog.
 *
 * Gets the URL that the user entered with leading and trailing whitespace
 * removed.
 *
 * Returns: (transfer full): The URL that the user entered.
 */
gchar *
talkatu_link_dialog_get_url(TalkatuLinkDialog *dialog) {
	g_return_val_if_fail(TALKATU_IS_LINK_DIALOG(dialog), NULL);

	if(GTK_IS_ENTRY(dialog->url)) {
		gchar *raw = NULL, *ret = NULL;

		raw = g_strdup(gtk_editable_get_text(GTK_EDITABLE(dialog->url)));
		ret = g_strdup(g_strstrip(raw));
		g_free(raw);

		return ret;
	}

	return NULL;
}

/**
 * talkatu_link_dialog_get_display_text:
 * @dialog: The #TalkatuLinkDialog.
 *
 * Gets the display text that the user entered with leading and trailing
 * whitespace removed.
 *
 * Returns: (transfer full): The display text that the user entered.
 */
gchar *
talkatu_link_dialog_get_display_text(TalkatuLinkDialog *dialog) {
	g_return_val_if_fail(TALKATU_IS_LINK_DIALOG(dialog), NULL);

	if(GTK_IS_ENTRY(dialog->display)) {
		gchar *ret = NULL;

		ret = g_strdup(gtk_editable_get_text(GTK_EDITABLE(dialog->display)));
		return g_strstrip(ret);
	}

	return NULL;
}
