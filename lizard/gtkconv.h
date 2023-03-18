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

#if !defined(PIDGIN_GLOBAL_HEADER_INSIDE) && !defined(PIDGIN_COMPILATION)
# error "only <pidgin.h> may be included directly"
#endif

#ifndef _PIDGIN_CONVERSATION_H_
#define _PIDGIN_CONVERSATION_H_

#include <gtk/gtk.h>

typedef struct _PidginConversation PidginConversation;

#define PIDGIN_CONVERSATION(conv) \
	((PidginConversation *)g_object_get_data(G_OBJECT(conv), "pidgin"))

#define PIDGIN_IS_PIDGIN_CONVERSATION(conv) \
	(purple_conversation_get_ui_ops(conv) == \
	 pidgin_conversations_get_conv_ui_ops())

#include <purple.h>

/**************************************************************************
 * Structures
 **************************************************************************/

/**
 * PidginConversation:
 *
 * A GTK conversation pane.
 */
struct _PidginConversation
{
	PurpleConversation *active_conv;
	GList *convs;

	GtkWidget *tab_cont;

	GtkAdjustment *vadjustment;
	GtkWidget *history;

	GtkWidget *editor;
	GtkWidget *entry;

	GtkWidget *infopane;
};

G_BEGIN_DECLS

/**************************************************************************
 * GTK Conversation API
 **************************************************************************/

/**
 * pidgin_conversations_get_conv_ui_ops:
 *
 * Returns the UI operations structure for GTK conversations.
 *
 * Returns: The GTK conversation operations structure.
 */
PurpleConversationUiOps *pidgin_conversations_get_conv_ui_ops(void);

/**
 * pidgin_conv_switch_active_conversation:
 * @conv: The conversation
 *
 * Sets the active conversation within a GTK-conversation.
 */
void pidgin_conv_switch_active_conversation(PurpleConversation *conv);

/**
 * pidgin_conv_attach_to_conversation:
 * @conv:  The conversation.
 *
 * Reattach Pidgin UI to a conversation.
 *
 * Returns: Whether Pidgin UI was successfully attached.
 */
gboolean pidgin_conv_attach_to_conversation(PurpleConversation *conv);

/**************************************************************************/
/* GTK Conversations Subsystem                                            */
/**************************************************************************/

/**
 * pidgin_conversations_get_handle:
 *
 * Returns the gtk conversations subsystem handle.
 *
 * Returns: The conversations subsystem handle.
 */
void *pidgin_conversations_get_handle(void);

/**
 * pidgin_conversations_init:
 *
 * Initializes the GTK conversations subsystem.
 */
void pidgin_conversations_init(void);

/**
 * pidgin_conversations_uninit:
 *
 * Uninitialized the GTK conversation subsystem.
 */
void pidgin_conversations_uninit(void);

void pidgin_conversation_detach(PurpleConversation *conv);

G_END_DECLS

#endif /* _PIDGIN_CONVERSATION_H_ */
