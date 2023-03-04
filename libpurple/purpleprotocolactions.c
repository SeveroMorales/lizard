/*
 * Purple - Internet Messaging Library
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
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

#include "purpleprotocolactions.h"

enum {
	SIG_ACTIONS_CHANGED,
	N_SIGNALS,
};
static guint signals[N_SIGNALS] = {0, };

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_INTERFACE(PurpleProtocolActions, purple_protocol_actions,
                   PURPLE_TYPE_PROTOCOL)

static void
purple_protocol_actions_default_init(PurpleProtocolActionsInterface *iface) {
	/**
	 * PurpleProtocolActions::actions-changed:
	 * @self: The instance.
	 * @account: The [class@Account] whose actions changed.
	 *
	 * A signal that is emitted to tell interested parties that the actions
	 * have changed.
	 *
	 * Since: 3.0.0
	 */
	signals[SIG_ACTIONS_CHANGED] = g_signal_new_class_handler(
		"actions-changed",
		G_TYPE_FROM_INTERFACE(iface),
		G_SIGNAL_RUN_LAST,
		NULL,
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		1,
		PURPLE_TYPE_ACCOUNT);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
const gchar *
purple_protocol_actions_get_prefix(PurpleProtocolActions *actions) {
	PurpleProtocolActionsInterface *iface = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL_ACTIONS(actions), NULL);

	iface = PURPLE_PROTOCOL_ACTIONS_GET_IFACE(actions);
	if(iface != NULL) {
		if(iface->get_prefix != NULL) {
			return iface->get_prefix(actions);
		} else {
			return purple_protocol_get_id(PURPLE_PROTOCOL(actions));
		}
	}

	return NULL;
}

GActionGroup *
purple_protocol_actions_get_action_group(PurpleProtocolActions *actions,
                                         PurpleConnection *connection)
{
	PurpleProtocolActionsInterface *iface = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL_ACTIONS(actions), NULL);
	g_return_val_if_fail(PURPLE_IS_CONNECTION(connection), NULL);

	iface = PURPLE_PROTOCOL_ACTIONS_GET_IFACE(actions);
	if(iface != NULL && iface->get_action_group != NULL) {
		return iface->get_action_group(actions, connection);
	}

	return NULL;
}

GMenu *
purple_protocol_actions_get_menu(PurpleProtocolActions *actions,
                                 PurpleConnection *connection)
{
	PurpleProtocolActionsInterface *iface = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL_ACTIONS(actions), NULL);

	iface = PURPLE_PROTOCOL_ACTIONS_GET_IFACE(actions);
	if(iface != NULL && iface->get_menu != NULL) {
		return iface->get_menu(actions, connection);
	}

	return NULL;
}

void
purple_protocol_actions_changed(PurpleProtocolActions *actions,
                                PurpleAccount *account)
{
	g_return_if_fail(PURPLE_IS_PROTOCOL_ACTIONS(actions));
	g_return_if_fail(PURPLE_IS_ACCOUNT(account));

	g_signal_emit(actions, signals[SIG_ACTIONS_CHANGED], 0, account);
}
