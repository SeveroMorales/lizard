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
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <https://www.gnu.org/licenses/>.
 */

#include <glib.h>

#include <purple.h>

#include "test_ui.h"

/******************************************************************************
 * Tests
 *****************************************************************************/
static void
test_purple_whiteboard_manager_get_default(void) {
	PurpleWhiteboardManager *manager1 = NULL, *manager2 = NULL;

	manager1 = purple_whiteboard_manager_get_default();
	g_assert_true(PURPLE_IS_WHITEBOARD_MANAGER(manager1));

	manager2 = purple_whiteboard_manager_get_default();
	g_assert_true(PURPLE_IS_WHITEBOARD_MANAGER(manager2));

	g_assert_true(manager1 == manager2);
}

/******************************************************************************
 * Main
 *****************************************************************************/
gint
main(gint argc, gchar *argv[]) {
	g_test_init(&argc, &argv, NULL);

	test_ui_purple_init();

	g_test_add_func("/whiteboard-manager/get-default",
	                test_purple_whiteboard_manager_get_default);

	return g_test_run();
}
