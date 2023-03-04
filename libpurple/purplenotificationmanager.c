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

#include "purplenotificationmanager.h"

#include "purpleprivate.h"

enum {
	PROP_ZERO,
	PROP_UNREAD_COUNT,
	N_PROPERTIES,
};
static GParamSpec *properties[N_PROPERTIES] = { NULL, };

enum {
	SIG_ADDED,
	SIG_REMOVED,
	SIG_READ,
	SIG_UNREAD,
	N_SIGNALS,
};
static guint signals[N_SIGNALS] = { 0, };

struct _PurpleNotificationManager {
	GObject parent;

	GPtrArray *notifications;

	guint unread_count;
};

static PurpleNotificationManager *default_manager = NULL;

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
purple_notification_manager_set_unread_count(PurpleNotificationManager *manager,
                                             guint unread_count)
{
	if(manager->unread_count != unread_count) {
		manager->unread_count = unread_count;

		g_object_notify_by_pspec(G_OBJECT(manager),
		                         properties[PROP_UNREAD_COUNT]);
	}
}

static inline void
purple_notification_manager_increment_unread_count(PurpleNotificationManager *manager)
{
	if(manager->unread_count < G_MAXUINT) {
		purple_notification_manager_set_unread_count(manager,
		                                             manager->unread_count + 1);
	}
}

static inline void
purple_notification_manager_decrement_unread_count(PurpleNotificationManager *manager)
{
	if(manager->unread_count > 0) {
		purple_notification_manager_set_unread_count(manager,
		                                             manager->unread_count - 1);
	}
}

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static void
purple_notification_manager_notify_cb(GObject *obj,
                                      G_GNUC_UNUSED GParamSpec *pspec,
                                      gpointer data)
{
	PurpleNotification *notification = PURPLE_NOTIFICATION(obj);
	PurpleNotificationManager *manager = data;
	guint signal_id = 0;

	/* This function is called after the property is changed. So we need to
	 * get the new value to determine how the state changed.
	 */

	if(purple_notification_get_read(notification)) {
		purple_notification_manager_decrement_unread_count(manager);

		signal_id = signals[SIG_READ];
	} else {
		purple_notification_manager_increment_unread_count(manager);

		signal_id = signals[SIG_UNREAD];
	}

	g_signal_emit(manager, signal_id, 0, notification);
}

/******************************************************************************
 * GListModel Implementation
 *****************************************************************************/
static GType
purple_notification_manager_get_item_type(G_GNUC_UNUSED GListModel *list) {
	return PURPLE_TYPE_NOTIFICATION;
}

static guint
purple_notification_manager_get_n_items(GListModel *list) {
	PurpleNotificationManager *manager = PURPLE_NOTIFICATION_MANAGER(list);

	return manager->notifications->len;
}

static gpointer
purple_notification_manager_get_item(GListModel *list, guint position) {
	PurpleNotificationManager *manager = PURPLE_NOTIFICATION_MANAGER(list);
	PurpleNotification *notification = NULL;

	if(position < manager->notifications->len) {
		notification = g_ptr_array_index(manager->notifications, position);
		g_object_ref(notification);
	}

	return notification;
}

