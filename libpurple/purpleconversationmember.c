/*
 * Purple - Internet Messaging Library
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that `it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#include "purpleconversationmember.h"

#include "purpleenums.h"

struct _PurpleConversationMember {
	GObject parent;

	PurpleContactInfo *contact_info;
	PurpleTags *tags;

	guint typing_timeout;
	PurpleTypingState typing_state;
};

enum {
	PROP_0,
	PROP_CONTACT_INFO,
	PROP_TAGS,
	PROP_TYPING_STATE,
	N_PROPERTIES
};
static GParamSpec *properties[N_PROPERTIES] = {NULL, };

G_DEFINE_TYPE(PurpleConversationMember, purple_conversation_member,
              G_TYPE_OBJECT)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
purple_conversation_member_set_contact_info(PurpleConversationMember *member,
                                            PurpleContactInfo *contact_info)
{
	g_return_if_fail(PURPLE_IS_CONVERSATION_MEMBER(member));
	g_return_if_fail(PURPLE_IS_CONTACT_INFO(contact_info));

	if(g_set_object(&member->contact_info, contact_info)) {
		g_object_notify_by_pspec(G_OBJECT(member),
		                         properties[PROP_CONTACT_INFO]);
	}
}

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static gboolean
purple_conversation_member_reset_typing_state(gpointer data) {
	PurpleConversationMember *member = data;

	purple_conversation_member_set_typing_state(member,
	                                            PURPLE_TYPING_STATE_NONE,
	                                            0);

	return G_SOURCE_REMOVE;
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
purple_conversation_member_get_property(GObject *obj, guint param_id,
                                        GValue *value, GParamSpec *pspec)
{
	PurpleConversationMember *member = PURPLE_CONVERSATION_MEMBER(obj);

	switch(param_id) {
		case PROP_CONTACT_INFO:
			g_value_set_object(value,
			                   purple_conversation_member_get_contact_info(member));
			break;
		case PROP_TAGS:
			g_value_set_object(value,
			                   purple_conversation_member_get_tags(member));
			break;
		case PROP_TYPING_STATE:
			g_value_set_enum(value,
			                 purple_conversation_member_get_typing_state(member));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_conversation_member_set_property(GObject *obj, guint param_id,
                                        const GValue *value, GParamSpec *pspec)
{
	PurpleConversationMember *member = PURPLE_CONVERSATION_MEMBER(obj);

	switch(param_id) {
		case PROP_CONTACT_INFO:
			purple_conversation_member_set_contact_info(member,
			                                            g_value_get_object(value));
			break;
		case PROP_TYPING_STATE:
			purple_conversation_member_set_typing_state(member,
			                                            g_value_get_enum(value),
			                                            0);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_conversation_member_dispose(GObject *obj) {
	PurpleConversationMember *member = PURPLE_CONVERSATION_MEMBER(obj);

	g_clear_object(&member->contact_info);

	if(member->typing_timeout != 0) {
		g_source_remove(member->typing_timeout);
		member->typing_timeout = 0;
	}

	G_OBJECT_CLASS(purple_conversation_member_parent_class)->dispose(obj);
}

static void
purple_conversation_member_finalize(GObject *obj) {
	PurpleConversationMember *member = PURPLE_CONVERSATION_MEMBER(obj);

	g_clear_object(&member->tags);

	G_OBJECT_CLASS(purple_conversation_member_parent_class)->finalize(obj);
}

static void
purple_conversation_member_init(PurpleConversationMember *member) {
	member->tags = purple_tags_new();
}

static void
purple_conversation_member_class_init(PurpleConversationMemberClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->dispose = purple_conversation_member_dispose;
	obj_class->finalize = purple_conversation_member_finalize;
	obj_class->get_property = purple_conversation_member_get_property;
	obj_class->set_property = purple_conversation_member_set_property;

	/**
	 * PurpleConversationMember:contact-info:
	 *
	 * The contact info that this member is for.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_CONTACT_INFO] = g_param_spec_object(
		"contact-info", "contact-info",
		"The contact-info this member is for",
		PURPLE_TYPE_CONTACT_INFO,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleConversationMember:tags:
	 *
	 * The [class@Purple.Tags] instance for this member.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_TAGS] = g_param_spec_object(
		"tags", "tags",
		"The tags for this member",
		PURPLE_TYPE_TAGS,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleConversationMember:typing-state:
	 *
	 * The [enum@Purple.TypingState] for this member.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_TYPING_STATE] = g_param_spec_enum(
		"typing-state", "typing-state",
		"The typing state for this member",
		PURPLE_TYPE_TYPING_STATE,
		PURPLE_TYPING_STATE_NONE,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
PurpleConversationMember *
purple_conversation_member_new(PurpleContactInfo *info) {
	g_return_val_if_fail(PURPLE_IS_CONTACT_INFO(info), NULL);

	return g_object_new(
		PURPLE_TYPE_CONVERSATION_MEMBER,
		"contact-info", info,
		NULL);
}

PurpleContactInfo *
purple_conversation_member_get_contact_info(PurpleConversationMember *member) {
	g_return_val_if_fail(PURPLE_IS_CONVERSATION_MEMBER(member), NULL);

	return member->contact_info;
}

PurpleTags *
purple_conversation_member_get_tags(PurpleConversationMember *member) {
	g_return_val_if_fail(PURPLE_IS_CONVERSATION_MEMBER(member), NULL);

	return member->tags;
}

PurpleTypingState
purple_conversation_member_get_typing_state(PurpleConversationMember *member)
{
	g_return_val_if_fail(PURPLE_IS_CONVERSATION_MEMBER(member),
	                     PURPLE_TYPING_STATE_NONE);

	return member->typing_state;
}

void
purple_conversation_member_set_typing_state(PurpleConversationMember *member,
                                            PurpleTypingState state,
                                            guint seconds)
{
	g_return_if_fail(PURPLE_IS_CONVERSATION_MEMBER(member));

	/* Remove an existing timeout if necessary. */
	if(member->typing_timeout != 0) {
		g_source_remove(member->typing_timeout);
		member->typing_timeout = 0;
	}

	/* If the state has changed, notify. */
	if(state != member->typing_state) {
		member->typing_state = state;

		g_object_notify_by_pspec(G_OBJECT(member),
		                         properties[PROP_TYPING_STATE]);
	}

	/* If we got a timeout, add it. */
	if(seconds > 0) {
		guint source = 0;

		source = g_timeout_add_seconds(seconds,
		                               purple_conversation_member_reset_typing_state,
		                               member);

		member->typing_timeout = source;
	}
}
