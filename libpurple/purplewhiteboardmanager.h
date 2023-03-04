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
# error "only <purple.h> may be included directly"
#endif

#ifndef PURPLE_WHITEBOARD_MANAGER_H
#define PURPLE_WHITEBOARD_MANAGER_H

#include <glib.h>
#include <glib-object.h>

#include <libpurple/purplewhiteboard.h>

G_BEGIN_DECLS

/**
 * PURPLE_WHITEBOARD_MANAGER_DOMAIN:
 *
 * A #GError domain for errors from #PurpleWhiteboardManager.
 *
 * Since: 3.0.0
 */
#define PURPLE_WHITEBOARD_MANAGER_DOMAIN (g_quark_from_static_string("purple-whiteboard-manager"))

#define PURPLE_TYPE_WHITEBOARD_MANAGER (purple_whiteboard_manager_get_type())

/**
 * PurpleWhiteboardManager:
 *
 * #PurpleWhiteboardManager keeps track of all whiteboards and emits signals
 * when whiteboards are registered and unregistered.
 *
 * Since: 3.0.0
 */
G_DECLARE_FINAL_TYPE(PurpleWhiteboardManager, purple_whiteboard_manager,
                     PURPLE, WHITEBOARD_MANAGER, GObject)

/**
 * purple_whiteboard_manager_get_default:
 *
 * Gets the default #PurpleWhiteboardManager instance.
 *
 * Returns: (transfer none): The default #PurpleWhiteboardManager instance.
 *
 * Since: 3.0.0
 */
PurpleWhiteboardManager *purple_whiteboard_manager_get_default(void);

/**
 * purple_whiteboard_manager_get_default_as_model:
 *
 * Gets the default manager as a [iface@Gio.ListModel].
 *
 * Returns: (transfer none): The default manager cast to a
 *          [iface@Gio.ListModel].
 *
 * Since: 3.0.0
 */
GListModel *purple_whiteboard_manager_get_default_as_model(void);

/**
 * purple_whiteboard_manager_register:
 * @manager: The #PurpleWhiteboardManager instance.
 * @whiteboard: The #PurpleWhiteboard to register.
 * @error: Return address for a #GError, or %NULL.
 *
 * Registers @whiteboard with @manager.
 *
 * Returns: %TRUE if @whiteboard was successfully registered with @manager,
 *          %FALSE otherwise.
 *
 * Since: 3.0.0
 */
gboolean purple_whiteboard_manager_register(PurpleWhiteboardManager *manager, PurpleWhiteboard *whiteboard, GError **error);

/**
 * purple_whiteboard_manager_unregister:
 * @manager: The #PurpleWhiteboardManager instance.
 * @whiteboard: The #PurpleWhiteboard to unregister.
 * @error: Return address for a #GError, or %NULL.
 *
 * Unregisters @whiteboard from @manager.
 *
 * Returns: %TRUE if @whiteboard was successfully unregistered from @manager,
 *          %FALSE otherwise.
 *
 * Since: 3.0.0
 */
gboolean purple_whiteboard_manager_unregister(PurpleWhiteboardManager *manager, PurpleWhiteboard *whiteboard, GError **error);

/**
 * purple_whiteboard_manager_find:
 * @manager: The #PurpleWhiteboardManager instance.
 * @id: The id of the #PurpleWhiteboard to find.
 *
 * Gets the #PurpleWhiteboard identified by @id if found, otherwise %NULL.
 *
 * Returns: (transfer none): The #PurpleWhiteboard identified by @id or %NULL.
 *
 * Since: 3.0.0
 */
PurpleWhiteboard *purple_whiteboard_manager_find(PurpleWhiteboardManager *manager, const gchar *id);

G_END_DECLS

#endif /* PURPLE_WHITEBOARD_MANAGER_H */
