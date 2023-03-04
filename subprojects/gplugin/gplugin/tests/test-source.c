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

#include "gplugin-source.h"

/******************************************************************************
 * TestSource Implementation
 *****************************************************************************/
G_DECLARE_FINAL_TYPE(
	TestGPluginSource,
	test_gplugin_source,
	TEST_GPLUGIN,
	SOURCE,
	GObject)

struct _TestGPluginSource {
	GObject parent;

	gboolean scan_result;
};

static gboolean
test_gplugin_source_scan(GPluginSource *source)
{
	TestGPluginSource *test_source = TEST_GPLUGIN_SOURCE(source);

	return test_source->scan_result;
}

static void
test_gplugin_source_iface_init(GPluginSourceInterface *iface)
{
	iface->scan = test_gplugin_source_scan;
}

G_DEFINE_TYPE_WITH_CODE(
	TestGPluginSource,
	test_gplugin_source,
	G_TYPE_OBJECT,
	G_IMPLEMENT_INTERFACE(GPLUGIN_TYPE_SOURCE, test_gplugin_source_iface_init))

static void
test_gplugin_source_init(G_GNUC_UNUSED TestGPluginSource *source)
{
}

static void
test_gplugin_source_class_init(G_GNUC_UNUSED TestGPluginSourceClass *klass)
{
}

static GPluginSource *
test_gplugin_source_new(gboolean scan_result)
{
	TestGPluginSource *source =
		g_object_new(test_gplugin_source_get_type(), NULL);

	source->scan_result = scan_result;

	return GPLUGIN_SOURCE(source);
}

/******************************************************************************
 * Tests
 *****************************************************************************/
static void
test_gplugin_source_scan_false(void)
{
	GPluginSource *source = test_gplugin_source_new(FALSE);

	g_assert_false(gplugin_source_scan(source));

	g_clear_object(&source);
}

static void
test_gplugin_source_scan_true(void)
{
	GPluginSource *source = test_gplugin_source_new(TRUE);

	g_assert_true(gplugin_source_scan(source));

	g_clear_object(&source);
}

/******************************************************************************
 * Main
 *****************************************************************************/
gint
main(gint argc, gchar **argv)
{
	g_test_init(&argc, &argv, G_TEST_OPTION_ISOLATE_DIRS, NULL);

	g_test_add_func("/source/scan/false", test_gplugin_source_scan_false);
	g_test_add_func("/source/scan/true", test_gplugin_source_scan_true);

	return g_test_run();
}
