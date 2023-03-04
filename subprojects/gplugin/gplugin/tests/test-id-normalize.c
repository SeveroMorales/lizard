/*
 * Copyright (C) 2011-2020 Gary Kramlich <grim@reaperworld.com>
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

#include <gplugin.h>

/******************************************************************************
 * Tests
 *****************************************************************************/
static void
test_id_normalize(const gchar *input, const gchar *expected)
{
	GPluginPluginInfo *info = NULL;
	gchar *normalized = NULL;

	info = gplugin_plugin_info_new(input, 0, NULL);
	normalized = gplugin_plugin_info_get_id_normalized(info);
	g_object_unref(G_OBJECT(info));

	g_assert_cmpstr(normalized, ==, expected);

	g_free(normalized);
}

static void
test_id_normalize_null(void)
{
	test_id_normalize(NULL, NULL);
}

static void
test_id_normalize_alpha_upper(void)
{
	test_id_normalize(G_CSET_A_2_Z, G_CSET_A_2_Z);
}

static void
test_id_normalize_alpha_lower(void)
{
	test_id_normalize(G_CSET_a_2_z, G_CSET_a_2_z);
}

static void
test_id_normalize_digits(void)
{
	test_id_normalize(G_CSET_DIGITS, G_CSET_DIGITS);
}

static void
test_id_normalize_whitespace(void)
{
	test_id_normalize(" ", "-");
	test_id_normalize("\r", "-");
	test_id_normalize("\n", "-");
	test_id_normalize("\r\n", "--");
	test_id_normalize("\t", "-");

	/* this is a zero width space */
	test_id_normalize("\xe2\x80\x8b", "---");
}

static void
test_id_normalize_symbols(void)
{
	test_id_normalize("a~!@#$%^&*()_+{}|:\"<>?z", "a---------------------z");
	test_id_normalize("a`-=[]\\;',./z", "a-----------z");
}

static void
test_id_normalize_emoji(void)
{
	/* 🔌 is 4 bytes which is why we need 4 -'s */
	test_id_normalize("🔌", "----");
	test_id_normalize("a🔌z", "a----z");
}

/******************************************************************************
 * Main
 *****************************************************************************/
gint
main(gint argc, gchar **argv)
{
	g_test_init(&argc, &argv, G_TEST_OPTION_ISOLATE_DIRS, NULL);

	gplugin_init(GPLUGIN_CORE_FLAGS_NONE);

	g_test_add_func("/id-normalize/null", test_id_normalize_null);
	g_test_add_func("/id-normalize/alpha/upper", test_id_normalize_alpha_upper);
	g_test_add_func("/id-normalize/alpha/lower", test_id_normalize_alpha_lower);
	g_test_add_func("/id-normalize/digits", test_id_normalize_digits);
	g_test_add_func("/id-normalize/whitespace", test_id_normalize_whitespace);
	g_test_add_func("/id-normalize/symbols", test_id_normalize_symbols);
	g_test_add_func("/id-normalize/emoji", test_id_normalize_emoji);

	return g_test_run();
}
