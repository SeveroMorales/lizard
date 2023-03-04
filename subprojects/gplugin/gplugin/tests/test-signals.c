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

typedef struct {
	gboolean loading;
	gboolean loaded;
	gboolean unloading;
	gboolean unloaded;
	gboolean load_failed;
	gboolean unload_failed;
} TestGPluginManagerSignalsData;

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static gboolean
test_gplugin_manager_signals_normal_loading(
	G_GNUC_UNUSED GObject *manager,
	G_GNUC_UNUSED GPluginPlugin *plugin,
	G_GNUC_UNUSED GError **error,
	gpointer d)
{
	TestGPluginManagerSignalsData *data = (TestGPluginManagerSignalsData *)d;

	data->loading = TRUE;

	return TRUE;
}

static void
test_gplugin_manager_signals_normal_loaded(
	G_GNUC_UNUSED GObject *manager,
	G_GNUC_UNUSED GPluginPlugin *plugin,
	gpointer d)
{
	TestGPluginManagerSignalsData *data = (TestGPluginManagerSignalsData *)d;

	data->loaded = TRUE;
}

static gboolean
test_gplugin_manager_signals_normal_unloading(
	G_GNUC_UNUSED GObject *manager,
	G_GNUC_UNUSED GPluginPlugin *plugin,
	G_GNUC_UNUSED GError **error,
	gpointer d)
{
	TestGPluginManagerSignalsData *data = (TestGPluginManagerSignalsData *)d;

	data->unloading = TRUE;

	return TRUE;
}

static void
test_gplugin_manager_signals_normal_unloaded(
	G_GNUC_UNUSED GObject *manager,
	G_GNUC_UNUSED GPluginPlugin *plugin,
	gpointer d)
{
	TestGPluginManagerSignalsData *data = (TestGPluginManagerSignalsData *)d;

	data->unloaded = TRUE;
}

static gboolean
test_gplugin_manager_signals_stop_loading(
	G_GNUC_UNUSED GObject *manager,
	G_GNUC_UNUSED GPluginPlugin *plugin,
	GError **error,
	gpointer d)
{
	TestGPluginManagerSignalsData *data = (TestGPluginManagerSignalsData *)d;

	data->loading = TRUE;

	*error = g_error_new(GPLUGIN_DOMAIN, 0, "loading failed");

	return FALSE;
}

static gboolean
test_gplugin_manager_signals_stop_unloading(
	G_GNUC_UNUSED GObject *manager,
	G_GNUC_UNUSED GPluginPlugin *plugin,
	GError **error,
	gpointer d)
{
	TestGPluginManagerSignalsData *data = (TestGPluginManagerSignalsData *)d;

	data->unloading = TRUE;

	*error = g_error_new(GPLUGIN_DOMAIN, 0, "unloading failed");

	return FALSE;
}

static void
test_gplugin_manager_signals_load_failed(
	G_GNUC_UNUSED GObject *manager,
	G_GNUC_UNUSED GPluginPlugin *plugin,
	G_GNUC_UNUSED GError *error,
	gpointer d)
{
	TestGPluginManagerSignalsData *data = (TestGPluginManagerSignalsData *)d;

	data->load_failed = TRUE;
}

static void
test_gplugin_manager_signals_unload_failed(
	G_GNUC_UNUSED GObject *manager,
	G_GNUC_UNUSED GPluginPlugin *plugin,
	G_GNUC_UNUSED GError *error,
	gpointer d)
{
	TestGPluginManagerSignalsData *data = (TestGPluginManagerSignalsData *)d;

	data->unload_failed = TRUE;
}

/******************************************************************************
 * Tests
 *****************************************************************************/
