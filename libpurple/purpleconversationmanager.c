/*
 * Purple - Internet Messaging Library
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <https://www.gnu.org/licenses/>.
 */

#include <purpleconversationmanager.h>

#include <purplechatconversation.h>
#include <purpleimconversation.h>
#include <purpleprivate.h>

enum {
	SIG_REGISTERED,
	SIG_UNREGISTERED,
	N_SIGNALS,
};
static guint signals[N_SIGNALS] = {0, };

struct _PurpleConversationManager {
	GObject parent;

	GHashTable *conversations;
};

static PurpleConversationManager *default_manager = NULL;

G_DEFINE_TYPE(PurpleConversationManager, purple_conversation_manager,
              G_TYPE_OBJECT)

typedef gboolean (*PurpleConversationManagerCompareFunc)(PurpleConversation *conversation, gpointer userdata);

/******************************************************************************
 * Helpers
 *****************************************************************************/
static gboolean
purple_conversation_is_im(PurpleConversation *conversation,
                          G_GNUC_UNUSED gpointer userdata)
{
	return PURPLE_IS_IM_CONVERSATION(conversation);
}

static gboolean
purple_conversation_is_chat(PurpleConversation *conversation,
                            G_GNUC_UNUSED gpointer userdata)
{
	return PURPLE_IS_CHAT_CONVERSATION(conversation);
}

static gboolean
purple_conversation_chat_has_id(PurpleConversation *conversation,
                                gpointer userdata)
{
	PurpleChatConversation *chat = NULL;
	gint id = GPOINTER_TO_INT(userdata);


	if(!PURPLE_IS_CHAT_CONVERSATION(conversation)) {
		return FALSE;
	}

	chat = PURPLE_CHAT_CONVERSATION(conversation);

	return (purple_chat_conversation_get_id(chat) == id);
}

