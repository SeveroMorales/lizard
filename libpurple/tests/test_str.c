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

#include <glib.h>

#include <purple.h>

/******************************************************************************
 * Tests
 *****************************************************************************/
static void
test_purple_strmatches_null(void) {
	g_assert_false(purple_strmatches("abc", NULL));
}

static void
test_purple_strmatches_adjacent(void) {
	g_assert_true(purple_strmatches("abc", "abc"));
	g_assert_true(purple_strmatches("abc", "123abc"));
	g_assert_true(purple_strmatches("abc", "abcxyz"));
	g_assert_true(purple_strmatches("abc", "123abcxyz"));
}

static void
test_purple_strmatches_sparse(void) {
	g_assert_true(purple_strmatches("adf", "abcdefg"));
	g_assert_true(purple_strmatches("jon", "john"));
	g_assert_true(purple_strmatches("lce", "alice"));
}

static void
test_purple_strmatches_iterates_correctly(void) {
	/* These tests are to make sure that we are iterating both the pattern and
	 * the string to check properly.
	 */

	/* since we have 2 e's in the pattern, this test makes sure that we iterate
	 * past the first matching e in the string we're checking.
	 */
	g_assert_false(purple_strmatches("beer", "berry"));
}

/******************************************************************************
 * Public API
 *****************************************************************************/
gint
main(gint argc, gchar **argv) {
	g_test_init(&argc, &argv, NULL);

	g_test_add_func("/strmatches/null", test_purple_strmatches_null);
	g_test_add_func("/strmatches/adjacent", test_purple_strmatches_adjacent);
	g_test_add_func("/strmatches/sparse", test_purple_strmatches_sparse);
	g_test_add_func("/strmatches/iterates_correctly",
	                test_purple_strmatches_iterates_correctly);

	return g_test_run();
}
