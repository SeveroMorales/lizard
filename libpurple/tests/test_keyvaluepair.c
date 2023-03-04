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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib.h>

#include <purple.h>

/******************************************************************************
 * Tests
 *****************************************************************************/
static void
test_key_value_pair_new(void) {
	PurpleKeyValuePair *kvp = NULL;

	kvp = purple_key_value_pair_new("abc", "123");
	g_assert_nonnull(kvp);

	g_assert_cmpstr(kvp->key, ==, "abc");
	g_assert_cmpstr(kvp->value, ==, "123");
	g_assert_null(kvp->value_destroy_func);

	purple_key_value_pair_free(kvp);
}

static void
test_key_value_pair_new_full(void) {
	PurpleKeyValuePair *kvp = NULL;

	kvp = purple_key_value_pair_new_full("abc", g_strdup("123"), g_free);
	g_assert_nonnull(kvp);

	g_assert_cmpstr(kvp->key, ==, "abc");
	g_assert_cmpstr(kvp->value, ==, "123");
	g_assert_true(kvp->value_destroy_func == g_free);

	purple_key_value_pair_free(kvp);
}

static void
test_key_value_pair_copy(void) {
	PurpleKeyValuePair *kvp1 = NULL, *kvp2 = NULL;

	kvp1 = purple_key_value_pair_new("abc", "123");
	g_assert_nonnull(kvp1);

	kvp2 = purple_key_value_pair_copy(kvp1);
	g_assert_nonnull(kvp2);

	g_assert_cmpstr(kvp1->key, ==, kvp2->key);
	g_assert_cmpstr(kvp1->value, ==, kvp2->value);;
	g_assert_true(kvp1->value_destroy_func == kvp2->value_destroy_func);

	purple_key_value_pair_free(kvp1);
	purple_key_value_pair_free(kvp2);
}

static void
test_key_value_pair_copy_allocated_subprocess(void) {
	PurpleKeyValuePair *kvp1 = NULL, *kvp2 = NULL;

	kvp1 = purple_key_value_pair_new_full("abc", g_strdup("123"), g_free);
	g_assert_nonnull(kvp1);

	kvp2 = purple_key_value_pair_copy(kvp1);
	g_assert_null(kvp2);

	purple_key_value_pair_free(kvp1);
}

static void
test_key_value_pair_copy_allocated(void) {
	g_test_trap_subprocess("/key-value-pair/copy-allocated/subprocess", 0, 0);

	g_test_trap_assert_failed();
}

/******************************************************************************
 * Main
 *****************************************************************************/
gint
main(gint argc, gchar **argv) {
	g_test_init(&argc, &argv, NULL);

	g_test_set_nonfatal_assertions();

	g_test_add_func("/key-value-pair/new",
	                test_key_value_pair_new);
	g_test_add_func("/key-value-pair/new-full",
	                test_key_value_pair_new_full);
	g_test_add_func("/key-value-pair/copy",
	                test_key_value_pair_copy);
	g_test_add_func("/key-value-pair/copy-allocated",
	                test_key_value_pair_copy_allocated);
	g_test_add_func("/key-value-pair/copy-allocated/subprocess",
	                test_key_value_pair_copy_allocated_subprocess);

	return g_test_run();
}
