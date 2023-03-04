/* pidgin
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
 */

#include "pidginattachment.h"

struct _PidginAttachment {
	GObject parent;

	PurpleAttachment *attachment;
};

enum {
	PROP_0,
	PROP_ATTACHMENT,
	N_PROPERTIES,
	/* overrides */
	PROP_ID = N_PROPERTIES,
	PROP_CONTENT_TYPE,
	PROP_LOCAL_URI,
	PROP_REMOTE_URI,
	PROP_SIZE,
};
static GParamSpec *properties[N_PROPERTIES] = {NULL, };

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
pidgin_attachment_set_attachment(PidginAttachment *attachment,
                                 PurpleAttachment *purple_attachment)
{
	if(g_set_object(&attachment->attachment, purple_attachment)) {
		g_object_notify_by_pspec(G_OBJECT(attachment),
		                         properties[PROP_ATTACHMENT]);
	}
}

/******************************************************************************
 * TalkatuAttachment Implementation
 *****************************************************************************/
static guint64 *
pidgin_attachment_get_hash_key(TalkatuAttachment *attachment) {
	PidginAttachment *wrapper = PIDGIN_ATTACHMENT(attachment);

	return purple_attachment_get_hash_key(wrapper->attachment);
}

static void
pidgin_attachment_talkatu_attachment_init(TalkatuAttachmentInterface *iface) {
	iface->get_hash_key = pidgin_attachment_get_hash_key;
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_TYPE_EXTENDED(
	PidginAttachment,
	pidgin_attachment,
	G_TYPE_OBJECT,
	0,
	G_IMPLEMENT_INTERFACE(
		TALKATU_TYPE_ATTACHMENT,
	    pidgin_attachment_talkatu_attachment_init
	)
);

static void
pidgin_attachment_get_property(GObject *obj, guint param_id, GValue *value, GParamSpec *pspec) {
	PidginAttachment *wrapper = PIDGIN_ATTACHMENT(obj);
	PurpleAttachment *attachment = wrapper->attachment;

	switch(param_id) {
		case PROP_ATTACHMENT:
			g_value_set_object(value, attachment);
			break;
		case PROP_ID:
			g_value_set_uint(value, purple_attachment_get_id(attachment));
			break;
		case PROP_CONTENT_TYPE:
			g_value_set_string(value,
			                   purple_attachment_get_content_type(attachment));
			break;
		case PROP_LOCAL_URI:
			g_value_set_string(value,
			                   purple_attachment_get_local_uri(attachment));
			break;
		case PROP_REMOTE_URI:
			g_value_set_string(value,
			                   purple_attachment_get_remote_uri(attachment));
			break;
		case PROP_SIZE:
			g_value_set_uint64(value,
			                   purple_attachment_get_size(attachment));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
pidgin_attachment_set_property(GObject *obj, guint param_id, const GValue *value, GParamSpec *pspec) {
	PidginAttachment *attachment = PIDGIN_ATTACHMENT(obj);

	switch(param_id) {
		case PROP_ATTACHMENT:
			pidgin_attachment_set_attachment(attachment,
			                                 g_value_get_object(value));
			break;
		case PROP_ID:
		case PROP_CONTENT_TYPE:
		case PROP_LOCAL_URI:
		case PROP_REMOTE_URI:
		case PROP_SIZE:
			/* we don't allow setting these, if you need to change them, use
			 * the underlying PurpleAttachment.
			 */
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
pidgin_attachment_init(G_GNUC_UNUSED PidginAttachment *attachment) {
}

static void
pidgin_attachment_finalize(GObject *obj) {
	PidginAttachment *attachment = PIDGIN_ATTACHMENT(obj);

	g_clear_object(&attachment->attachment);

	G_OBJECT_CLASS(pidgin_attachment_parent_class)->finalize(obj);
}

static void
pidgin_attachment_class_init(PidginAttachmentClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->get_property = pidgin_attachment_get_property;
	obj_class->set_property = pidgin_attachment_set_property;
	obj_class->finalize = pidgin_attachment_finalize;

	/* add our custom properties */
	properties[PROP_ATTACHMENT] = g_param_spec_object(
		"attachment", "attachment", "The purple attachment",
		PURPLE_TYPE_MESSAGE,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY
	);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);

	/* add our overridden properties */
	g_object_class_override_property(obj_class, PROP_ID, "id");
	g_object_class_override_property(obj_class, PROP_CONTENT_TYPE, "content-type");
	g_object_class_override_property(obj_class, PROP_LOCAL_URI, "local-uri");
	g_object_class_override_property(obj_class, PROP_REMOTE_URI, "remote-uri");
	g_object_class_override_property(obj_class, PROP_SIZE, "size");
}

/******************************************************************************
 * API
 *****************************************************************************/
PidginAttachment *
pidgin_attachment_new(PurpleAttachment *attachment) {
	return g_object_new(
		PIDGIN_TYPE_ATTACHMENT,
		"attachment", attachment,
		NULL
	);
}

PurpleAttachment *
pidgin_attachment_get_attachment(PidginAttachment *attachment) {
	g_return_val_if_fail(PIDGIN_IS_ATTACHMENT(attachment), NULL);

	return attachment->attachment;
}