static void
purple_notification_manager_list_model_init(GListModelInterface *iface) {
	iface->get_item_type = purple_notification_manager_get_item_type;
	iface->get_n_items = purple_notification_manager_get_n_items;
	iface->get_item = purple_notification_manager_get_item;
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_TYPE_EXTENDED(PurpleNotificationManager, purple_notification_manager,
                       G_TYPE_OBJECT, G_TYPE_FLAG_FINAL,
                       G_IMPLEMENT_INTERFACE(G_TYPE_LIST_MODEL, purple_notification_manager_list_model_init));


static void
purple_notification_manager_get_property(GObject *obj, guint param_id,
                                         GValue *value, GParamSpec *pspec)
{
	PurpleNotificationManager *manager = PURPLE_NOTIFICATION_MANAGER(obj);

	switch(param_id) {
		case PROP_UNREAD_COUNT:
			g_value_set_uint(value,
			                 purple_notification_manager_get_unread_count(manager));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_notification_manager_finalize(GObject *obj) {
	PurpleNotificationManager *manager = NULL;

	manager = PURPLE_NOTIFICATION_MANAGER(obj);

	g_ptr_array_free(manager->notifications, TRUE);
	manager->notifications = NULL;

	G_OBJECT_CLASS(purple_notification_manager_parent_class)->finalize(obj);
}

static void
purple_notification_manager_init(PurpleNotificationManager *manager) {
	manager->notifications = g_ptr_array_new_full(0,
	                                              (GDestroyNotify)g_object_unref);
}

static void
purple_notification_manager_class_init(PurpleNotificationManagerClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->get_property = purple_notification_manager_get_property;
	obj_class->finalize = purple_notification_manager_finalize;

	/* Properties */

	/**
	 * PurpleNotificationManager:unread-count:
	 *
	 * The number of unread notifications in the manager.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_UNREAD_COUNT] = g_param_spec_uint(
		"unread-count", "unread-count",
		"The number of unread messages in the manager.",
		0, G_MAXUINT, 0,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);

	/* Signals */

	/**
	 * PurpleNotificationManager::added:
	 * @manager: The instance.
	 * @notification: The [class@Notification] that was added.
	 *
	 * Emitted after @notification has been added to @manager.
	 *
	 * Since: 3.0.0
	 */
	signals[SIG_ADDED] = g_signal_new_class_handler(
		"added",
		G_OBJECT_CLASS_TYPE(klass),
		G_SIGNAL_RUN_LAST,
		NULL,
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		1,
		PURPLE_TYPE_NOTIFICATION);

	/**
	 * PurpleNotificationManager::removed:
	 * @manager: The instance.
	 * @notification: The [class@Notification] that was removed.
	 *
	 * Emitted after @notification has been removed from @manager.
	 *
	 * Since: 3.0.0
	 */
	signals[SIG_REMOVED] = g_signal_new_class_handler(
		"removed",
		G_OBJECT_CLASS_TYPE(klass),
		G_SIGNAL_RUN_LAST,
		NULL,
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		1,
		PURPLE_TYPE_NOTIFICATION);

	/**
	 * PurpleNotificationManager::read:
	 * @manager: The instance.
	 * @notification: The [class@Notification].
	 *
	 * Emitted after @notification has been marked as read.
	 *
	 * Since: 3.0.0
	 */
	signals[SIG_READ] = g_signal_new_class_handler(
		"read",
		G_OBJECT_CLASS_TYPE(klass),
		G_SIGNAL_RUN_LAST,
		NULL,
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		1,
		PURPLE_TYPE_NOTIFICATION);

	/**
	 * PurpleNotificationManager::unread:
	 * @manager: The instance.
	 * @notification: The [class@Notification].
	 *
	 * Emitted after @notification has been marked as unread.
	 *
	 * Since: 3.0.0
	 */
	signals[SIG_UNREAD] = g_signal_new_class_handler(
		"unread",
		G_OBJECT_CLASS_TYPE(klass),
		G_SIGNAL_RUN_LAST,
		NULL,
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		1,
		PURPLE_TYPE_NOTIFICATION);
}

/******************************************************************************
 * Private API
 *****************************************************************************/
void
purple_notification_manager_startup(void) {
	if(default_manager == NULL) {
		default_manager = g_object_new(PURPLE_TYPE_NOTIFICATION_MANAGER, NULL);
		g_object_add_weak_pointer(G_OBJECT(default_manager),
		                          (gpointer *)&default_manager);
	}
}

void
purple_notification_manager_shutdown(void) {
	g_clear_object(&default_manager);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
PurpleNotificationManager *
purple_notification_manager_get_default(void) {
	return default_manager;
}

GListModel *
purple_notification_manager_get_default_as_model(void) {
	if(PURPLE_IS_NOTIFICATION_MANAGER(default_manager)) {
		return G_LIST_MODEL(default_manager);
	}

	return NULL;
}

void
purple_notification_manager_add(PurpleNotificationManager *manager,
                                PurpleNotification *notification)
{
	g_return_if_fail(PURPLE_IS_NOTIFICATION_MANAGER(manager));
	g_return_if_fail(PURPLE_IS_NOTIFICATION(notification));

	if(g_ptr_array_find(manager->notifications, notification, NULL)) {
		const gchar *id = purple_notification_get_id(notification);

		g_warning("double add detected for notification %s", id);

		return;
	}

	g_ptr_array_insert(manager->notifications, 0, g_object_ref(notification));

	/* Connect to the notify signal for the read property only so we can
	 * propagate out changes for any notification.
	 */
	g_signal_connect_object(notification, "notify::read",
	                        G_CALLBACK(purple_notification_manager_notify_cb),
	                        manager, 0);

	/* If the notification is not read, we need to increment the unread count.
	 */
	if(!purple_notification_get_read(notification)) {
		purple_notification_manager_increment_unread_count(manager);
	}

	g_signal_emit(G_OBJECT(manager), signals[SIG_ADDED], 0, notification);
	g_list_model_items_changed(G_LIST_MODEL(manager), 0, 0, 1);
}

void
purple_notification_manager_remove(PurpleNotificationManager *manager,
                                   PurpleNotification *notification)
{
	guint index = 0;

	g_return_if_fail(PURPLE_IS_NOTIFICATION_MANAGER(manager));
	g_return_if_fail(PURPLE_IS_NOTIFICATION(notification));

	if(g_ptr_array_find(manager->notifications, notification, &index)) {
		/* Reference the notification so we can emit the signal after it's been
		 * removed from the hash table.
		 */
		g_object_ref(notification);

		g_ptr_array_remove_index(manager->notifications, index);
		/* Remove the notify signal handler for the read state incase someone
		 * else added a reference to the notification which would then mess
		 * with our unread count accounting.
		 */
		g_signal_handlers_disconnect_by_func(notification,
		                                     G_CALLBACK(purple_notification_manager_notify_cb),
		                                     manager);

		/* If the notification is not read, we need to decrement the unread
		 * count.
		 */
		if(!purple_notification_get_read(notification)) {
			purple_notification_manager_decrement_unread_count(manager);
		}

		g_signal_emit(G_OBJECT(manager), signals[SIG_REMOVED], 0, notification);
		g_list_model_items_changed(G_LIST_MODEL(manager), index, 1, 0);

		g_object_unref(notification);
	}
}

/*
This function uses the following algorithm to optimally remove items from the
g_ptr_array to minimize the number of calls to g_list_model_items_changed. See
the pseudo code below for an easier to follow version.

A
A B C
B C
A A B C
B A C
B A A C
B C A
B C A A

set len = number_of_items
set pos = 0
set have_same = false
while pos < len
  check item at pos
    if same
      if not have_same
        reset count = 0
        set start = pos
        set have_same = TRUE

      set count = count + 1
    else
      if have_same
        remove count items from start
        set pos = pos - count
        set len = len - count
        set have_same = FALSE
  set pos = pos + 1
if have_same
  remove count items from start
*/
void
purple_notification_manager_remove_with_account(PurpleNotificationManager *manager,
                                                PurpleAccount *account,
                                                gboolean all)
{
	GListModel *model = NULL;
	guint pos = 0, len = 0;
	guint start = 0, count = 0;
	gboolean have_same = FALSE;

	g_return_if_fail(PURPLE_IS_NOTIFICATION_MANAGER(manager));
	g_return_if_fail(PURPLE_IS_ACCOUNT(account));

	model = G_LIST_MODEL(manager);

	for(pos = 0; pos < manager->notifications->len; pos++) {
		PurpleAccount *account2 = NULL;
		PurpleNotification *notification = NULL;
		PurpleNotificationType type;
		gboolean can_remove = TRUE;

		notification = g_ptr_array_index(manager->notifications, pos);

		/* If the notification's type is connection error, set can_remove to
		 * the value of the all parameter.
		 */
		type = purple_notification_get_notification_type(notification);
		if(type == PURPLE_NOTIFICATION_TYPE_CONNECTION_ERROR) {
			can_remove = all;
		}

		account2 = purple_notification_get_account(notification);
		if(account == account2 && can_remove) {
			/* If this is the first item with the right account store its
			 * position.
			 */
			if(!have_same) {
				count = 0;
				start = pos;
				have_same = TRUE;
			}

			/* Increment the count of items starting at the start position. */
			count++;
		} else {
			if(have_same) {
				/* Remove the run of items from the list. */
				g_ptr_array_remove_range(manager->notifications, start, count);
				g_list_model_items_changed(model, start, count, 0);

				/* Adjust pos and len for the items that we removed. */
				pos = pos - count;
				len = len - count;

				have_same = FALSE;
			}
		}
	}

	/* Clean up the last bit if the last item needs to be removed. */
	if(have_same) {
		g_ptr_array_remove_range(manager->notifications, start, count);
		g_list_model_items_changed(model, start, count, 0);
	}
}

guint
purple_notification_manager_get_unread_count(PurpleNotificationManager *manager) {
	g_return_val_if_fail(PURPLE_IS_NOTIFICATION_MANAGER(manager), 0);

	return manager->unread_count;
}

void
purple_notification_manager_clear(PurpleNotificationManager *manager) {
	guint count = 0;

	g_return_if_fail(PURPLE_IS_NOTIFICATION_MANAGER(manager));

	count = manager->notifications->len;

	for(guint pos = 0; pos < count; pos++) {
		PurpleNotification *notification = NULL;

		notification = g_ptr_array_index(manager->notifications, pos);

		g_signal_emit(manager, signals[SIG_REMOVED], 0, notification);
	}

	g_ptr_array_remove_range(manager->notifications, 0, count);

	g_list_model_items_changed(G_LIST_MODEL(manager), 0, count, 0);
}
