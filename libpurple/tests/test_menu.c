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

typedef struct {
	const gchar *dynamic_target;
	const gchar *target;
} TestPurpleMenuItemData;

typedef struct {
	GList *items;
} TestPurpleMenuWalkData;

/******************************************************************************
 * Helpers
 *****************************************************************************/
static TestPurpleMenuItemData *
test_purple_menu_item_new(const gchar *dynamic_target, const gchar *target) {
	TestPurpleMenuItemData *data = g_new(TestPurpleMenuItemData, 1);

	data->dynamic_target = dynamic_target;
	data->target = target;

	return data;
}

static void
test_purple_menu_items_func(GMenuModel *model, gint index, gpointer data) {
	TestPurpleMenuItemData *item = NULL;
	TestPurpleMenuWalkData *walk_data = data;
	gchar *actual = NULL;
	gboolean found = FALSE;

	/* Set item to the first item in the list. */
	item = walk_data->items->data;

	/* Check that the dynamic-target value matches our expectations. */
	g_menu_model_get_item_attribute(model, index,
	                                PURPLE_MENU_ATTRIBUTE_DYNAMIC_TARGET, "s",
	                                &actual);

	g_assert_cmpstr(item->dynamic_target, ==, actual);
	g_free(actual);

	/* Check that the target value matches our expectations. */
	found = g_menu_model_get_item_attribute(model, index,
	                                        G_MENU_ATTRIBUTE_TARGET, "s",
	                                        &actual);

	if(item->target != NULL) {
		g_assert_true(found);

		g_assert_cmpstr(item->target, ==, actual);
		g_free(actual);
	} else {
		g_assert_false(found);
	}

	/* Free our data and move to the next item. */
	g_free(item);
	walk_data->items = g_list_delete_link(walk_data->items, walk_data->items);
}

static void
test_purple_menu_items(GMenu *menu, GList *items) {
	TestPurpleMenuWalkData data = {
		.items = items,
	};

	purple_menu_walk(G_MENU_MODEL(menu), test_purple_menu_items_func, &data);

	g_assert_null(data.items);
}

/******************************************************************************
 * Tests
 *****************************************************************************/
static void
test_purple_menu_single_no_dynamic_target(void) {
	GMenu *menu = g_menu_new();
	GList *expected = NULL;

	g_menu_append(menu, "item1", NULL);
	expected = g_list_append(expected, test_purple_menu_item_new(NULL, NULL));

	purple_menu_populate_dynamic_targets(menu, "property", "1", NULL);

	test_purple_menu_items(menu, expected);

	g_object_unref(menu);
}

static void
test_purple_menu_single_unset_dynamic_target(void) {
	GMenu *menu = g_menu_new();
	GMenuItem *item = NULL;
	GList *expected = NULL;

	item = g_menu_item_new("item1", NULL);
	g_menu_item_set_attribute(item, PURPLE_MENU_ATTRIBUTE_DYNAMIC_TARGET, "s",
	                          "foo");
	g_menu_append_item(menu, item);
	g_object_unref(item);
	expected = g_list_append(expected, test_purple_menu_item_new("foo", NULL));

	purple_menu_populate_dynamic_targets(menu, "bar", "123", NULL);

	test_purple_menu_items(menu, expected);

	g_object_unref(menu);
}

static void
test_purple_menu_single_single_dynamic_target(void) {
	GMenu *menu = g_menu_new();
	GMenuItem *item = NULL;
	GList *expected = NULL;

	item = g_menu_item_new("item1", NULL);
	g_menu_item_set_attribute(item, PURPLE_MENU_ATTRIBUTE_DYNAMIC_TARGET, "s",
	                          "foo");
	g_menu_append_item(menu, item);
	g_object_unref(item);
	expected = g_list_append(expected, test_purple_menu_item_new("foo", "bar"));

	purple_menu_populate_dynamic_targets(menu, "foo", "bar", NULL);

	test_purple_menu_items(menu, expected);

	g_object_unref(menu);
}

