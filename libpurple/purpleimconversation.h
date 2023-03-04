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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#if !defined(PURPLE_GLOBAL_HEADER_INSIDE) && !defined(PURPLE_COMPILATION)
# error "only <purple.h> may be included directly"
#endif

#ifndef PURPLE_IM_CONVERSATION_H
#define PURPLE_IM_CONVERSATION_H

#include <glib.h>
#include <glib-object.h>

/**
 * PurpleIMTypingState:
 * @PURPLE_IM_NOT_TYPING: Not typing.
 * @PURPLE_IM_TYPING:     Currently typing.
 * @PURPLE_IM_TYPED:      Stopped typing momentarily.
 *
 * The typing state of a user.
 */
typedef enum
{
	PURPLE_IM_NOT_TYPING = 0,
	PURPLE_IM_TYPING,
	PURPLE_IM_TYPED

} PurpleIMTypingState;

#define PURPLE_TYPE_IM_CONVERSATION            (purple_im_conversation_get_type())
#define PURPLE_IM_CONVERSATION(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), PURPLE_TYPE_IM_CONVERSATION, PurpleIMConversation))
#define PURPLE_IM_CONVERSATION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), PURPLE_TYPE_IM_CONVERSATION, PurpleIMConversationClass))
#define PURPLE_IS_IM_CONVERSATION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), PURPLE_TYPE_IM_CONVERSATION))
#define PURPLE_IS_IM_CONVERSATION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), PURPLE_TYPE_IM_CONVERSATION))
#define PURPLE_IM_CONVERSATION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), PURPLE_TYPE_IM_CONVERSATION, PurpleIMConversationClass))

typedef struct _PurpleIMConversation           PurpleIMConversation;
typedef struct _PurpleIMConversationClass      PurpleIMConversationClass;

#include "account.h"
#include <purpleconversation.h>

/**
 * PurpleIMConversation:
 *
 * Structure representing an IM conversation instance.
 */

/**
 * PurpleIMConversationClass:
 *
 * Base class for all #PurpleIMConversation's
 */
struct _PurpleIMConversationClass {
	PurpleConversationClass parent_class;

	/*< private >*/
	gpointer _purple_reserved[4];
};

G_BEGIN_DECLS

/**
 * purple_im_conversation_get_type:
 *
 * Returns: The #GType for the IMConversation object.
 */
GType purple_im_conversation_get_type(void);

/**
 * purple_im_conversation_new:
 * @account: The account opening the conversation window on the purple user's
 *           end.
 * @name:    Name of the buddy.
 *
 * Creates a new IM conversation.
 *
 * Returns: The new conversation.
 */
PurpleConversation *purple_im_conversation_new(PurpleAccount *account, const gchar *name);

/**
 * purple_im_conversation_set_typing_state:
 * @im:    The IM.
 * @state: The typing state.
 *
 * Sets the IM's typing state.
 */
void purple_im_conversation_set_typing_state(PurpleIMConversation *im, PurpleIMTypingState state);

/**
 * purple_im_conversation_get_typing_state:
 * @im: The IM.
 *
 * Returns the IM's typing state.
 *
 * Returns: The IM's typing state.
 */
PurpleIMTypingState purple_im_conversation_get_typing_state(PurpleIMConversation *im);

/**
 * purple_im_conversation_start_typing_timeout:
 * @im:      The IM.
 * @timeout: How long in seconds to wait before setting the typing state to
 *           PURPLE_IM_NOT_TYPING.
 *
 * Starts the IM's typing timeout.
 */
void purple_im_conversation_start_typing_timeout(PurpleIMConversation *im, int timeout);

/**
 * purple_im_conversation_stop_typing_timeout:
 * @im: The IM.
 *
 * Stops the IM's typing timeout.
 */
void purple_im_conversation_stop_typing_timeout(PurpleIMConversation *im);

/**
 * purple_im_conversation_get_typing_timeout:
 * @im: The IM.
 *
 * Returns the IM's typing timeout.
 *
 * Returns: The timeout.
 */
guint purple_im_conversation_get_typing_timeout(PurpleIMConversation *im);

/**
 * purple_im_conversation_set_type_again:
 * @im:  The IM.
 * @val: The number of seconds to wait before allowing another #PURPLE_IM_TYPING
 *       message to be sent to the user.  Or 0 to not send another
 *       #PURPLE_IM_TYPING message.
 *
 * Sets the quiet-time when no #PURPLE_IM_TYPING messages will be sent.
 * Few protocols need this (maybe only MSN).  If the user is still
 * typing after this quiet-period, then another #PURPLE_IM_TYPING message
 * will be sent.
 */
void purple_im_conversation_set_type_again(PurpleIMConversation *im, guint val);

/**
 * purple_im_conversation_get_type_again:
 * @im: The IM.
 *
 * Returns the time after which another PURPLE_IM_TYPING message should be sent.
 *
 * Returns: The time in seconds since the epoch.  Or 0 if no additional
 *         PURPLE_IM_TYPING message should be sent.
 */
time_t purple_im_conversation_get_type_again(PurpleIMConversation *im);

/**
 * purple_im_conversation_start_send_typed_timeout:
 * @im:      The IM.
 *
 * Starts the IM's type again timeout.
 */
void purple_im_conversation_start_send_typed_timeout(PurpleIMConversation *im);

/**
 * purple_im_conversation_stop_send_typed_timeout:
 * @im: The IM.
 *
 * Stops the IM's type again timeout.
 */
void purple_im_conversation_stop_send_typed_timeout(PurpleIMConversation *im);

/**
 * purple_im_conversation_get_send_typed_timeout:
 * @im: The IM.
 *
 * Returns the IM's type again timeout interval.
 *
 * Returns: The type again timeout interval.
 */
guint purple_im_conversation_get_send_typed_timeout(PurpleIMConversation *im);

/**
 * purple_im_conversation_update_typing:
 * @im: The IM.
 *
 * Updates the visual typing notification for an IM conversation.
 */
void purple_im_conversation_update_typing(PurpleIMConversation *im);

G_END_DECLS

#endif /* PURPLE_IM_CONVERSATION_H */
