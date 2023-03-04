/*
 * Purple - Internet Messaging Library
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
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

#if !defined(PURPLE_GLOBAL_HEADER_INSIDE) && !defined(PURPLE_COMPILATION)
# error "only <pidgin.h> may be included directly"
#endif

#ifndef PURPLE_CONVERSATION_MEMBER_H
#define PURPLE_CONVERSATION_MEMBER_H

#include <glib.h>
#include <glib-object.h>

#include <libpurple/purplecontactinfo.h>
#include <libpurple/purpletags.h>
#include <libpurple/purpletyping.h>

G_BEGIN_DECLS

#define PURPLE_TYPE_CONVERSATION_MEMBER (purple_conversation_member_get_type())
G_DECLARE_FINAL_TYPE(PurpleConversationMember, purple_conversation_member,
                     PURPLE, CONVERSATION_MEMBER, GObject)

/**
 * PurpleConversationMember:
 *
 * A conversation member links a [class@Purple.ContactInfo] to a
 * [class@Purple.Conversation] as well as any data that is unique to the link.
 *
 * Some examples of this are typing state, badges, tags, etc.
 *
 * This does not hold a reference to a [class@Purple.Conversation] as you
 * should not need to hold onto these and will have the
 * [class@Purple.Conversation] when you need to look it up.
 *
 * Since: 3.0.0
 */

/**
 * purple_conversation_member_new:
 * @info: The [class@Purple.ContactInfo] for the member.
 *
 * Creates a new [class@Purple.ConversationMember]. This does not track the
 * [class@Purple.Conversation] as you already need to know the conversation to
 * access the member.
 *
 * Returns: (transfer full): The new instance.
 *
 * Since: 3.0.0
 */
PurpleConversationMember *purple_conversation_member_new(PurpleContactInfo *info);

/**
 * purple_conversation_member_get_contact_info:
 * @conversation_member: The instance.
 *
 * Gets the [class@Purple.ContactInfo] for @conversation_member.
 *
 * Returns: (transfer none): The [class@Purple.ContactInfo] for
 *          @conversation_member.
 *
 * Since: 3.0.0
 */
PurpleContactInfo *purple_conversation_member_get_contact_info(PurpleConversationMember *conversation_member);

/**
 * purple_conversation_member_get_tags:
 * @conversation_member: The instance.
 *
 * Gets the [class@Purple.Tags] instance for @conversation_member.
 *
 * Returns: (transfer none): The [class@Purple.Tags] for @conversation_member.
 *
 * Since: 3.0.0
 */
PurpleTags *purple_conversation_member_get_tags(PurpleConversationMember *conversation_member);

/**
 * purple_conversation_member_get_typing_state:
 * @member: The instance.
 *
 * Gets the current [enum@Purple.TypingState] for @conversation_member.
 *
 * Returns: The current typing state for @conversation_member.
 *
 * Since: 3.0.0
 */
PurpleTypingState purple_conversation_member_get_typing_state(PurpleConversationMember *member);

/**
 * purple_conversation_member_set_typing_state:
 * @member: The instance.
 * @state: The new typing state.
 * @seconds: The number of seconds before resetting the state.
 *
 * Sets the typing state of @conversation_member to @state.
 *
 * If @seconds is greater than %0, a timeout will be added for @seconds to
 * reset the state to none.
 *
 * Since: 3.0.0
 */
void purple_conversation_member_set_typing_state(PurpleConversationMember *member, PurpleTypingState state, guint seconds);

G_END_DECLS

#endif /* PURPLE_CONVERSATION_MEMBER_H */
