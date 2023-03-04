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

#include "talkatu/talkatuattachmentpreview.h"

/**
 * TalkatuAttachmentPreview:
 *
 * A #GtkWidget that displays a preview of a #TalkatuAttachment and allows the
 * user to save it.
 */
struct _TalkatuAttachmentPreview {
	GtkWidget parent;

	TalkatuAttachment *attachment;

	GtkWidget *preview;
	GtkWidget *filename;
	GtkWidget *filesize;
};

enum {
	PROP_0 = 0,
	PROP_ATTACHMENT,
	N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES] = {NULL, };

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static void
talkatu_attachment_preview_download_cb(G_GNUC_UNUSED GtkInfoBar *self,
                                       G_GNUC_UNUSED gint response_id,
                                       G_GNUC_UNUSED gpointer user_data)
{
}

/******************************************************************************
 * Setters
 *****************************************************************************/
static void
talkatu_attachment_preview_set_attachment(TalkatuAttachmentPreview *preview,
                                          TalkatuAttachment *attachment)
{
	if(g_set_object(&preview->attachment, attachment)) {
		if(TALKATU_IS_ATTACHMENT(attachment)) {
			GIcon *icon = talkatu_attachment_get_preview(attachment);
			gchar *filename = NULL;
			gchar *filesize = NULL;

			if(G_IS_ICON(icon)) {
				gtk_image_set_from_gicon(GTK_IMAGE(preview->preview), icon);
				g_object_unref(G_OBJECT(icon));
			}

			filename = talkatu_attachment_get_filename(preview->attachment);
			gtk_label_set_text(GTK_LABEL(preview->filename), filename);
			g_free(filename);

			filesize = g_format_size(talkatu_attachment_get_size(preview->attachment));
			gtk_label_set_text(GTK_LABEL(preview->filesize), filesize);
			g_free(filesize);
		}

		g_object_notify_by_pspec(G_OBJECT(preview),
		                         properties[PROP_ATTACHMENT]);
	}
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_TYPE(TalkatuAttachmentPreview, talkatu_attachment_preview, GTK_TYPE_WIDGET)

static void
talkatu_attachment_preview_get_property(GObject *obj, guint prop_id,
                                       GValue *value, GParamSpec *pspec)
{
	TalkatuAttachmentPreview *preview = TALKATU_ATTACHMENT_PREVIEW(obj);

	switch(prop_id) {
		case PROP_ATTACHMENT:
			g_value_take_object(value, talkatu_attachment_preview_get_attachment(preview));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
			break;
	}
}

static void
talkatu_attachment_preview_set_property(GObject *obj, guint prop_id,
                                       const GValue *value, GParamSpec *pspec)
{
	TalkatuAttachmentPreview *preview = TALKATU_ATTACHMENT_PREVIEW(obj);

	switch(prop_id) {
		case PROP_ATTACHMENT:
			talkatu_attachment_preview_set_attachment(preview, g_value_get_object(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
			break;
	}
}

static void
talkatu_attachment_preview_finalize(GObject *obj) {
	TalkatuAttachmentPreview *preview = TALKATU_ATTACHMENT_PREVIEW(obj);

	g_clear_object(&preview->attachment);

	G_OBJECT_CLASS(talkatu_attachment_preview_parent_class)->finalize(obj);
}

static void
talkatu_attachment_preview_init(TalkatuAttachmentPreview *preview) {
	gtk_widget_init_template(GTK_WIDGET(preview));
}

static void
talkatu_attachment_preview_class_init(TalkatuAttachmentPreviewClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	obj_class->get_property = talkatu_attachment_preview_get_property;
	obj_class->set_property = talkatu_attachment_preview_set_property;
	obj_class->finalize = talkatu_attachment_preview_finalize;

	properties[PROP_ATTACHMENT] = g_param_spec_object(
		"attachment", "attachment",
		"The attachment that this preview is customizing",
		G_TYPE_OBJECT,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);

	gtk_widget_class_set_template_from_resource(
		widget_class,
		"/org/imfreedom/keep/talkatu/talkatu/ui/attachmentpreview.ui"
	);

	gtk_widget_class_bind_template_child(widget_class, TalkatuAttachmentPreview, preview);
	gtk_widget_class_bind_template_child(widget_class, TalkatuAttachmentPreview, filename);
	gtk_widget_class_bind_template_child(widget_class, TalkatuAttachmentPreview, filesize);

	gtk_widget_class_bind_template_callback(widget_class, talkatu_attachment_preview_download_cb);
}

/******************************************************************************
 * Public API
 *****************************************************************************/

/**
 * talkatu_attachment_preview_new:
 * @attachment: The #TalkatuAttachment we're previewing.
 *
 * Creates a new #TalkatuAttachmentPreview for the given attachment.
 *
 * Returns: (transfer full): The new #TalkatuAttachmentPreview.
 */
GtkWidget *
talkatu_attachment_preview_new(TalkatuAttachment *attachment) {
	g_return_val_if_fail(TALKATU_IS_ATTACHMENT(attachment), NULL);

	return GTK_WIDGET(g_object_new(
		TALKATU_TYPE_ATTACHMENT_PREVIEW,
		"attachment", attachment,
		NULL
	));
}

/**
 * talkatu_attachment_preview_get_attachment:
 * @preview: The #TalkatuAttachmentPreview.
 *
 * Gets the #TalkatuAttachment from @preview.
 *
 * Returns: (transfer full): The #TalkatuAttachment from @preview.
 */
TalkatuAttachment *
talkatu_attachment_preview_get_attachment(TalkatuAttachmentPreview *preview) {
	g_return_val_if_fail(TALKATU_IS_ATTACHMENT_PREVIEW(preview), NULL);

	return g_object_ref(preview->attachment);
}
