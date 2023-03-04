/*
 * Pidgin - Internet Messenger
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#include "pidginmessage.h"

#include "pidginattachment.h"
#include "pidgincolor.h"

struct _PidginMessage {
	GObject parent;

	PurpleMessage *message;
};

typedef struct {
	TalkatuAttachmentForeachFunc func;
	gpointer data;
} PidginMessageAttachmentForeachData;

enum {
	PROP_0,
	PROP_MESSAGE,
	N_PROPERTIES,
	/* overrides */
	PROP_ID = N_PROPERTIES,
	PROP_CONTENT_TYPE,
	PROP_AUTHOR,
	PROP_AUTHOR_NAME_COLOR,
	PROP_CONTENTS,
	PROP_TIMESTAMP,
	PROP_EDITED,
};
static GParamSpec *properties[N_PROPERTIES] = {NULL, };

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
pidgin_message_set_message(PidginMessage *message, PurpleMessage *purple_msg) {
	if(g_set_object(&message->message, purple_msg)) {
		g_object_notify_by_pspec(G_OBJECT(message), properties[PROP_MESSAGE]);
	}
}

static GdkRGBA *
pidgin_message_parse_author_name_color(PidginMessage *message) {
	GdkRGBA *ret = NULL;
	const gchar *color = NULL;
	gboolean set = FALSE;

	ret = g_new0(GdkRGBA, 1);

	color = purple_message_get_author_name_color(message->message);
	if(color != NULL && gdk_rgba_parse(ret, color)) {
		set = TRUE;
	}

	if(!set) {
		const gchar *author = purple_message_get_author_alias(message->message);

		if(author == NULL) {
			author = purple_message_get_author(message->message);
		}

		pidgin_color_calculate_for_text(author, ret);
	}

	return ret;
}

/******************************************************************************
 * TalkatuMessage Implementation
 *****************************************************************************/
static gboolean
pidgin_message_add_attachment(TalkatuMessage *tmessage,
                              TalkatuAttachment *tattachment)
{
	PidginMessage *pmessage = PIDGIN_MESSAGE(tmessage);
	PurpleAttachment *pattachment = NULL;
	gboolean ret = FALSE;

	pattachment = purple_attachment_new(
		talkatu_attachment_get_id(tattachment),
		talkatu_attachment_get_content_type(tattachment)
	);

	ret = purple_message_add_attachment(pmessage->message, pattachment);

	g_object_unref(G_OBJECT(pattachment));

	return ret;
}

static gboolean
pidgin_message_remove_attachment(TalkatuMessage *tmessage, guint64 id) {
	PidginMessage *pmessage = PIDGIN_MESSAGE(tmessage);

	return purple_message_remove_attachment(pmessage->message, id);
}

static TalkatuAttachment *
pidgin_message_get_attachment(TalkatuMessage *tmessage, guint64 id) {
	PidginMessage *pmessage = PIDGIN_MESSAGE(tmessage);
	PidginAttachment *pidgin_attachment = NULL;
	PurpleAttachment *purple_attachment = NULL;

	purple_attachment = purple_message_get_attachment(pmessage->message, id);
	pidgin_attachment = pidgin_attachment_new(purple_attachment);
	g_object_unref(G_OBJECT(purple_attachment));

	return TALKATU_ATTACHMENT(pidgin_attachment);
}

static void
pidgin_message_foreach_attachment_helper(PurpleAttachment *attachment,
                                         gpointer data)
{
	PidginAttachment *pidgin_attachment = NULL;
	PidginMessageAttachmentForeachData *d = NULL;

	d = (PidginMessageAttachmentForeachData *)data;
	pidgin_attachment = pidgin_attachment_new(attachment);

	d->func(TALKATU_ATTACHMENT(pidgin_attachment), d->data);

	g_object_unref(G_OBJECT(pidgin_attachment));
}

static void
pidgin_message_foreach_attachment(TalkatuMessage *tmessage,
                                  TalkatuAttachmentForeachFunc func,
                                  gpointer data)
{
	PidginMessage *pmessage = PIDGIN_MESSAGE(tmessage);
	PidginMessageAttachmentForeachData *d = NULL;

	/* PurpleAttachmentForeachFunc and TalkatuAttachmentForeachFunc may not
	 * always have the same signature.  So to work around that, we use a helper
	 * function that has the signature of PurpleAttachmentForeachFunc but will
	 * call the TalkatuAttachmentForeachFunc while also wrapping the
	 * PurpleAttachments.
	 */

	d = g_new(PidginMessageAttachmentForeachData, 1);
	d->func = func;
	d->data = data;

	purple_message_foreach_attachment(
		pmessage->message,
		pidgin_message_foreach_attachment_helper,
		d
	);

	g_free(d);
}

