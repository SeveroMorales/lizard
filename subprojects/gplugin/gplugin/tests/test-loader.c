/*
 * Copyright (C) 2011-2021 Gary Kramlich <grim@reaperworld.com>
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
 * TestGPluginPlugin
 *****************************************************************************/
#define TEST_GPLUGIN_TYPE_PLUGIN (test_gplugin_plugin_get_type())
G_DECLARE_FINAL_TYPE(
	TestGPluginPlugin,
	test_gplugin_plugin,
	TEST_GPLUGIN,
	PLUGIN,
	GObject)

enum {
	PROP_0,
	PROP_FILENAME,
	PROP_LOADER,
	PROP_INFO,
	PROP_STATE,
	PROP_DESIRED_STATE,
	PROP_ERROR,
};

struct _TestGPluginPlugin {
	GObject parent;

	/* overrides */
	gchar *filename;
	GPluginLoader *loader;
	GPluginPluginInfo *info;
	GPluginPluginState state;
	GPluginPluginState desired_state;
	GError *error;
};

static void
test_gplugin_plugin_iface_init(G_GNUC_UNUSED GPluginPluginInterface *iface)
{
}

G_DEFINE_TYPE_WITH_CODE(
	TestGPluginPlugin,
	test_gplugin_plugin,
	G_TYPE_OBJECT,
	G_IMPLEMENT_INTERFACE(GPLUGIN_TYPE_PLUGIN, test_gplugin_plugin_iface_init))

