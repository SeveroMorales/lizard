/*
 * purple
 *
 * Purple is the legal property of its developers, whose names are too numerous
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
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, see <https://www.gnu.org/licenses/>.
 */

#include "libpurple/purpleattachment.h"

#include <glib/gi18n-lib.h>

struct _PurpleAttachment {
	GObject parent;

	guint64 id;
	gchar *content_type;

	gchar *local_uri;
	gchar *remote_uri;

	guint64 size;
};

G_DEFINE_TYPE(PurpleAttachment, purple_attachment, G_TYPE_OBJECT);

enum {
	PROP_0 = 0,
	PROP_ID,
	PROP_CONTENT_TYPE,
	PROP_LOCAL_URI,
	PROP_REMOTE_URI,
	PROP_SIZE,
	N_PROPERTIES,
};
static GParamSpec *properties[N_PROPERTIES];

/******************************************************************************
 * Private Setters
 *****************************************************************************/
static void
purple_attachment_set_content_type(PurpleAttachment *attachment,
                                    const gchar *content_type)
{
	if(attachment->content_type == content_type) {
		return;
	}

	g_clear_pointer(&attachment->content_type, g_free);

	attachment->content_type = g_strdup(content_type);

	g_object_notify_by_pspec(G_OBJECT(attachment),
	                         properties[PROP_CONTENT_TYPE]);
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
purple_attachment_get_property(GObject *obj, guint prop_id, GValue *value, GParamSpec *pspec) {
	PurpleAttachment *attachment = PURPLE_ATTACHMENT(obj);

	switch(prop_id) {
		case PROP_ID:
			g_value_set_uint64(value, purple_attachment_get_id(attachment));
			break;
		case PROP_CONTENT_TYPE:
			g_value_set_string(value, purple_attachment_get_content_type(attachment));
			break;
		case PROP_LOCAL_URI:
			g_value_set_string(value, purple_attachment_get_local_uri(attachment));
			break;
		case PROP_REMOTE_URI:
			g_value_set_string(value, purple_attachment_get_remote_uri(attachment));
			break;
		case PROP_SIZE:
			g_value_set_uint64(value, purple_attachment_get_size(attachment));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
			break;
	}
}

static void
purple_attachment_set_property(GObject *obj, guint prop_id, const GValue *value, GParamSpec *pspec) {
	PurpleAttachment *attachment = PURPLE_ATTACHMENT(obj);

	switch(prop_id) {
		case PROP_ID:
			purple_attachment_set_id(attachment, g_value_get_uint64(value));
			break;
		case PROP_CONTENT_TYPE:
			purple_attachment_set_content_type(attachment, g_value_get_string(value));
			break;
		case PROP_LOCAL_URI:
			purple_attachment_set_local_uri(attachment, g_value_get_string(value));
			break;
		case PROP_REMOTE_URI:
			purple_attachment_set_remote_uri(attachment, g_value_get_string(value));
			break;
		case PROP_SIZE:
			purple_attachment_set_size(attachment, g_value_get_uint64(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
			break;
	}
}

static void
purple_attachment_finalize(GObject *obj) {
	PurpleAttachment *attachment = PURPLE_ATTACHMENT(obj);

	g_clear_pointer(&attachment->content_type, g_free);
	g_clear_pointer(&attachment->local_uri, g_free);
	g_clear_pointer(&attachment->remote_uri, g_free);

	G_OBJECT_CLASS(purple_attachment_parent_class)->finalize(obj);
}

static void
purple_attachment_init(G_GNUC_UNUSED PurpleAttachment *attachment) {
}

static void
purple_attachment_class_init(PurpleAttachmentClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->get_property = purple_attachment_get_property;
	obj_class->set_property = purple_attachment_set_property;
	obj_class->finalize = purple_attachment_finalize;

	/* add our properties */
	properties[PROP_ID] = g_param_spec_uint64(
		"id", "id", "The identifier of the attachment",
		0, G_MAXUINT64, 0,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS
	);

	properties[PROP_CONTENT_TYPE] = g_param_spec_string(
		"content-type", "content-type", "The content type of the attachment",
		"application/octet-stream",
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS
	);

	properties[PROP_LOCAL_URI] = g_param_spec_string(
		"local-uri", "local-uri", "The local URI of the attachment",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS
	);

	properties[PROP_REMOTE_URI] = g_param_spec_string(
		"remote-uri", "remote-uri", "The remote URI of the attachment",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS
	);

	properties[PROP_SIZE] = g_param_spec_uint64(
		"size", "size", "The file size of the attachment in bytes",
		0, G_MAXUINT64, 0,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS
	);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

/******************************************************************************
 * Public API
 *****************************************************************************/

PurpleAttachment *
purple_attachment_new(guint64 id, const gchar *content_type) {
	g_return_val_if_fail(content_type != NULL, NULL);

	return PURPLE_ATTACHMENT(g_object_new(
		PURPLE_TYPE_ATTACHMENT,
		"id", id,
		"content-type", content_type,
		NULL
	));
}

guint64
purple_attachment_get_id(PurpleAttachment *attachment) {
	g_return_val_if_fail(PURPLE_IS_ATTACHMENT(attachment), 0);

	return attachment->id;
}

guint64 *
purple_attachment_get_hash_key(PurpleAttachment *attachment) {
	g_return_val_if_fail(PURPLE_IS_ATTACHMENT(attachment), NULL);

	return &attachment->id;
}

void
purple_attachment_set_id(PurpleAttachment *attachment, guint64 id) {
	g_return_if_fail(PURPLE_IS_ATTACHMENT(attachment));

	if(attachment->id == id) {
		return;
	}

	attachment->id = id;

	g_object_notify_by_pspec(G_OBJECT(attachment), properties[PROP_ID]);
}

const gchar *
purple_attachment_get_content_type(PurpleAttachment *attachment) {
	g_return_val_if_fail(PURPLE_IS_ATTACHMENT(attachment), NULL);

	return attachment->content_type;
}

const gchar *
purple_attachment_get_local_uri(PurpleAttachment *attachment) {
	g_return_val_if_fail(PURPLE_IS_ATTACHMENT(attachment), NULL);

	return attachment->local_uri;
}

void
purple_attachment_set_local_uri(PurpleAttachment *attachment,
                                 const gchar *local_uri)
{
	g_return_if_fail(PURPLE_IS_ATTACHMENT(attachment));

	if(attachment->local_uri == local_uri) {
		return;
	}

	g_free(attachment->local_uri);

	if(local_uri != NULL) {
		gchar *scheme = g_uri_parse_scheme(local_uri);
		if(scheme == NULL) {
			attachment->local_uri = g_filename_to_uri(local_uri, NULL, NULL);
		} else {
			g_free(scheme);
			attachment->local_uri = g_strdup(local_uri);
		}
	} else {
		attachment->local_uri = NULL;
	}

	g_object_notify_by_pspec(G_OBJECT(attachment), properties[PROP_LOCAL_URI]);
}

const gchar *
purple_attachment_get_remote_uri(PurpleAttachment *attachment) {
	g_return_val_if_fail(PURPLE_IS_ATTACHMENT(attachment), NULL);

	return attachment->remote_uri;
}

void
purple_attachment_set_remote_uri(PurpleAttachment *attachment,
                                  const gchar *remote_uri)
{
	g_return_if_fail(PURPLE_IS_ATTACHMENT(attachment));

	if(attachment->remote_uri == remote_uri) {
		return;
	}

	g_free(attachment->remote_uri);
	attachment->remote_uri = g_strdup(remote_uri);

	g_object_notify_by_pspec(G_OBJECT(attachment), properties[PROP_REMOTE_URI]);
}

guint64
purple_attachment_get_size(PurpleAttachment *attachment) {
	g_return_val_if_fail(PURPLE_IS_ATTACHMENT(attachment), 0);

	return attachment->size;
}

void
purple_attachment_set_size(PurpleAttachment *attachment, guint64 size) {
	g_return_if_fail(PURPLE_IS_ATTACHMENT(attachment));

	attachment->size = size;

	g_object_notify_by_pspec(G_OBJECT(attachment), properties[PROP_SIZE]);
}

gchar *
purple_attachment_get_filename(PurpleAttachment *attachment) {
	g_return_val_if_fail(PURPLE_IS_ATTACHMENT(attachment), NULL);

	if(attachment->remote_uri != NULL && attachment->remote_uri[0] != '\0') {
		return g_path_get_basename(attachment->remote_uri);
	}

	if(attachment->local_uri != NULL && attachment->local_uri[0] != '\0') {
		return g_path_get_basename(attachment->local_uri);
	}

	return g_strdup("unknown");
}
