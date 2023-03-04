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

#include <libpurple/purpleimconversation.h>

#include <libpurple/debug.h>
#include <libpurple/conversations.h>
#include <libpurple/purpleconversationmanager.h>
#include <libpurple/purpleenums.h>
#include <libpurple/purpleprivate.h>
#include <libpurple/purpleprotocolclient.h>

struct _PurpleIMConversation {
	PurpleConversation parent;

	PurpleIMTypingState typing_state;
	guint  typing_timeout;
	time_t type_again;
	guint  send_typed_timeout;
};

enum {
	PROP_0 = 0,
	PROP_TYPING_STATE,
	N_PROPERTIES,
};
static GParamSpec *properties[N_PROPERTIES];

#define SEND_TYPED_TIMEOUT_SECONDS 5

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static gboolean
purple_im_conversation_reset_typing_cb(gpointer data)
{
	PurpleIMConversation *im = PURPLE_IM_CONVERSATION(data);

	purple_im_conversation_set_typing_state(im, PURPLE_IM_NOT_TYPING);
	purple_im_conversation_stop_typing_timeout(im);

	return FALSE;
}

static gboolean
purple_im_conversation_send_typed_cb(gpointer data)
{
	PurpleIMConversation *im = PURPLE_IM_CONVERSATION(data);
	PurpleConnection *pc = NULL;
	const gchar *name;

	g_return_val_if_fail(im != NULL, FALSE);

	pc = purple_conversation_get_connection(PURPLE_CONVERSATION(im));
	name = purple_conversation_get_name(PURPLE_CONVERSATION(im));

	if(pc != NULL && name != NULL) {
		/* We set this to 1 so that PURPLE_IM_TYPING will be sent if the Purple
		 * user types anything else.
		 */
		purple_im_conversation_set_type_again(im, 1);

		purple_serv_send_typing(pc, name, PURPLE_IM_TYPED);

		purple_debug_misc("purple-im-conversation", "typed...");
	}

	return FALSE;
}

/******************************************************************************
 * PurpleConversation Implementation
 *****************************************************************************/
