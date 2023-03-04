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
#include <glib-object.h>
#include <glib/gi18n-lib.h>

#include <gplugin/gplugin-core.h>
#include <gplugin/gplugin-manager.h>
#include <gplugin/gplugin-options.h>

/******************************************************************************
 * Options
 *****************************************************************************/
static gboolean add_default_paths = TRUE, register_native_loader = TRUE;
static gchar **paths = NULL;

static gboolean
gplugin_options_no_default_paths_cb(
	G_GNUC_UNUSED const gchar *n,
	G_GNUC_UNUSED const gchar *v,
	G_GNUC_UNUSED gpointer d,
	G_GNUC_UNUSED GError **e)
{
	add_default_paths = FALSE;

	return TRUE;
}

static gboolean
gplugin_options_no_native_loader_cb(
	G_GNUC_UNUSED const gchar *n,
	G_GNUC_UNUSED const gchar *v,
	G_GNUC_UNUSED gpointer d,
	G_GNUC_UNUSED GError **e)
{
	register_native_loader = FALSE;

	return TRUE;
}
/* clang-format off */
static GOptionEntry entries[] = {
	{
		"no-default-paths", 'D', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
		gplugin_options_no_default_paths_cb,
		N_("Do not search the default plugin paths"),
		NULL,
	}, {
		"no-native-loader", 0, G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
		gplugin_options_no_native_loader_cb,
		N_("Do not register the native plugin loaders"),
		NULL,
	}, {
		"path", 'p', 0, G_OPTION_ARG_STRING_ARRAY,
		&paths, N_("Additional path to look for plugins"),
		NULL,
	}, {
		NULL, 0, 0, 0, NULL, NULL, NULL
	},
};
/* clang-format on */

static gboolean
gplugin_options_post_parse_cb(
	G_GNUC_UNUSED GOptionContext *ctx,
	G_GNUC_UNUSED GOptionGroup *group,
	G_GNUC_UNUSED gpointer data,
	G_GNUC_UNUSED GError **error)
{
	GPluginManager *manager = NULL;
	GPluginCoreFlags flags = GPLUGIN_CORE_FLAGS_NONE;

	if(!register_native_loader) {
		flags |= GPLUGIN_CORE_FLAGS_DISABLE_NATIVE_LOADER;
	}

	gplugin_init(flags);

	manager = gplugin_manager_get_default();

	if(add_default_paths) {
		gplugin_manager_add_default_paths(manager);
	}

	if(paths != NULL) {
		guint i = 0;

		for(i = 0; paths[i] != NULL; i++) {
			gplugin_manager_prepend_path(manager, paths[i]);
		}

		g_clear_pointer(&paths, g_strfreev);
	}

	return TRUE;
}

/******************************************************************************
 * API
 *****************************************************************************/
/**
 * gplugin_get_option_group:
 *
 * Returns an option group for the commandline arguments recognized by GPlugin.
 *
 * You should add this option group to your [struct@GLib.OptionContext] with
 * [method@GLib.OptionContext.add_group], if you are using
 * [method@GLib.OptionContext.parse] to parse your commandline arguments.
 *
 * If [func@GPlugin.init] has yet to be called before
 * [method@GLib.OptionContext.parse] is called, [func@GPlugin.init] will be
 * called automatically.
 *
 * Returns: (transfer full): An option group for the commandline arguments
 *          recognized by GPlugin.
 */
GOptionGroup *
gplugin_get_option_group(void)
{
	GOptionGroup *group = NULL;

	group = g_option_group_new(
		"gplugin",
		_("GPlugin Options"),
		_("Show GPlugin Options"),
		NULL,
		NULL);

	g_option_group_set_parse_hooks(group, NULL, gplugin_options_post_parse_cb);
	g_option_group_add_entries(group, entries);
	g_option_group_set_translation_domain(group, GETTEXT_PACKAGE);

	return group;
}
