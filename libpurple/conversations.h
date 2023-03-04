/* purple
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 */

#if !defined(PURPLE_GLOBAL_HEADER_INSIDE) && !defined(PURPLE_COMPILATION)
# error "only <purple.h> may be included directly"
#endif

#ifndef PURPLE_CONVERSATIONS_H
#define PURPLE_CONVERSATIONS_H

#include <purpleconversation.h>
#include "server.h"

G_BEGIN_DECLS

/**************************************************************************/
/* Conversations Subsystem                                                */
/**************************************************************************/

/**
 * purple_conversations_set_ui_ops:
 * @ops:  The UI conversation operations structure.
 *
 * Sets the default conversation UI operations structure.
 */
void purple_conversations_set_ui_ops(PurpleConversationUiOps *ops);

/**
 * purple_conversations_get_ui_ops:
 *
 * Gets the default conversation UI operations structure.
 *
 * Returns:  The UI conversation operations structure.
 */
PurpleConversationUiOps *purple_conversations_get_ui_ops(void);

/**
 * purple_conversations_get_handle:
 *
 * Returns the conversation subsystem handle.
 *
 * Returns: The conversation subsystem handle.
 */
void *purple_conversations_get_handle(void);

/**
 * purple_conversations_init:
 *
 * Initializes the conversation subsystem.
 */
void purple_conversations_init(void);

/**
 * purple_conversations_uninit:
 *
 * Uninitializes the conversation subsystem.
 */
void purple_conversations_uninit(void);

G_END_DECLS

#endif /* PURPLE_CONVERSATIONS_H */