static void
im_conversation_write_message(PurpleConversation *conv, PurpleMessage *msg) {
	PurpleIMConversation *im = PURPLE_IM_CONVERSATION(conv);
	gboolean is_recv = FALSE;

	g_return_if_fail(im != NULL);
	g_return_if_fail(msg != NULL);

	is_recv = (purple_message_get_flags(msg) & PURPLE_MESSAGE_RECV);

	if(is_recv) {
		purple_im_conversation_set_typing_state(im, PURPLE_IM_NOT_TYPING);
	}

	_purple_conversation_write_common(conv, msg);
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_TYPE(PurpleIMConversation, purple_im_conversation,
              PURPLE_TYPE_CONVERSATION);

static void
purple_im_conversation_get_property(GObject *obj, guint param_id, GValue *value,
                                    GParamSpec *pspec)
{
	PurpleIMConversation *im = PURPLE_IM_CONVERSATION(obj);

	switch (param_id) {
		case PROP_TYPING_STATE:
			g_value_set_enum(value,
			                 purple_im_conversation_get_typing_state(im));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_im_conversation_set_property(GObject *obj, guint param_id,
                                    const GValue *value, GParamSpec *pspec)
{
	PurpleIMConversation *im = PURPLE_IM_CONVERSATION(obj);

	switch (param_id) {
		case PROP_TYPING_STATE:
			purple_im_conversation_set_typing_state(im,
			                                        g_value_get_enum(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_im_conversation_init(G_GNUC_UNUSED PurpleIMConversation *im) {
}

static void
purple_im_conversation_finalize(GObject *obj) {
	PurpleIMConversation *im = PURPLE_IM_CONVERSATION(obj);
	PurpleConnection *pc = purple_conversation_get_connection(PURPLE_CONVERSATION(im));
	PurpleProtocol *protocol = NULL;
	const gchar *name = purple_conversation_get_name(PURPLE_CONVERSATION(im));

	if(pc != NULL) {
		/* Still connected */
		protocol = purple_connection_get_protocol(pc);

		if(purple_prefs_get_bool("/purple/conversations/im/send_typing")) {
			purple_serv_send_typing(pc, name, PURPLE_IM_NOT_TYPING);
		}

		if(PURPLE_IS_PROTOCOL_CLIENT(protocol)) {
			purple_protocol_client_convo_closed(PURPLE_PROTOCOL_CLIENT(protocol),
			                                    pc, name);
		}
	}

	purple_im_conversation_stop_typing_timeout(im);
	purple_im_conversation_stop_send_typed_timeout(im);

	G_OBJECT_CLASS(purple_im_conversation_parent_class)->finalize(obj);
}

static void
purple_im_conversation_class_init(PurpleIMConversationClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	PurpleConversationClass *conv_class = PURPLE_CONVERSATION_CLASS(klass);

	obj_class->get_property = purple_im_conversation_get_property;
	obj_class->set_property = purple_im_conversation_set_property;
	obj_class->finalize = purple_im_conversation_finalize;

	conv_class->write_message = im_conversation_write_message;

	properties[PROP_TYPING_STATE] = g_param_spec_enum(
		"typing-state", "Typing state",
		"Status of the user's typing of a message.",
		PURPLE_TYPE_IM_TYPING_STATE, PURPLE_IM_NOT_TYPING,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
PurpleConversation *
purple_im_conversation_new(PurpleAccount *account, const char *name)
{
	PurpleConversation *im = NULL;
	PurpleConversationManager *manager = NULL;

	g_return_val_if_fail(PURPLE_IS_ACCOUNT(account), NULL);
	g_return_val_if_fail(name != NULL, NULL);

	/* Check if this conversation already exists. */
	manager = purple_conversation_manager_get_default();
	im = purple_conversation_manager_find_im(manager, account, name);
	if(PURPLE_IS_IM_CONVERSATION(im)) {
		return g_object_ref(im);
	}

	im = g_object_new(PURPLE_TYPE_IM_CONVERSATION,
		"account", account,
		"name",    name,
		"title",   name,
		NULL);

	return im;
}

void
purple_im_conversation_set_typing_state(PurpleIMConversation *im,
                                        PurpleIMTypingState state)
{
	PurpleAccount *account = NULL;
	const gchar *name = NULL;

	g_return_if_fail(PURPLE_IS_IM_CONVERSATION(im));

	name = purple_conversation_get_name(PURPLE_CONVERSATION(im));
	account = purple_conversation_get_account(PURPLE_CONVERSATION(im));

	if(im->typing_state != state) {
		im->typing_state = state;

		g_object_notify_by_pspec(G_OBJECT(im), properties[PROP_TYPING_STATE]);

		switch (state) {
			case PURPLE_IM_TYPING:
				purple_signal_emit(purple_conversations_get_handle(),
								   "buddy-typing", account, name);
				break;
			case PURPLE_IM_TYPED:
				purple_signal_emit(purple_conversations_get_handle(),
								   "buddy-typed", account, name);
				break;
			case PURPLE_IM_NOT_TYPING:
				purple_signal_emit(purple_conversations_get_handle(),
								   "buddy-typing-stopped", account, name);
				break;
		}

		purple_im_conversation_update_typing(im);
	}
}

PurpleIMTypingState
purple_im_conversation_get_typing_state(PurpleIMConversation *im) {
	g_return_val_if_fail(PURPLE_IS_IM_CONVERSATION(im), 0);

	return im->typing_state;
}

void
purple_im_conversation_start_typing_timeout(PurpleIMConversation *im,
                                            gint timeout)
{
	g_return_if_fail(PURPLE_IS_IM_CONVERSATION(im));

	if(im->typing_timeout > 0) {
		purple_im_conversation_stop_typing_timeout(im);
	}

	im->typing_timeout =
		g_timeout_add_seconds(timeout, purple_im_conversation_reset_typing_cb,
		                      im);
}

void
purple_im_conversation_stop_typing_timeout(PurpleIMConversation *im) {
	g_return_if_fail(PURPLE_IS_IM_CONVERSATION(im));

	if(im->typing_timeout == 0) {
		return;
	}

	g_source_remove(im->typing_timeout);
	im->typing_timeout = 0;
}

guint
purple_im_conversation_get_typing_timeout(PurpleIMConversation *im) {
	g_return_val_if_fail(PURPLE_IS_IM_CONVERSATION(im), 0);

	return im->typing_timeout;
}

void
purple_im_conversation_set_type_again(PurpleIMConversation *im, guint val) {
	g_return_if_fail(PURPLE_IS_IM_CONVERSATION(im));

	if(val == 0) {
		im->type_again = 0;
	} else {
		im->type_again = time(NULL) + val;
	}
}

time_t
purple_im_conversation_get_type_again(PurpleIMConversation *im) {
	g_return_val_if_fail(PURPLE_IS_IM_CONVERSATION(im), 0);

	return im->type_again;
}

void
purple_im_conversation_start_send_typed_timeout(PurpleIMConversation *im) {
	g_return_if_fail(PURPLE_IS_IM_CONVERSATION(im));

	purple_im_conversation_stop_send_typed_timeout(im);
	im->send_typed_timeout =
		g_timeout_add_seconds(SEND_TYPED_TIMEOUT_SECONDS,
		                      purple_im_conversation_send_typed_cb, im);
}

void
purple_im_conversation_stop_send_typed_timeout(PurpleIMConversation *im) {
	g_return_if_fail(PURPLE_IS_IM_CONVERSATION(im));

	if(im->send_typed_timeout == 0) {
		return;
	}

	g_source_remove(im->send_typed_timeout);
	im->send_typed_timeout = 0;
}

guint
purple_im_conversation_get_send_typed_timeout(PurpleIMConversation *im) {
	g_return_val_if_fail(PURPLE_IS_IM_CONVERSATION(im), 0);

	return im->send_typed_timeout;
}

void
purple_im_conversation_update_typing(PurpleIMConversation *im) {
	g_return_if_fail(PURPLE_IS_IM_CONVERSATION(im));

	purple_conversation_update(PURPLE_CONVERSATION(im),
	                           PURPLE_CONVERSATION_UPDATE_TYPING);
}
