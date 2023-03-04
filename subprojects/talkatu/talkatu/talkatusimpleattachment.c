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

#include "talkatu/talkatusimpleattachment.h"

#include <glib/gi18n-lib.h>

/**
 * TalkatuSimpleAttachment:
 *
 * #TalkatuSimpleAttachment is simple implementation of the #TalkatuAttachment
 * interface.  This is the easiest way to create a #TalkatuAttachment and
 * attach it to a #TalkatuMessage.
 */
struct _TalkatuSimpleAttachment {
	GObject parent;

	guint64 id;
	gchar *content_type;

	gchar *local_uri;
	gchar *remote_uri;

	guint64 size;
};

enum {
	PROP_0 = 0,
	N_PROPERTIES,
	/* overrides */
	PROP_ID = N_PROPERTIES,
	PROP_CONTENT_TYPE,
	PROP_LOCAL_URI,
	PROP_REMOTE_URI,
	PROP_SIZE,
};

/******************************************************************************
 * TalkatuAttachment Implementation
 *****************************************************************************/
static guint64
talkatu_simple_attachment_get_id(TalkatuSimpleAttachment *attachment) {
	return attachment->id;
}

static guint64 *
talkatu_simple_attachment_get_hash_key(TalkatuAttachment *attachment) {
	TalkatuSimpleAttachment *simple = TALKATU_SIMPLE_ATTACHMENT(attachment);

	return &simple->id;
}

static void
talkatu_simple_attachment_set_id(TalkatuSimpleAttachment *attachment,
                                 guint64 id)
{
	attachment->id = id;

	g_object_notify(G_OBJECT(attachment), "id");
}

static gchar *
talkatu_simple_attachment_get_content_type(TalkatuSimpleAttachment *attachment) {
	return g_strdup(attachment->content_type);
}

static void
talkatu_simple_attachment_set_content_type(TalkatuSimpleAttachment *attachment,
                                           const gchar *content_type)
{
	g_clear_pointer(&attachment->content_type, g_free);

	attachment->content_type = g_strdup(content_type);

	g_object_notify(G_OBJECT(attachment), "content-type");
}

static gchar *
talkatu_simple_attachment_get_local_uri(TalkatuSimpleAttachment *attachment) {
	return g_strdup(attachment->local_uri);
}

static void
talkatu_simple_attachment_set_local_uri(TalkatuSimpleAttachment *attachment,
                                        const gchar *local_uri)
{
	g_free(attachment->local_uri);

	if(local_uri != NULL) {
		gchar *scheme  = g_uri_parse_scheme(local_uri);
		if(scheme == NULL) {
			attachment->local_uri = g_filename_to_uri(local_uri, NULL, NULL);
		} else {
			g_free(scheme);
			attachment->local_uri = g_strdup(local_uri);
		}
	} else {
		attachment->local_uri = NULL;
	}

	g_object_notify(G_OBJECT(attachment), "local-uri");
}

static gchar *
talkatu_simple_attachment_get_remote_uri(TalkatuSimpleAttachment *attachment) {
	return g_strdup(attachment->remote_uri);
}

static void
talkatu_simple_attachment_set_remote_uri(TalkatuSimpleAttachment *attachment,
                                         const gchar *remote_uri)
{
	g_free(attachment->remote_uri);
	attachment->remote_uri = g_strdup(remote_uri);

	g_object_notify(G_OBJECT(attachment), "remote-uri");
}

static guint64
talkatu_simple_attachment_get_size(TalkatuSimpleAttachment *attachment) {
	return attachment->size;
}

static void
talkatu_simple_attachment_set_size(TalkatuSimpleAttachment *attachment,
                                   guint64 size)
{
	attachment->size = size;

	g_object_notify(G_OBJECT(attachment), "size");
}