static void
test_gplugin_manager_signals_normal(void)
{
	GPluginPlugin *plugin = NULL;
	GPluginManager *manager = gplugin_manager_get_default();
	GError *error = NULL;
	TestGPluginManagerSignalsData data = {
		.loading = FALSE,
		.loaded = FALSE,
		.unloading = FALSE,
		.unloaded = FALSE,
	};
	gulong signals[] = {0, 0, 0, 0};

	signals[0] = g_signal_connect(
		manager,
		"loading-plugin",
		G_CALLBACK(test_gplugin_manager_signals_normal_loading),
		&data);
	signals[1] = g_signal_connect(
		manager,
		"loaded-plugin",
		G_CALLBACK(test_gplugin_manager_signals_normal_loaded),
		&data);
	signals[2] = g_signal_connect(
		manager,
		"unloading-plugin",
		G_CALLBACK(test_gplugin_manager_signals_normal_unloading),
		&data);
	signals[3] = g_signal_connect(
		manager,
		"unloaded-plugin",
		G_CALLBACK(test_gplugin_manager_signals_normal_unloaded),
		&data);

	gplugin_manager_append_path(manager, TEST_DIR);
	gplugin_manager_refresh(manager);

	plugin =
		gplugin_manager_find_plugin(manager, "gplugin/native-basic-plugin");
	gplugin_manager_load_plugin(manager, plugin, &error);
	g_assert_no_error(error);
	g_assert_true(data.loading);
	g_assert_true(data.loaded);

	gplugin_manager_unload_plugin(manager, plugin, &error);
	g_assert_no_error(error);
	g_assert_true(data.unloading);
	g_assert_true(data.unloaded);

	g_signal_handler_disconnect(manager, signals[0]);
	g_signal_handler_disconnect(manager, signals[1]);
	g_signal_handler_disconnect(manager, signals[2]);
	g_signal_handler_disconnect(manager, signals[3]);
}

static void
test_gplugin_manager_signals_loading_stopped(void)
{
	GPluginPlugin *plugin = NULL;
	GPluginManager *manager = gplugin_manager_get_default();
	GError *error = NULL;
	TestGPluginManagerSignalsData data = {
		.loading = FALSE,
		.loaded = FALSE,
		.unloading = FALSE,
		.unloaded = FALSE,
	};
	gulong signals[] = {0, 0, 0, 0};

	signals[0] = g_signal_connect(
		manager,
		"loading-plugin",
		G_CALLBACK(test_gplugin_manager_signals_stop_loading),
		&data);
	signals[1] = g_signal_connect(
		manager,
		"loaded-plugin",
		G_CALLBACK(test_gplugin_manager_signals_normal_loaded),
		&data);
	signals[2] = g_signal_connect(
		manager,
		"unloading-plugin",
		G_CALLBACK(test_gplugin_manager_signals_normal_unloading),
		&data);
	signals[3] = g_signal_connect(
		manager,
		"unloaded-plugin",
		G_CALLBACK(test_gplugin_manager_signals_normal_unloaded),
		&data);

	gplugin_manager_append_path(manager, TEST_DIR);
	gplugin_manager_refresh(manager);

	plugin =
		gplugin_manager_find_plugin(manager, "gplugin/native-basic-plugin");
	gplugin_manager_load_plugin(manager, plugin, &error);
	g_assert_error(error, GPLUGIN_DOMAIN, 0);
	g_error_free(error);

	g_assert_true(data.loading);
	g_assert_false(data.loaded);
	g_assert_false(data.unloading);
	g_assert_false(data.unloaded);

	g_signal_handler_disconnect(manager, signals[0]);
	g_signal_handler_disconnect(manager, signals[1]);
	g_signal_handler_disconnect(manager, signals[2]);
	g_signal_handler_disconnect(manager, signals[3]);
}

static void
test_gplugin_manager_signals_unloading_stopped(void)
{
	GPluginPlugin *plugin = NULL;
	GPluginManager *manager = gplugin_manager_get_default();
	GError *error = NULL;
	TestGPluginManagerSignalsData data = {
		.loading = FALSE,
		.loaded = FALSE,
		.unloading = FALSE,
		.unloaded = FALSE,
	};
	gulong signals[] = {0, 0, 0, 0};

	signals[0] = g_signal_connect(
		manager,
		"loading-plugin",
		G_CALLBACK(test_gplugin_manager_signals_normal_loading),
		&data);
	signals[1] = g_signal_connect(
		manager,
		"loaded-plugin",
		G_CALLBACK(test_gplugin_manager_signals_normal_loaded),
		&data);
	signals[2] = g_signal_connect(
		manager,
		"unloading-plugin",
		G_CALLBACK(test_gplugin_manager_signals_stop_unloading),
		&data);
	signals[3] = g_signal_connect(
		manager,
		"unloaded-plugin",
		G_CALLBACK(test_gplugin_manager_signals_normal_unloaded),
		&data);

	gplugin_manager_append_path(manager, TEST_DIR);
	gplugin_manager_refresh(manager);

	plugin =
		gplugin_manager_find_plugin(manager, "gplugin/native-basic-plugin");
	gplugin_manager_load_plugin(manager, plugin, &error);
	g_assert_no_error(error);
	g_assert_true(data.loading);
	g_assert_true(data.loaded);

	gplugin_manager_unload_plugin(manager, plugin, &error);
	g_assert_error(error, GPLUGIN_DOMAIN, 0);
	g_error_free(error);

	g_assert_true(data.unloading);
	g_assert_false(data.unloaded);

	g_signal_handler_disconnect(manager, signals[0]);
	g_signal_handler_disconnect(manager, signals[1]);
	g_signal_handler_disconnect(manager, signals[2]);
	g_signal_handler_disconnect(manager, signals[3]);
}

