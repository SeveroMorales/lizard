/*
 * Copyright (C) 2011-2014 Gary Kramlich <grim@reaperworld.com>
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

#define test_path_count(manager, e) \
	G_STMT_START \
		{ \
			GList *paths = gplugin_manager_get_paths((manager)); \
			g_assert_cmpint(g_list_length(paths), ==, (e)); \
		} \
	G_STMT_END

/******************************************************************************
 * Tests
 *****************************************************************************/
static void
test_gplugin_manager_paths_single(void)
{
	GPluginManager *manager = gplugin_manager_get_default();

	gplugin_manager_append_path(manager, "foo");
	test_path_count(manager, 1);

	gplugin_manager_remove_path(manager, "foo");
	test_path_count(manager, 0);
}

static void
test_gplugin_manager_paths_duplicate(void)
{
	GPluginManager *manager = gplugin_manager_get_default();

	gplugin_manager_append_path(manager, "foo");
	gplugin_manager_append_path(manager, "foo");

	test_path_count(manager, 1);

	gplugin_manager_remove_path(manager, "foo");
	test_path_count(manager, 0);
}

static void
test_gplugin_manager_paths_multiple_fifo(void)
{
	GPluginManager *manager = gplugin_manager_get_default();

	/* add */
	gplugin_manager_append_path(manager, "foo");
	test_path_count(manager, 1);

	gplugin_manager_append_path(manager, "bar");
	test_path_count(manager, 2);

	/* remove */
	gplugin_manager_remove_path(manager, "foo");
	test_path_count(manager, 1);

	gplugin_manager_remove_path(manager, "bar");
	test_path_count(manager, 0);
}

static void
test_gplugin_manager_paths_multiple_filo(void)
{
	GPluginManager *manager = gplugin_manager_get_default();

	/* add */
	gplugin_manager_append_path(manager, "foo");
	test_path_count(manager, 1);

	gplugin_manager_append_path(manager, "bar");
	test_path_count(manager, 2);

	/* remove */
	gplugin_manager_remove_path(manager, "bar");
	test_path_count(manager, 1);

	gplugin_manager_remove_path(manager, "foo");
	test_path_count(manager, 0);
}

static void
test_gplugin_manager_paths_unicode(void)
{
	GPluginManager *manager = gplugin_manager_get_default();

	test_path_count(manager, 0);

	gplugin_manager_append_path(manager, "/home/🐦/.plugins");
	test_path_count(manager, 1);

	gplugin_manager_append_path(manager, "/home/user/.plugins");
	test_path_count(manager, 2);

	gplugin_manager_remove_path(manager, "/home/🐦/.plugins");
	test_path_count(manager, 1);

	gplugin_manager_remove_path(manager, "/home/user/.plugins");
	test_path_count(manager, 0);
}

static void
test_gplugin_manager_add_multiple_mixed_trailing_slashes(void)
{
	GPluginManager *manager = gplugin_manager_get_default();

	test_path_count(manager, 0);

	gplugin_manager_append_path(
		manager,
		G_DIR_SEPARATOR_S "home" G_DIR_SEPARATOR_S "user1" G_DIR_SEPARATOR_S
						  ".plugins");
	test_path_count(manager, 1);

	gplugin_manager_append_path(
		manager,
		G_DIR_SEPARATOR_S "home" G_DIR_SEPARATOR_S "user2" G_DIR_SEPARATOR_S
						  ".plugins" G_DIR_SEPARATOR_S "");
	test_path_count(manager, 2);

	gplugin_manager_remove_path(
		manager,
		G_DIR_SEPARATOR_S "home" G_DIR_SEPARATOR_S "user1" G_DIR_SEPARATOR_S
						  ".plugins" G_DIR_SEPARATOR_S "");
	test_path_count(manager, 1);

	gplugin_manager_remove_path(
		manager,
		G_DIR_SEPARATOR_S "home" G_DIR_SEPARATOR_S "user2" G_DIR_SEPARATOR_S
						  ".plugins");
	test_path_count(manager, 0);
}

static void
test_gplugin_manager_add_default_paths(void)
{
	GPluginManager *manager = gplugin_manager_get_default();
	GHashTable *req = NULL;
	GList *paths = NULL, *l = NULL;
	gchar *path = NULL;
	guint size = 0;

	/* build our table of required paths */
	req = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

	/* create and add the paths we are expecting to the table */
	path = g_build_filename(PREFIX, LIBDIR, "gplugin", G_DIR_SEPARATOR_S, NULL);
	g_hash_table_add(req, path);

	path = g_build_filename(
		g_get_user_config_dir(),
		"gplugin",
		G_DIR_SEPARATOR_S,
		NULL);
	g_hash_table_add(req, path);

	/* now tell the plugin manager to add the default paths */
	gplugin_manager_add_default_paths(manager);

	/* now remove each path that the manager knows about from our table */
	paths = gplugin_manager_get_paths(manager);
	for(l = paths; l; l = l->next) {
		g_hash_table_remove(req, l->data);
	}

	size = g_hash_table_size(req);

	g_hash_table_destroy(req);

	g_assert_cmpuint(0, ==, size);
}