static void
talkatu_simple_attachment_attachment_init(TalkatuAttachmentInterface *iface) {
	iface->get_hash_key = talkatu_simple_attachment_get_hash_key;
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_TYPE_EXTENDED(
	TalkatuSimpleAttachment,
	talkatu_simple_attachment,
	G_TYPE_OBJECT,
	0,
	G_IMPLEMENT_INTERFACE(TALKATU_TYPE_ATTACHMENT,
	                      talkatu_simple_attachment_attachment_init)
);

static void
talkatu_simple_attachment_get_property(GObject *obj, guint prop_id,
                                       GValue *value, GParamSpec *pspec)
{
	TalkatuSimpleAttachment *attachment = TALKATU_SIMPLE_ATTACHMENT(obj);

	switch(prop_id) {
		case PROP_ID:
			g_value_set_uint64(value, talkatu_simple_attachment_get_id(attachment));
			break;
		case PROP_CONTENT_TYPE:
			g_value_take_string(value, talkatu_simple_attachment_get_content_type(attachment));
			break;
		case PROP_LOCAL_URI:
			g_value_take_string(value, talkatu_simple_attachment_get_local_uri(attachment));
			break;
		case PROP_REMOTE_URI:
			g_value_take_string(value, talkatu_simple_attachment_get_remote_uri(attachment));
			break;
		case PROP_SIZE:
			g_value_set_uint64(value, talkatu_simple_attachment_get_size(attachment));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
			break;
	}
}

static void
talkatu_simple_attachment_set_property(GObject *obj, guint prop_id,
                                       const GValue *value, GParamSpec *pspec)
{
	TalkatuSimpleAttachment *attachment = TALKATU_SIMPLE_ATTACHMENT(obj);

	switch(prop_id) {
		case PROP_ID:
			talkatu_simple_attachment_set_id(attachment, g_value_get_uint64(value));
			break;
		case PROP_CONTENT_TYPE:
			talkatu_simple_attachment_set_content_type(attachment, g_value_get_string(value));
			break;
		case PROP_LOCAL_URI:
			talkatu_simple_attachment_set_local_uri(attachment, g_value_get_string(value));
			break;
		case PROP_REMOTE_URI:
			talkatu_simple_attachment_set_remote_uri(attachment, g_value_get_string(value));
			break;
		case PROP_SIZE:
			talkatu_simple_attachment_set_size(attachment, g_value_get_uint64(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
			break;
	}
}

static void
talkatu_simple_attachment_finalize(GObject *obj) {
	TalkatuSimpleAttachment *attachment = TALKATU_SIMPLE_ATTACHMENT(obj);

	g_clear_pointer(&attachment->content_type, g_free);
	g_clear_pointer(&attachment->local_uri, g_free);
	g_clear_pointer(&attachment->remote_uri, g_free);

	G_OBJECT_CLASS(talkatu_simple_attachment_parent_class)->finalize(obj);
}

static void
talkatu_simple_attachment_init(G_GNUC_UNUSED TalkatuSimpleAttachment *attachment) {
}

static void
talkatu_simple_attachment_class_init(TalkatuSimpleAttachmentClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->get_property = talkatu_simple_attachment_get_property;
	obj_class->set_property = talkatu_simple_attachment_set_property;
	obj_class->finalize = talkatu_simple_attachment_finalize;

	/* add our overridden properties */
	g_object_class_override_property(obj_class, PROP_ID, "id");
	g_object_class_override_property(obj_class, PROP_CONTENT_TYPE, "content-type");
	g_object_class_override_property(obj_class, PROP_LOCAL_URI, "local-uri");
	g_object_class_override_property(obj_class, PROP_REMOTE_URI, "remote-uri");
	g_object_class_override_property(obj_class, PROP_SIZE, "size");
}

/******************************************************************************
 * Public API
 *****************************************************************************/

/**
 * talkatu_simple_attachment_new:
 * @id: The identifier of the attachment.
 * @content_type: The content type of the attachment.
 *
 * Creates a new attachment with @content_type.
 *
 * Returns: (transfer full): The new #TalkatuSimpleAttachment.
 */
TalkatuAttachment *
talkatu_simple_attachment_new(guint64 id, const gchar *content_type) {
	g_return_val_if_fail(content_type != NULL, NULL);

	return TALKATU_ATTACHMENT(g_object_new(
		TALKATU_TYPE_SIMPLE_ATTACHMENT,
		"id", id,
		"content-type", content_type,
		NULL
	));
}