static PurpleConversation *
purple_conversation_manager_find_internal(PurpleConversationManager *manager,
                                          PurpleAccount *account,
                                          const gchar *name,
                                          PurpleConversationManagerCompareFunc func,
                                          gpointer userdata)
{
	GHashTableIter iter;
	gpointer key;

	g_return_val_if_fail(PURPLE_IS_ACCOUNT(account), NULL);

	g_hash_table_iter_init(&iter, manager->conversations);
	while(g_hash_table_iter_next(&iter, &key, NULL)) {
		PurpleConversation *conversation = PURPLE_CONVERSATION(key);

		if(name != NULL) {
			const gchar *conv_name = purple_conversation_get_name(conversation);

			if(!purple_strequal(conv_name, name)) {
				continue;
			}
		}

		if(purple_conversation_get_account(conversation) != account) {
			continue;
		}

		if(func != NULL && !func(conversation, userdata)) {
			continue;
		}

		return conversation;
	}

	return NULL;
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
purple_conversation_manager_init(PurpleConversationManager *manager) {
	manager->conversations = g_hash_table_new_full(g_direct_hash,
	                                               g_direct_equal,
	                                               g_object_unref, NULL);
}

static void
purple_conversation_manager_finalize(GObject *obj) {
	PurpleConversationManager *manager = PURPLE_CONVERSATION_MANAGER(obj);

	g_hash_table_destroy(manager->conversations);

	G_OBJECT_CLASS(purple_conversation_manager_parent_class)->finalize(obj);
}

static void
purple_conversation_manager_class_init(PurpleConversationManagerClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->finalize = purple_conversation_manager_finalize;

	/**
	 * PurpleConversationManager::registered:
	 * @manager: The manager.
	 * @conversation: The conversation that was registered.
	 *
	 * Emitted after @conversation has been registered with @manager.
	 *
	 * Since: 3.0.0
	 */
	signals[SIG_REGISTERED] = g_signal_new_class_handler(
		"registered",
		G_OBJECT_CLASS_TYPE(klass),
		G_SIGNAL_RUN_LAST,
		NULL,
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		1,
		PURPLE_TYPE_CONVERSATION);

	/**
	 * PurpleConversationManager::unregistered:
	 * @manager: The manager.
	 * @conversation: The conversation that was unregistered.
	 *
	 * Emitted after @conversation has been unregistered from @manager.
	 *
	 * Since: 3.0.0
	 */
	signals[SIG_UNREGISTERED] = g_signal_new_class_handler(
		"unregistered",
		G_OBJECT_CLASS_TYPE(klass),
		G_SIGNAL_RUN_LAST,
		NULL,
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		1,
		PURPLE_TYPE_CONVERSATION);
}

/******************************************************************************
 * Private API
 *****************************************************************************/
void
purple_conversation_manager_startup(void) {
	if(default_manager == NULL) {
		default_manager = g_object_new(PURPLE_TYPE_CONVERSATION_MANAGER, NULL);
	}
}

void
purple_conversation_manager_shutdown(void) {
	g_clear_object(&default_manager);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
PurpleConversationManager *
purple_conversation_manager_get_default(void) {
	return default_manager;
}

gboolean
purple_conversation_manager_register(PurpleConversationManager *manager,
                                     PurpleConversation *conversation)
{
	gboolean registered = FALSE;

	g_return_val_if_fail(PURPLE_IS_CONVERSATION_MANAGER(manager), FALSE);
	g_return_val_if_fail(PURPLE_IS_CONVERSATION(conversation), FALSE);

	/* g_hash_table_add calls the key_destroy_func if the key already exists
	 * which means we don't need to worry about the reference we're creating
	 * during the addition.
	 */
	registered = g_hash_table_add(manager->conversations,
	                              g_object_ref(conversation));

	if(registered) {
		g_signal_emit(manager, signals[SIG_REGISTERED], 0, conversation);
	}

	return registered;
}

gboolean
purple_conversation_manager_unregister(PurpleConversationManager *manager,
                                       PurpleConversation *conversation)
{
	gboolean unregistered = FALSE;

	g_return_val_if_fail(PURPLE_IS_CONVERSATION_MANAGER(manager), FALSE);
	g_return_val_if_fail(PURPLE_IS_CONVERSATION(conversation), FALSE);

	unregistered = g_hash_table_remove(manager->conversations, conversation);
	if(unregistered) {
		g_signal_emit(manager, signals[SIG_UNREGISTERED], 0, conversation);
	}

	return unregistered;
}

gboolean
purple_conversation_manager_is_registered(PurpleConversationManager *manager,
                                          PurpleConversation *conversation)
{
	g_return_val_if_fail(PURPLE_IS_CONVERSATION_MANAGER(manager), FALSE);
	g_return_val_if_fail(PURPLE_IS_CONVERSATION(conversation), FALSE);

	return g_hash_table_contains(manager->conversations, conversation);
}

void
purple_conversation_manager_foreach(PurpleConversationManager *manager,
                                    PurpleConversationManagerForeachFunc func,
                                    gpointer data)
{
	GHashTableIter iter;
	gpointer key;

	g_return_if_fail(PURPLE_IS_CONVERSATION_MANAGER(manager));
	g_return_if_fail(func != NULL);

	g_hash_table_iter_init(&iter, manager->conversations);
	while(g_hash_table_iter_next(&iter, &key, NULL)) {
		func(PURPLE_CONVERSATION(key), data);
	}
}

GList *
purple_conversation_manager_get_all(PurpleConversationManager *manager) {
	g_return_val_if_fail(PURPLE_IS_CONVERSATION_MANAGER(manager), NULL);

	return g_hash_table_get_keys(manager->conversations);
}


PurpleConversation *
purple_conversation_manager_find(PurpleConversationManager *manager,
                                 PurpleAccount *account, const gchar *name)
{
	g_return_val_if_fail(PURPLE_IS_CONVERSATION_MANAGER(manager), NULL);
	g_return_val_if_fail(PURPLE_IS_ACCOUNT(account), NULL);
	g_return_val_if_fail(name != NULL, NULL);

	return purple_conversation_manager_find_internal(manager, account, name,
	                                                 NULL, NULL);
}

PurpleConversation *
purple_conversation_manager_find_im(PurpleConversationManager *manager,
                                    PurpleAccount *account, const gchar *name)
{
	g_return_val_if_fail(PURPLE_IS_CONVERSATION_MANAGER(manager), NULL);
	g_return_val_if_fail(PURPLE_IS_ACCOUNT(account), NULL);
	g_return_val_if_fail(name != NULL, NULL);

	return purple_conversation_manager_find_internal(manager, account, name,
	                                                 purple_conversation_is_im,
	                                                 NULL);
}

PurpleConversation *
purple_conversation_manager_find_chat(PurpleConversationManager *manager,
                                      PurpleAccount *account,
                                      const gchar *name)
{
	g_return_val_if_fail(PURPLE_IS_CONVERSATION_MANAGER(manager), NULL);
	g_return_val_if_fail(PURPLE_IS_ACCOUNT(account), NULL);
	g_return_val_if_fail(name != NULL, NULL);

	return purple_conversation_manager_find_internal(manager, account, name,
	                                                 purple_conversation_is_chat,
	                                                 NULL);
}

PurpleConversation *
purple_conversation_manager_find_chat_by_id(PurpleConversationManager *manager,
                                            PurpleAccount *account, gint id)
{
	g_return_val_if_fail(PURPLE_IS_CONVERSATION_MANAGER(manager), NULL);
	g_return_val_if_fail(PURPLE_IS_ACCOUNT(account), NULL);

	return purple_conversation_manager_find_internal(manager, account, NULL,
	                                                 purple_conversation_chat_has_id,
	                                                 GINT_TO_POINTER(id));
}