static void
pidgin_message_clear_attachments(TalkatuMessage *tmessage) {
	PidginMessage *pmessage = PIDGIN_MESSAGE(tmessage);

	purple_message_clear_attachments(pmessage->message);
}

static void
pidgin_message_talkatu_message_init(TalkatuMessageInterface *iface) {
	iface->add_attachment = pidgin_message_add_attachment;
	iface->remove_attachment = pidgin_message_remove_attachment;
	iface->get_attachment = pidgin_message_get_attachment;
	iface->foreach_attachment = pidgin_message_foreach_attachment;
	iface->clear_attachments = pidgin_message_clear_attachments;
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_TYPE_EXTENDED(
	PidginMessage,
	pidgin_message,
	G_TYPE_OBJECT,
	0,
	G_IMPLEMENT_INTERFACE(TALKATU_TYPE_MESSAGE,
	                      pidgin_message_talkatu_message_init)
);

static void
pidgin_message_get_property(GObject *obj, guint param_id, GValue *value,
                            GParamSpec *pspec)
{
	PidginMessage *message = PIDGIN_MESSAGE(obj);

	switch(param_id) {
		case PROP_MESSAGE:
			g_value_set_object(value, message->message);
			break;
		case PROP_ID:
			g_value_set_string(value, purple_message_get_id(message->message));
			break;
		case PROP_CONTENT_TYPE:
			g_value_set_enum(value, TALKATU_CONTENT_TYPE_PLAIN);
			break;
		case PROP_AUTHOR:
			g_value_set_string(value,
			                   purple_message_get_author(message->message));
			break;
		case PROP_AUTHOR_NAME_COLOR:
			g_value_take_boxed(value,
			                   pidgin_message_parse_author_name_color(message));
			break;
		case PROP_CONTENTS:
			g_value_set_string(value,
			                   purple_message_get_contents(message->message));
			break;
		case PROP_TIMESTAMP:
			g_value_set_boxed(value,
			                  purple_message_get_timestamp(message->message));
			break;
		case PROP_EDITED:
			g_value_set_boolean(value, FALSE);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
pidgin_message_set_property(GObject *obj, guint param_id, const GValue *value,
                            GParamSpec *pspec)
{
	PidginMessage *message = PIDGIN_MESSAGE(obj);

	switch(param_id) {
		case PROP_MESSAGE:
			pidgin_message_set_message(message, g_value_get_object(value));
			break;
		case PROP_ID:
		case PROP_CONTENT_TYPE:
		case PROP_TIMESTAMP:
		case PROP_AUTHOR:
		case PROP_AUTHOR_NAME_COLOR:
		case PROP_CONTENTS:
		case PROP_EDITED:
			/* we don't allow settings these */
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
pidgin_message_init(G_GNUC_UNUSED PidginMessage *message) {
}

static void
pidgin_message_finalize(GObject *obj) {
	PidginMessage *message = PIDGIN_MESSAGE(obj);

	g_clear_object(&message->message);

	G_OBJECT_CLASS(pidgin_message_parent_class)->finalize(obj);
}

static void
pidgin_message_class_init(PidginMessageClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->get_property = pidgin_message_get_property;
	obj_class->set_property = pidgin_message_set_property;
	obj_class->finalize = pidgin_message_finalize;

	/* add our custom properties */
	properties[PROP_MESSAGE] = g_param_spec_object(
		"message", "message", "The purple message",
		PURPLE_TYPE_MESSAGE,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY
	);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);

	/* add our overridden properties */
	g_object_class_override_property(obj_class, PROP_ID, "id");
	g_object_class_override_property(obj_class, PROP_TIMESTAMP, "timestamp");
	g_object_class_override_property(obj_class, PROP_CONTENT_TYPE,
	                                 "content-type");
	g_object_class_override_property(obj_class, PROP_AUTHOR, "author");
	g_object_class_override_property(obj_class, PROP_AUTHOR_NAME_COLOR,
	                                 "author-name-color");
	g_object_class_override_property(obj_class, PROP_CONTENTS, "contents");
	g_object_class_override_property(obj_class, PROP_EDITED, "edited");
}

/******************************************************************************
 * API
 *****************************************************************************/
PidginMessage *
pidgin_message_new(PurpleMessage *message) {
	return g_object_new(
		PIDGIN_TYPE_MESSAGE,
		"message", message,
		NULL
	);
}

PurpleMessage *
pidgin_message_get_message(PidginMessage *message) {
	g_return_val_if_fail(PIDGIN_IS_MESSAGE(message), NULL);

	return message->message;
}
