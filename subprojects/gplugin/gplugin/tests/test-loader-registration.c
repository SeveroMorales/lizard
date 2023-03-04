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

#include <gplugin.h>

#include "../gplugin-private.h"

#include <glib.h>

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
#define TEST_GPLUGIN_TYPE_LOADER (test_gplugin_loader_get_type())
G_DECLARE_FINAL_TYPE(
	TestGPluginLoader,
	test_gplugin_loader,
	TEST_GPLUGIN,
	LOADER,
	GPluginLoader)

struct _TestGPluginLoader {
	GPluginLoader parent;
};

static GSList *
test_gplugin_loader_supported_extensions(G_GNUC_UNUSED GPluginLoader *loader)
{
	return NULL;
}

static GPluginPlugin *
test_gplugin_loader_query(
	G_GNUC_UNUSED GPluginLoader *loader,
	G_GNUC_UNUSED const gchar *filename,
	G_GNUC_UNUSED GError **error)
{
	return NULL;
}

static gboolean
test_gplugin_loader_load(
	G_GNUC_UNUSED GPluginLoader *loader,
	G_GNUC_UNUSED GPluginPlugin *plugin,
	G_GNUC_UNUSED GError **error)
{
	return FALSE;
}

static gboolean
test_gplugin_loader_unload(
	G_GNUC_UNUSED GPluginLoader *loader,
	G_GNUC_UNUSED GPluginPlugin *plugin,
	G_GNUC_UNUSED gboolean shutdown,
	G_GNUC_UNUSED GError **error)
{
	return FALSE;
}

G_DEFINE_TYPE(TestGPluginLoader, test_gplugin_loader, GPLUGIN_TYPE_LOADER)

static void
test_gplugin_loader_init(G_GNUC_UNUSED TestGPluginLoader *loader)
{
}

static void
test_gplugin_loader_class_init(TestGPluginLoaderClass *klass)
{
	GPluginLoaderClass *loader_class = GPLUGIN_LOADER_CLASS(klass);

	loader_class->supported_extensions =
		test_gplugin_loader_supported_extensions;
	loader_class->query = test_gplugin_loader_query;
	loader_class->load = test_gplugin_loader_load;
	loader_class->unload = test_gplugin_loader_unload;
}

static GPluginLoader *
test_gplugin_loader_new(void)
{
	/* clang-format off */
	return GPLUGIN_LOADER(g_object_new(
		TEST_GPLUGIN_TYPE_LOADER,
		"id", "test-loader",
		NULL));
	/* clang-format on */
}

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
test_gplugin_manager_loader_registration_counter(
	G_GNUC_UNUSED GPluginManager *manager,
	G_GNUC_UNUSED GPluginLoader *loader,
	gpointer data)
{
	gint *counter = data;

	*counter = *counter + 1;
}

/******************************************************************************
 * Tests
 *****************************************************************************/
static void
test_gplugin_manager_loader_register_unregister(void)
{
	GPluginManager *manager = NULL;
	GPluginLoader *loader = NULL;
	GError *error = NULL;
	gboolean ret;
	gint registrations = 0, unregistrations = 0;

	gplugin_manager_private_uninit();
	gplugin_manager_private_init(TRUE);

	manager = gplugin_manager_get_default();
	g_signal_connect(
		manager,
		"loader-registered",
		G_CALLBACK(test_gplugin_manager_loader_registration_counter),
		&registrations);
	g_signal_connect(
		manager,
		"loader-unregistered",
		G_CALLBACK(test_gplugin_manager_loader_registration_counter),
		&unregistrations);

	loader = test_gplugin_loader_new();

	ret = gplugin_manager_register_loader(manager, loader, &error);
	g_assert_no_error(error);
	g_assert_true(ret);

	ret = gplugin_manager_unregister_loader(manager, loader, &error);
	g_assert_no_error(error);
	g_assert_true(ret);

	g_assert_cmpuint(registrations, ==, 1);
	g_assert_cmpuint(unregistrations, ==, 1);

	g_clear_object(&loader);
}

static void
test_gplugin_manager_loader_register_twice(void)
{
	GPluginManager *manager = NULL;
	GPluginLoader *loader = NULL;
	GError *error = NULL;
	gboolean ret;
	guint registrations = 0, unregistrations = 0;

	gplugin_manager_private_uninit();
	gplugin_manager_private_init(TRUE);

	manager = gplugin_manager_get_default();
	g_signal_connect(
		manager,
		"loader-registered",
		G_CALLBACK(test_gplugin_manager_loader_registration_counter),
		&registrations);
	g_signal_connect(
		manager,
		"loader-unregistered",
		G_CALLBACK(test_gplugin_manager_loader_registration_counter),
		&unregistrations);

	loader = test_gplugin_loader_new();

	ret = gplugin_manager_register_loader(manager, loader, &error);
	g_assert_no_error(error);
	g_assert_true(ret);

	ret = gplugin_manager_register_loader(manager, loader, &error);
	g_assert_false(ret);
	g_assert_error(error, GPLUGIN_DOMAIN, 0);
	g_clear_error(&error);

	ret = gplugin_manager_unregister_loader(manager, loader, &error);
	g_assert_no_error(error);
	g_assert_true(ret);

	g_assert_cmpuint(registrations, ==, 1);
	g_assert_cmpuint(unregistrations, ==, 1);

	g_clear_object(&loader);
}

static void
test_gplugin_manager_loader_unregister_twice(void)
{
	GPluginManager *manager = NULL;
	GPluginLoader *loader = NULL;
	GError *error = NULL;
	gboolean ret;
	guint registrations = 0, unregistrations = 0;

	gplugin_manager_private_uninit();
	gplugin_manager_private_init(TRUE);

	manager = gplugin_manager_get_default();
	g_signal_connect(
		manager,
		"loader-registered",
		G_CALLBACK(test_gplugin_manager_loader_registration_counter),
		&registrations);
	g_signal_connect(
		manager,
		"loader-unregistered",
		G_CALLBACK(test_gplugin_manager_loader_registration_counter),
		&unregistrations);

	loader = test_gplugin_loader_new();

	ret = gplugin_manager_register_loader(manager, loader, &error);
	g_assert_no_error(error);
	g_assert_true(ret);

	ret = gplugin_manager_unregister_loader(manager, loader, &error);
	g_assert_no_error(error);
	g_assert_true(ret);

	ret = gplugin_manager_unregister_loader(manager, loader, &error);
	g_assert_false(ret);
	g_assert_error(error, GPLUGIN_DOMAIN, 0);
	g_clear_error(&error);

	g_assert_cmpuint(registrations, ==, 1);
	g_assert_cmpuint(unregistrations, ==, 1);

	g_clear_object(&loader);
}

/******************************************************************************
 * Main
 *****************************************************************************/
gint
main(gint argc, gchar **argv)
{
	gint r = 0;

	g_test_init(&argc, &argv, G_TEST_OPTION_ISOLATE_DIRS, NULL);

	gplugin_init(GPLUGIN_CORE_FLAGS_NONE);

	g_test_add_func(
		"/manager/loader/register_unregister",
		test_gplugin_manager_loader_register_unregister);
	g_test_add_func(
		"/manager/loader/register-twice",
		test_gplugin_manager_loader_register_twice);
	g_test_add_func(
		"/manager/loader/unregister-twice",
		test_gplugin_manager_loader_unregister_twice);

	r = g_test_run();

	gplugin_uninit();

	return r;
}
