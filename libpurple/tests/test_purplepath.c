#include <glib.h>

#include <purple.h>

#include "test_ui.h"

static void
test_purplepath_home_dir(void) {
	const gchar *home_dir;

#ifndef _WIN32
	home_dir = g_get_home_dir();
#else
	home_dir = g_getenv("APPDATA");
#endif
	g_assert_cmpstr(home_dir, ==, purple_home_dir());
}

static void
test_purplepath_cache_dir(void) {
	gchar *cache_dir;

	cache_dir = g_build_filename(g_get_user_cache_dir(), "test", NULL);
	g_assert_cmpstr(cache_dir, ==, purple_cache_dir());
	g_free(cache_dir);
}

static void
test_purplepath_config_dir(void) {
	gchar *config_dir;

	config_dir = g_build_filename(g_get_user_config_dir(), "test", NULL);
	g_assert_cmpstr(config_dir, ==, purple_config_dir());
	g_free(config_dir);
}

static void
test_purplepath_data_dir(void) {
	gchar *data_dir;

	data_dir = g_build_filename(g_get_user_data_dir(), "test", NULL);
	g_assert_cmpstr(data_dir, ==, purple_data_dir());
	g_free(data_dir);
}

gint
main(gint argc, gchar **argv) {
	g_test_init(&argc, &argv, NULL);

	test_ui_purple_init();

	g_test_add_func("/purplepath/homedir",
	                test_purplepath_home_dir);
	g_test_add_func("/purplepath/cachedir",
	                test_purplepath_cache_dir);
	g_test_add_func("/purplepath/configdir",
	                test_purplepath_config_dir);
	g_test_add_func("/purplepath/datadir",
	                test_purplepath_data_dir);
	return g_test_run();
}
