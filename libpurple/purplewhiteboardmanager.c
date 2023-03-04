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

#include <glib/gi18n-lib.h>

#include "purplewhiteboardmanager.h"
#include "purpleprivate.h"

enum {
	SIG_REGISTERED,
	SIG_UNREGISTERED,
	N_SIGNALS,
};
static guint signals[N_SIGNALS] = {0, };

struct _PurpleWhiteboardManager {
	GObject parent;

	GPtrArray *whiteboards;
};

static PurpleWhiteboardManager *default_manager = NULL;

/******************************************************************************
 * GListModel Implementation
 *****************************************************************************/
static GType
purple_whiteboard_manager_get_item_type(G_GNUC_UNUSED GListModel *list) {
	return PURPLE_TYPE_WHITEBOARD;
}

static guint
purple_whiteboard_manager_get_n_items(GListModel *list) {
	PurpleWhiteboardManager *manager = PURPLE_WHITEBOARD_MANAGER(list);

	return manager->whiteboards->len;
}

static gpointer
purple_whiteboard_manager_get_item(GListModel *list, guint position) {
	PurpleWhiteboardManager *manager = PURPLE_WHITEBOARD_MANAGER(list);
	PurpleWhiteboard *whiteboard = NULL;

	if(position < manager->whiteboards->len) {
		whiteboard = g_ptr_array_index(manager->whiteboards, position);
		g_object_ref(whiteboard);
	}

	return whiteboard;
}

static void
purple_whiteboard_manager_list_model_init(GListModelInterface *iface) {
	iface->get_item_type = purple_whiteboard_manager_get_item_type;
	iface->get_n_items = purple_whiteboard_manager_get_n_items;
	iface->get_item = purple_whiteboard_manager_get_item;
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_TYPE_EXTENDED(PurpleWhiteboardManager, purple_whiteboard_manager,
                       G_TYPE_OBJECT, G_TYPE_FLAG_FINAL,
                       G_IMPLEMENT_INTERFACE(G_TYPE_LIST_MODEL, purple_whiteboard_manager_list_model_init));

static void
purple_whiteboard_manager_finalize(GObject *obj) {
	PurpleWhiteboardManager *manager = NULL;

	manager = PURPLE_WHITEBOARD_MANAGER(obj);

	g_ptr_array_free(manager->whiteboards, TRUE);
	manager->whiteboards = NULL;

	G_OBJECT_CLASS(purple_whiteboard_manager_parent_class)->finalize(obj);
}

static void
purple_whiteboard_manager_init(PurpleWhiteboardManager *manager) {
	manager->whiteboards = g_ptr_array_new_full(0, g_object_unref);
}

static void
purple_whiteboard_manager_class_init(PurpleWhiteboardManagerClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->finalize = purple_whiteboard_manager_finalize;

	/**
	 * PurpleWhiteboardManager::registered:
	 * @manager: The #PurpleWhiteboardManager instance.
	 * @whiteboard: The #PurpleWhiteboard that was registered.
	 *
	 * Emitted after @whiteboard has been registered in @manager.
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
		PURPLE_TYPE_WHITEBOARD);

	/**
	 * PurpleWhiteboardManager::unregistered:
	 * @manager: The #PurpleWhiteboardManager instance.
	 * @whiteboard: The #PurpleWhiteboard that was unregistered.
	 *
	 * Emitted after @whiteboard has been unregistered from @manager.
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
		PURPLE_TYPE_WHITEBOARD);
}

/******************************************************************************
 * Private API
 *****************************************************************************/
void
purple_whiteboard_manager_startup(void) {
	if(default_manager == NULL) {
		default_manager = g_object_new(PURPLE_TYPE_WHITEBOARD_MANAGER, NULL);
		g_object_add_weak_pointer(G_OBJECT(default_manager),
		                          (gpointer *)&default_manager);
	}
}

void
purple_whiteboard_manager_shutdown(void) {
	g_clear_object(&default_manager);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
PurpleWhiteboardManager *
purple_whiteboard_manager_get_default(void) {
	return default_manager;
}

GListModel *
purple_whiteboard_manager_get_default_as_model(void) {
	if(PURPLE_IS_WHITEBOARD_MANAGER(default_manager)) {
		return G_LIST_MODEL(default_manager);
	}

	return NULL;
}

gboolean
purple_whiteboard_manager_register(PurpleWhiteboardManager *manager,
                                   PurpleWhiteboard *whiteboard,
                                   GError **error)
{
	gboolean found = FALSE;

	g_return_val_if_fail(PURPLE_IS_WHITEBOARD_MANAGER(manager), FALSE);
	g_return_val_if_fail(PURPLE_IS_WHITEBOARD(whiteboard), FALSE);

	found = g_ptr_array_find_with_equal_func(manager->whiteboards, whiteboard,
	                                         (GEqualFunc)purple_whiteboard_equal,
	                                         NULL);

	if(found) {
		g_set_error(error, PURPLE_WHITEBOARD_MANAGER_DOMAIN, 0,
		            _("whiteboard %s is already registered"),
		            purple_whiteboard_get_id(whiteboard));

		return FALSE;
	}

	g_ptr_array_add(manager->whiteboards, g_object_ref(whiteboard));

	g_signal_emit(manager, signals[SIG_REGISTERED], 0, whiteboard);

	return TRUE;
}

gboolean
purple_whiteboard_manager_unregister(PurpleWhiteboardManager *manager,
                                     PurpleWhiteboard *whiteboard,
                                     GError **error)
{
	guint index = 0;
	gboolean found = FALSE;

	g_return_val_if_fail(PURPLE_IS_WHITEBOARD_MANAGER(manager), FALSE);
	g_return_val_if_fail(PURPLE_IS_WHITEBOARD(whiteboard), FALSE);

	found = g_ptr_array_find_with_equal_func(manager->whiteboards, whiteboard,
	                                         (GEqualFunc)purple_whiteboard_equal,
	                                         &index);

	if(!found) {
		g_set_error(error, PURPLE_WHITEBOARD_MANAGER_DOMAIN, 0,
		            _("whiteboard %s is not registered"),
		            purple_whiteboard_get_id(whiteboard));

		return FALSE;
	}

	/* Temporarily ref whiteboard so we can pass it along to the signal
	 * callbacks.
	 */
	g_object_ref(whiteboard);

	g_ptr_array_remove_index(manager->whiteboards, index);

	g_signal_emit(manager, signals[SIG_UNREGISTERED], 0, whiteboard);

	g_object_unref(whiteboard);

	return TRUE;
}

PurpleWhiteboard *
purple_whiteboard_manager_find(PurpleWhiteboardManager *manager,
                               const gchar *id)
{
	PurpleWhiteboard *needle = NULL;
	gboolean found = FALSE;
	guint index = 0;

	g_return_val_if_fail(PURPLE_IS_WHITEBOARD_MANAGER(manager), NULL);
	g_return_val_if_fail(id != NULL, NULL);

	needle = g_object_new(PURPLE_TYPE_WHITEBOARD, "id", id, NULL);

	found = g_ptr_array_find_with_equal_func(manager->whiteboards, needle,
	                                         (GEqualFunc)purple_whiteboard_equal,
	                                         &index);

	g_clear_object(&needle);

	if(found) {
		return g_ptr_array_index(manager->whiteboards, index);
	}

	return NULL;
}