static void
test_gplugin_plugin_get_property(
	GObject *obj,
	guint param_id,
	GValue *value,
	GParamSpec *pspec)
{
	TestGPluginPlugin *plugin = TEST_GPLUGIN_PLUGIN(obj);
	switch(param_id) {
		case PROP_FILENAME:
			g_value_set_string(value, plugin->filename);
			break;
		case PROP_LOADER:
			g_value_set_object(value, plugin->loader);
			break;
		case PROP_INFO:
			g_value_set_object(value, plugin->info);
			break;
		case PROP_STATE:
			g_value_set_enum(value, plugin->state);
			break;
		case PROP_DESIRED_STATE:
			g_value_set_enum(value, plugin->desired_state);
			break;
		case PROP_ERROR:
			g_value_set_boxed(value, plugin->error);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
test_gplugin_plugin_set_property(
	GObject *obj,
	guint param_id,
	const GValue *value,
	GParamSpec *pspec)
{
	TestGPluginPlugin *plugin = TEST_GPLUGIN_PLUGIN(obj);

	switch(param_id) {
		case PROP_FILENAME:
			plugin->filename = g_strdup(g_value_get_string(value));
			break;
		case PROP_LOADER:
			plugin->loader = g_value_dup_object(value);
			break;
		case PROP_INFO:
			plugin->info = g_value_dup_object(value);
			break;
		case PROP_STATE:
			plugin->state = g_value_get_enum(value);
			break;
		case PROP_DESIRED_STATE:
			plugin->desired_state = g_value_get_enum(value);
			break;
		case PROP_ERROR:
			plugin->error = g_value_dup_boxed(value);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
test_gplugin_plugin_finalize(GObject *obj)
{
	TestGPluginPlugin *plugin = TEST_GPLUGIN_PLUGIN(obj);

	g_clear_pointer(&plugin->filename, g_free);
	g_clear_object(&plugin->loader);
	g_clear_object(&plugin->info);
	g_clear_error(&plugin->error);

	G_OBJECT_CLASS(test_gplugin_plugin_parent_class)->finalize(obj);
}

static void
test_gplugin_plugin_init(G_GNUC_UNUSED TestGPluginPlugin *plugin)
{
}

static void
test_gplugin_plugin_class_init(TestGPluginPluginClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->get_property = test_gplugin_plugin_get_property;
	obj_class->set_property = test_gplugin_plugin_set_property;
	obj_class->finalize = test_gplugin_plugin_finalize;

	g_object_class_override_property(obj_class, PROP_FILENAME, "filename");
	g_object_class_override_property(obj_class, PROP_LOADER, "loader");
	g_object_class_override_property(obj_class, PROP_INFO, "info");
	g_object_class_override_property(obj_class, PROP_STATE, "state");
	g_object_class_override_property(
		obj_class,
		PROP_DESIRED_STATE,
		"desired-state");
	g_object_class_override_property(obj_class, PROP_ERROR, "error");
}

/******************************************************************************
 * TestGPluginLoader
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

	gboolean supported_extensions;
	gboolean query;
	gboolean load;
	gboolean unload;
};

static GSList *
test_gplugin_loader_supported_extensions(GPluginLoader *l)
{
	TestGPluginLoader *loader = TEST_GPLUGIN_LOADER(l);

	loader->supported_extensions = TRUE;

	return g_slist_append(NULL, "");
}

static GPluginPlugin *
test_gplugin_loader_query(
	GPluginLoader *l,
	G_GNUC_UNUSED const gchar *filename,
	G_GNUC_UNUSED GError **error)
{
	TestGPluginLoader *loader = TEST_GPLUGIN_LOADER(l);

	loader->query = TRUE;

	return GPLUGIN_PLUGIN(g_object_new(TEST_GPLUGIN_TYPE_PLUGIN, NULL));
}

static gboolean
test_gplugin_loader_load(
	GPluginLoader *l,
	G_GNUC_UNUSED GPluginPlugin *plugin,
	G_GNUC_UNUSED GError **error)
{
	TestGPluginLoader *loader = TEST_GPLUGIN_LOADER(l);

	loader->load = TRUE;

	return TRUE;
}

static gboolean
test_gplugin_loader_unload(
	GPluginLoader *l,
	G_GNUC_UNUSED GPluginPlugin *plugin,
	G_GNUC_UNUSED gboolean shutdown,
	G_GNUC_UNUSED GError **error)
{
	TestGPluginLoader *loader = TEST_GPLUGIN_LOADER(l);

	loader->unload = TRUE;

	return TRUE;
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
 * Tests
 *****************************************************************************/
static void
test_gplugin_loader_properties(void)
{
	GPluginLoader *loader = test_gplugin_loader_new();
	gchar *id = NULL;

	g_assert_cmpstr("test-loader", ==, gplugin_loader_get_id(loader));

	g_object_get(G_OBJECT(loader), "id", &id, NULL);
	g_assert_cmpstr("test-loader", ==, id);
	g_clear_pointer(&id, g_free);

	g_clear_object(&loader);
}

static void
test_gplugin_loader_methods(void)
{
	GPluginLoader *loader = test_gplugin_loader_new();
	TestGPluginLoader *tl = TEST_GPLUGIN_LOADER(loader);
	GPluginPlugin *plugin = NULL;
	GError *error = NULL;
	GSList *exts = NULL;
	gboolean r = FALSE;

	g_assert_false(tl->supported_extensions);
	g_assert_false(tl->query);
	g_assert_false(tl->load);
	g_assert_false(tl->unload);

	exts = gplugin_loader_get_supported_extensions(loader);
	g_assert_nonnull(exts);
	g_slist_free(exts);
	g_assert_true(tl->supported_extensions);

	plugin = gplugin_loader_query_plugin(loader, ":test:", &error);
	g_assert_no_error(error);
	g_assert_nonnull(plugin);
	g_assert_true(tl->query);

	r = gplugin_loader_load_plugin(loader, plugin, &error);
	g_assert_no_error(error);
	g_assert_true(r);
	g_assert_true(tl->load);

	r = gplugin_loader_unload_plugin(loader, plugin, FALSE, &error);
	g_assert_no_error(error);
	g_assert_true(r);
	g_assert_true(tl->unload);

	g_clear_object(&plugin);
	g_clear_object(&loader);
}

/******************************************************************************
 * Main
 *****************************************************************************/
gint
main(gint argc, gchar **argv)
{
	g_test_init(&argc, &argv, G_TEST_OPTION_ISOLATE_DIRS, NULL);

	g_test_add_func("/loader/properties", test_gplugin_loader_properties);
	g_test_add_func("/loader/methods", test_gplugin_loader_methods);

	return g_test_run();
}