static void
test_gplugin_manager_signals_load_failure(void)
{
	GPluginPlugin *plugin = NULL;
	GPluginManager *manager = gplugin_manager_get_default();
	GError *error = NULL;
	TestGPluginManagerSignalsData data = {
		.loading = FALSE,
		.load_failed = FALSE,
	};
	gulong signals[] = {0, 0, 0, 0};

	signals[0] = g_signal_connect(
		manager,
		"loading-plugin",
		G_CALLBACK(test_gplugin_manager_signals_normal_loading),
		&data);
	signals[1] = g_signal_connect(
		manager,
		"load-plugin-failed",
		G_CALLBACK(test_gplugin_manager_signals_load_failed),
		&data);

	gplugin_manager_append_path(manager, TEST_DIR);
	gplugin_manager_refresh(manager);

	plugin = gplugin_manager_find_plugin(manager, "gplugin/native-load-failed");
	gplugin_manager_load_plugin(manager, plugin, &error);
	g_assert_error(error, GPLUGIN_DOMAIN, 0);
	g_error_free(error);

	g_assert_true(data.loading);
	g_assert_true(data.load_failed);

	g_signal_handler_disconnect(manager, signals[0]);
	g_signal_handler_disconnect(manager, signals[1]);
}

static void
test_gplugin_manager_signals_unload_failure(void)
{
	GPluginPlugin *plugin = NULL;
	GPluginManager *manager = gplugin_manager_get_default();
	GError *error = NULL;
	TestGPluginManagerSignalsData data = {
		.loading = FALSE,
		.loaded = FALSE,
		.unloading = FALSE,
		.unload_failed = FALSE,
	};
	gulong signals[] = {0, 0, 0, 0};

	signals[0] = g_signal_connect(
		manager,
		"loading-plugin",
		G_CALLBACK(test_gplugin_manager_signals_normal_loading),
		&data);
	signals[1] = g_signal_connect(
		manager,
		"loaded-plugin",
		G_CALLBACK(test_gplugin_manager_signals_normal_loaded),
		&data);
	signals[2] = g_signal_connect(
		manager,
		"unloading-plugin",
		G_CALLBACK(test_gplugin_manager_signals_normal_unloading),
		&data);
	signals[3] = g_signal_connect(
		manager,
		"unload-plugin-failed",
		G_CALLBACK(test_gplugin_manager_signals_unload_failed),
		&data);

	gplugin_manager_append_path(manager, TEST_DIR);
	gplugin_manager_refresh(manager);

	plugin =
		gplugin_manager_find_plugin(manager, "gplugin/native-unload-failed");
	gplugin_manager_load_plugin(manager, plugin, &error);
	g_assert_no_error(error);
	g_assert_true(data.loading);
	g_assert_true(data.loaded);

	gplugin_manager_unload_plugin(manager, plugin, &error);
	g_assert_error(error, GPLUGIN_DOMAIN, 0);
	g_error_free(error);

	g_assert_true(data.unloading);
	g_assert_true(data.unload_failed);

	g_signal_handler_disconnect(manager, signals[0]);
	g_signal_handler_disconnect(manager, signals[1]);
}

/******************************************************************************
 * Main
 *****************************************************************************/
gint
main(gint argc, gchar **argv)
{
	g_test_init(&argc, &argv, G_TEST_OPTION_ISOLATE_DIRS, NULL);

	gplugin_init(GPLUGIN_CORE_FLAGS_NONE);

	g_test_add_func(
		"/manager/signals/normal",
		test_gplugin_manager_signals_normal);
	g_test_add_func(
		"/manager/signals/loading-stopped",
		test_gplugin_manager_signals_loading_stopped);
	g_test_add_func(
		"/manager/signals/unloading-stopped",
		test_gplugin_manager_signals_unloading_stopped);
	g_test_add_func(
		"/manager/signals/load-failed",
		test_gplugin_manager_signals_load_failure);
	g_test_add_func(
		"/manager/signals/unload-failed",
		test_gplugin_manager_signals_unload_failure);

	return g_test_run();
}