static void
test_purple_menu_section_single(void) {
	GMenu *menu = NULL, *section = NULL;
	GMenuItem *item = NULL;
	GList *expected = NULL;

	/* Create our section. */
	section = g_menu_new();
	expected = g_list_append(expected, test_purple_menu_item_new(NULL, NULL));

	/* Create our item and add it to the list. */
	item = g_menu_item_new("item1", NULL);
	g_menu_item_set_attribute(item, PURPLE_MENU_ATTRIBUTE_DYNAMIC_TARGET, "s",
	                          "foo");
	g_menu_append_item(section, item);
	g_object_unref(item);
	expected = g_list_append(expected, test_purple_menu_item_new("foo", "bar"));

	/* Finally add our section to our menu. */
	menu = g_menu_new();
	g_menu_append_section(menu, NULL, G_MENU_MODEL(section));
	g_object_unref(section);

	purple_menu_populate_dynamic_targets(menu, "foo", "bar", NULL);

	test_purple_menu_items(menu, expected);

	g_object_unref(menu);
}

static void
test_purple_menu_multiple_multiple_dynamic_target(void) {
	GMenu *menu = g_menu_new();
	GMenuItem *item = NULL;
	GList *expected = NULL;

	item = g_menu_item_new("item1", NULL);
	g_menu_item_set_attribute(item, PURPLE_MENU_ATTRIBUTE_DYNAMIC_TARGET, "s",
	                          "foo");
	g_menu_append_item(menu, item);
	g_object_unref(item);
	expected = g_list_append(expected, test_purple_menu_item_new("foo", "bar"));

	item = g_menu_item_new("item2", NULL);
	g_menu_item_set_attribute(item, PURPLE_MENU_ATTRIBUTE_DYNAMIC_TARGET, "s",
	                          "abc");
	g_menu_append_item(menu, item);
	g_object_unref(item);
	expected = g_list_append(expected, test_purple_menu_item_new("abc", "123"));

	purple_menu_populate_dynamic_targets(menu, "foo", "bar", "abc", "123",
	                                     NULL);

	test_purple_menu_items(menu, expected);

	g_object_unref(menu);
}

static void
test_purple_menu_multiple_mixed(void) {
	GMenu *menu = g_menu_new();
	GMenuItem *item = NULL;
	GList *expected = NULL;

	g_menu_append(menu, "item1", NULL);
	expected = g_list_append(expected, test_purple_menu_item_new(NULL, NULL));

	item = g_menu_item_new("item2", NULL);
	g_menu_item_set_attribute(item, PURPLE_MENU_ATTRIBUTE_DYNAMIC_TARGET, "s",
	                          "foo");
	g_menu_append_item(menu, item);
	g_object_unref(item);
	expected = g_list_append(expected, test_purple_menu_item_new("foo", "bar"));

	item = g_menu_item_new("item3", NULL);
	g_menu_item_set_attribute(item, PURPLE_MENU_ATTRIBUTE_DYNAMIC_TARGET, "s",
	                          "abc");
	g_menu_append_item(menu, item);
	g_object_unref(item);
	expected = g_list_append(expected, test_purple_menu_item_new("abc", "123"));

	purple_menu_populate_dynamic_targets(menu, "foo", "bar", "abc", "123",
	                                     NULL);

	test_purple_menu_items(menu, expected);

	g_object_unref(menu);
}

/******************************************************************************
 * Main
 *****************************************************************************/
gint
main(gint argc, gchar *argv[]) {
	g_test_init(&argc, &argv, NULL);

	g_test_add_func("/menu/single/no-dynamic-target",
	                test_purple_menu_single_no_dynamic_target);
	g_test_add_func("/menu/single/unset-dynamic-target",
	                test_purple_menu_single_unset_dynamic_target);
	g_test_add_func("/menu/single/single_dynamic_target",
	                test_purple_menu_single_single_dynamic_target);

	g_test_add_func("/menu/section/single",
	                test_purple_menu_section_single);

	g_test_add_func("/menu/multiple/multiple_dynamic_target",
	                test_purple_menu_multiple_multiple_dynamic_target);
	g_test_add_func("/menu/multiple/mixed",
	                test_purple_menu_multiple_mixed);

	return g_test_run();
}