static void
test_gplugin_manager_add_app_paths(void)
{
	GPluginManager *manager = gplugin_manager_get_default();
	GHashTable *req = NULL;
	GList *paths = NULL, *l = NULL;
	const gchar *prefix = "/usr/local/";
	const gchar *appname = "foo";
	gchar *path = NULL;
	guint size = 0;

	/* build our table of required paths */
	req = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

	path = g_build_filename(prefix, LIBDIR, appname, G_DIR_SEPARATOR_S, NULL);
	g_hash_table_add(req, path);

	path = g_build_filename(
		g_get_user_config_dir(),
		appname,
		"plugins",
		G_DIR_SEPARATOR_S,
		NULL);
	g_hash_table_add(req, path);

	/* now add the app paths */
	gplugin_manager_add_app_paths(manager, prefix, "foo");

	/* now get all the paths that the manager is managing and remove them from
	 * our required table.
	 */
	paths = gplugin_manager_get_paths(manager);
	for(l = paths; l != NULL; l = l->next) {
		g_hash_table_remove(req, l->data);
	}

	/* now check the hash table size, if it's > 0 then an expected path wasn't
	 * added.
	 */
	size = g_hash_table_size(req);

	g_hash_table_destroy(req);

	g_assert_cmpuint(0, ==, size);
}

static void
test_gplugin_manager_add_app_paths_via_environ(void)
{
	GList *paths = NULL, *l = NULL;
	gint path_pos = 0;
	gchar *pre_paths, *post_paths;
	gint found_paths = 0;
	gint path_count;
	const gchar *pre_env_var_name = "GPLUGIN_EXTRA_TEST_PREPATHS";
	const gchar *post_env_var_name = "GPLUGIN_EXTRA_TEST_POSTPATHS";

	GPluginManager *manager = gplugin_manager_get_default();

	/* get number of paths to start */
	paths = gplugin_manager_get_paths(manager);
	path_count = g_list_length(paths);

	/* build contents of environment vars */
	pre_paths = g_strdup_printf(
		"%s%s%s",
		G_DIR_SEPARATOR_S "prefoo",
		G_SEARCHPATH_SEPARATOR_S,
		G_DIR_SEPARATOR_S "prebar" G_DIR_SEPARATOR_S);
	post_paths = g_strdup_printf(
		"%s%s%s",
		G_DIR_SEPARATOR_S "postfoo",
		G_SEARCHPATH_SEPARATOR_S,
		G_DIR_SEPARATOR_S "postbar" G_DIR_SEPARATOR_S);

	/* put /envfoo and /envbar as paths in an environment variable */
	g_setenv(pre_env_var_name, pre_paths, TRUE);
	g_setenv(post_env_var_name, post_paths, TRUE);

	/* have the manager add them */
	gplugin_manager_prepend_paths_from_environment(manager, pre_env_var_name);
	gplugin_manager_append_paths_from_environment(manager, post_env_var_name);

	/* now get the manager's current paths */
	paths = gplugin_manager_get_paths(manager);

	/* four more paths now */
	test_path_count(manager, path_count + 4);

	/* check order is correct */
	path_pos = 0;
	for(l = paths; l != NULL; l = l->next) {
		path_pos += 1;
		if(g_strcmp0(l->data, G_DIR_SEPARATOR_S "prefoo" G_DIR_SEPARATOR_S) ==
		   0) {
			g_assert_cmpint(2, >=, path_pos);
			found_paths += 1;
		} else if(
			g_strcmp0(l->data, G_DIR_SEPARATOR_S "prebar" G_DIR_SEPARATOR_S) ==
			0) {
			g_assert_cmpint(2, >=, path_pos);
			found_paths += 1;
		} else if(
			g_strcmp0(l->data, G_DIR_SEPARATOR_S "postfoo" G_DIR_SEPARATOR_S) ==
			0) {
			g_assert_cmpint(2, <, path_pos);
			found_paths += 1;
		} else if(
			g_strcmp0(l->data, G_DIR_SEPARATOR_S "postbar" G_DIR_SEPARATOR_S) ==
			0) {
			g_assert_cmpint(2, <, path_pos);
			found_paths += 1;
		}
	}
	g_assert_cmpint(4, ==, found_paths);
	g_free(pre_paths);
	g_free(post_paths);
}

gint
main(gint argc, gchar **argv)
{

	g_test_init(&argc, &argv, G_TEST_OPTION_ISOLATE_DIRS, NULL);

	gplugin_init(GPLUGIN_CORE_FLAGS_NONE);

	g_test_add_func(
		"/plugins/paths/add_remove_single",
		test_gplugin_manager_paths_single);

	g_test_add_func(
		"/plugins/paths/add_remove_duplicate",
		test_gplugin_manager_paths_duplicate);

	g_test_add_func(
		"/plugins/paths/add_remove_multiple_fifo",
		test_gplugin_manager_paths_multiple_fifo);

	g_test_add_func(
		"/plugins/paths/add_remove_multiple_filo",
		test_gplugin_manager_paths_multiple_filo);

	g_test_add_func(
		"/plugins/paths/add_multiple_unicode",
		test_gplugin_manager_paths_unicode);

	g_test_add_func(
		"/plugins/paths/add_multiple_mixed_trailing_slashes",
		test_gplugin_manager_add_multiple_mixed_trailing_slashes);

	g_test_add_func(
		"/plugins/paths/add_default_paths",
		test_gplugin_manager_add_default_paths);

	g_test_add_func(
		"/plugins/paths/add_app_paths",
		test_gplugin_manager_add_app_paths);

	g_test_add_func(
		"/plugins/paths/add_app_paths_via_environ",
		test_gplugin_manager_add_app_paths_via_environ);

	return g_test_run();
}
