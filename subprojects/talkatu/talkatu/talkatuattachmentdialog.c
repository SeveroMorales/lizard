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

#include "talkatu/talkatuattachmentdialog.h"

/**
 * TalkatuAttachmentDialog:
 *
 * A #GtkDialog that allows the user to customize an image message.
 */
struct _TalkatuAttachmentDialog {
	GtkDialog parent;

	TalkatuAttachment *attachment;

	GtkWidget *preview;
	GtkWidget *filename;
	GtkWidget *comment;
};

enum {
	PROP_0 = 0,
	PROP_ATTACHMENT,
	PROP_COMMENT,
	N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES] = {NULL, };

/******************************************************************************
 * Setters
 *****************************************************************************/
static void
talkatu_attachment_dialog_set_attachment(TalkatuAttachmentDialog *dialog,
                                         TalkatuAttachment *attachment)
{
	if(g_set_object(&dialog->attachment, attachment)) {
		if(TALKATU_IS_ATTACHMENT(attachment)) {
			GIcon *preview = talkatu_attachment_get_preview(attachment);
			gchar *filename = NULL;

			if(G_IS_ICON(preview)) {
				gtk_image_set_from_gicon(GTK_IMAGE(dialog->preview), preview);
				g_object_unref(G_OBJECT(preview));
			}

			filename = talkatu_attachment_get_filename(attachment);
			gtk_label_set_text(GTK_LABEL(dialog->filename), filename);
			g_free(filename);
		}

		g_object_notify_by_pspec(G_OBJECT(dialog),
		                         properties[PROP_ATTACHMENT]);
	}
}

static void
talkatu_attachment_dialog_set_comment(TalkatuAttachmentDialog *dialog,
                                      const gchar *comment)
{
	if(GTK_IS_ENTRY(dialog->comment)) {
		gtk_editable_set_text(GTK_EDITABLE(dialog->comment), comment);

		g_object_notify_by_pspec(G_OBJECT(dialog), properties[PROP_COMMENT]);
	}
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_TYPE(TalkatuAttachmentDialog, talkatu_attachment_dialog, GTK_TYPE_DIALOG)

static void
talkatu_attachment_dialog_get_property(GObject *obj, guint prop_id,
                                       GValue *value, GParamSpec *pspec)
{
	TalkatuAttachmentDialog *dialog = TALKATU_ATTACHMENT_DIALOG(obj);

	switch(prop_id) {
		case PROP_ATTACHMENT:
			g_value_take_object(value, talkatu_attachment_dialog_get_attachment(dialog));
			break;
		case PROP_COMMENT:
			g_value_set_string(value, talkatu_attachment_dialog_get_comment(dialog));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
			break;
	}
}

static void
talkatu_attachment_dialog_set_property(GObject *obj, guint prop_id,
                                       const GValue *value, GParamSpec *pspec)
{
	TalkatuAttachmentDialog *dialog = TALKATU_ATTACHMENT_DIALOG(obj);

	switch(prop_id) {
		case PROP_ATTACHMENT:
			talkatu_attachment_dialog_set_attachment(dialog, g_value_get_object(value));
			break;
		case PROP_COMMENT:
			talkatu_attachment_dialog_set_comment(dialog,
			                                      g_value_get_string(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
			break;
	}
}

static void
talkatu_attachment_dialog_finalize(GObject *obj) {
	TalkatuAttachmentDialog *dialog = TALKATU_ATTACHMENT_DIALOG(obj);

	g_clear_object(&dialog->attachment);

	G_OBJECT_CLASS(talkatu_attachment_dialog_parent_class)->finalize(obj);
}

static void
talkatu_attachment_dialog_init(TalkatuAttachmentDialog *dialog) {
	gtk_widget_init_template(GTK_WIDGET(dialog));
}

static void
talkatu_attachment_dialog_class_init(TalkatuAttachmentDialogClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	obj_class->get_property = talkatu_attachment_dialog_get_property;
	obj_class->set_property = talkatu_attachment_dialog_set_property;
	obj_class->finalize = talkatu_attachment_dialog_finalize;

	properties[PROP_ATTACHMENT] = g_param_spec_object(
		"attachment", "attachment",
		"The attachment that this dialog is customizing",
		G_TYPE_OBJECT,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	properties[PROP_COMMENT] = g_param_spec_string(
		"comment", "comment",
		"The comment to add to the attachment",
		"",
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);

	gtk_widget_class_set_template_from_resource(
		widget_class,
		"/org/imfreedom/keep/talkatu/talkatu/ui/attachmentdialog.ui"
	);

	gtk_widget_class_bind_template_child(widget_class, TalkatuAttachmentDialog, preview);
	gtk_widget_class_bind_template_child(widget_class, TalkatuAttachmentDialog, filename);
	gtk_widget_class_bind_template_child(widget_class, TalkatuAttachmentDialog, comment);
}

/******************************************************************************
 * Public API
 *****************************************************************************/

/**
 * talkatu_attachment_dialog_new:
 * @attachment: The #TalkatuAttachment we're customizing.
 * @comment: A default comment to display.
 *
 * Creates a new #TalkatuAttachmentDialog with the given attachment and comment.
 *
 * Returns: (transfer full): The new #TalkatuAttachmentDialog.
 */
GtkWidget *
talkatu_attachment_dialog_new(TalkatuAttachment *attachment,
                              const gchar *comment)
{
	return GTK_WIDGET(g_object_new(
		TALKATU_TYPE_ATTACHMENT_DIALOG,
		"attachment", attachment,
		"comment", comment,
		NULL
	));
}

/**
 * talkatu_attachment_dialog_get_attachment:
 * @dialog: The #TalkatuAttachmentDialog.
 *
 * Gets the #TalkatuAttachment from @dialog.
 *
 * Returns: (transfer full): The #TalkatuAttachment for the file that the user
 *          selected.
 */
TalkatuAttachment *
talkatu_attachment_dialog_get_attachment(TalkatuAttachmentDialog *dialog) {
	g_return_val_if_fail(TALKATU_IS_ATTACHMENT_DIALOG(dialog), NULL);

	return g_object_ref(dialog->attachment);
}

/**
 * talkatu_attachment_dialog_get_comment:
 * @dialog: The #TalkatuAttachmentDialog.
 *
 * Get the comment the user entered, or empty string if nothing was entered.
 *
 * Returns: The comment that the user entered.
 */
const gchar *
talkatu_attachment_dialog_get_comment(TalkatuAttachmentDialog *dialog) {
	g_return_val_if_fail(TALKATU_IS_ATTACHMENT_DIALOG(dialog), "");

	if(GTK_IS_ENTRY(dialog->comment)) {
		return gtk_editable_get_text(GTK_EDITABLE(dialog->comment));
	}

	return "";
}
