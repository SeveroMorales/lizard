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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <glib/gi18n.h>

#include <gplugin.h>

/******************************************************************************
 * Globals
 *****************************************************************************/
static gint verbosity = 0;
static gboolean show_internal = FALSE;
static gboolean output_paths = FALSE;
static gboolean exit_early = FALSE;

/******************************************************************************
 * Helpers
 *****************************************************************************/
static gboolean
verbosity_cb(
	G_GNUC_UNUSED const gchar *n,
	G_GNUC_UNUSED const gchar *v,
	G_GNUC_UNUSED gpointer d,
	G_GNUC_UNUSED GError **e)
{
	verbosity++;

	return TRUE;
}

static gboolean
full_verbosity_cb(
	G_GNUC_UNUSED const gchar *n,
	G_GNUC_UNUSED const gchar *v,
	G_GNUC_UNUSED gpointer d,
	G_GNUC_UNUSED GError **e)
{
	verbosity = 1 << 11;

	return TRUE;
}

static gboolean
internal_cb(
	G_GNUC_UNUSED const gchar *n,
	G_GNUC_UNUSED const gchar *v,
	G_GNUC_UNUSED gpointer d,
	G_GNUC_UNUSED GError **e)
{
	show_internal = TRUE;

	return TRUE;
}

static gboolean
version_cb(
	G_GNUC_UNUSED const gchar *n,
	G_GNUC_UNUSED const gchar *v,
	G_GNUC_UNUSED gpointer d,
	G_GNUC_UNUSED GError **e)
{
	printf("gplugin-query %s\n", GPLUGIN_VERSION);

	exit_early = TRUE;

	return TRUE;
}

static gboolean
list_cb(
	G_GNUC_UNUSED const gchar *n,
	G_GNUC_UNUSED const gchar *v,
	G_GNUC_UNUSED gpointer d,
	G_GNUC_UNUSED GError **e)
{
	output_paths = TRUE;

	return TRUE;
}

static gboolean
output_plugin(const gchar *id)
{
	GPluginManager *manager = gplugin_manager_get_default();
	GSList *plugins = NULL, *l = NULL;
	gboolean first = TRUE, header_output = FALSE;

#define FORMAT "%-13s"
#define MAIN_FORMAT_NEL "  " FORMAT ": "
#define MAIN_FORMAT MAIN_FORMAT_NEL "%s\n"
#define STR_OR_EMPTY(str) ((str) ? (str) : "")

	plugins = gplugin_manager_find_plugins(manager, id);
	if(plugins == NULL) {
		printf("%s not found\n", id);

		return FALSE;
	}

	for(l = plugins; l; l = l->next) {
		GPluginPlugin *plugin = GPLUGIN_PLUGIN(l->data);
		GPluginPluginInfo *info = gplugin_plugin_get_info(plugin);
		gboolean internal, loq, bind_global, unloadable;
		guint32 abi_version;
		gchar *name, *version;
		gchar *license_id, *license_text, *license_url;
		gchar *icon_name, *summary, *description, *category, *website;
		gchar **authors, **dependencies;
		gint i = 0;

		internal = gplugin_plugin_info_get_internal(info);

		if(!show_internal && internal)
			continue;

		if(!header_output) {
			printf("%s:\n", id);
			header_output = TRUE;
		}

		/* clang-format off */
		g_object_get(
			G_OBJECT(info),
			"abi-version", &abi_version,
			"load-on-query", &loq,
			"name", &name,
			"version", &version,
			"license-id", &license_id,
			"license-text", &license_text,
			"license-url", &license_url,
			"icon-name", &icon_name,
			"summary", &summary,
			"description", &description,
			"category", &category,
			"authors", &authors,
			"website", &website,
			"dependencies", &dependencies,
			"bind-global", &bind_global,
			"unloadable", &unloadable,
			NULL);
		/* clang-format on */

		if(!first)
			printf("\n");

		printf(MAIN_FORMAT, "name", STR_OR_EMPTY(name));
		if(verbosity > 0)
			printf(MAIN_FORMAT, "category", STR_OR_EMPTY(category));
		printf(MAIN_FORMAT, "version", STR_OR_EMPTY(version));
		if(verbosity > 0) {
			printf(MAIN_FORMAT, "license", STR_OR_EMPTY(license_id));
			printf(MAIN_FORMAT, "license url", STR_OR_EMPTY(license_url));
		}
		printf(MAIN_FORMAT, "summary", STR_OR_EMPTY(summary));
		if(verbosity > 0) {
			printf(MAIN_FORMAT_NEL, "authors");
			if(authors) {
				for(i = 0; authors[i]; i++) {
					if(i > 0)
						printf("                 ");
					printf("%s\n", STR_OR_EMPTY(authors[i]));
				}
			} else {
				printf("\n");
			}
			printf(MAIN_FORMAT, "website", STR_OR_EMPTY(website));
		}
		if(verbosity > 1) {
			printf(
				MAIN_FORMAT,
				"filename",
				STR_OR_EMPTY(gplugin_plugin_get_filename(plugin)));
		}
		if(verbosity > 2) {
			GPluginLoader *loader = gplugin_plugin_get_loader(plugin);

			printf(MAIN_FORMAT_NEL "%08x\n", "abi version", abi_version);
			printf(MAIN_FORMAT, "internal", (internal) ? "yes" : "no");
			printf(MAIN_FORMAT, "load on query", (loq) ? "yes" : "no");
			printf(MAIN_FORMAT, "bind globally", (bind_global) ? "yes" : "no");
			printf(MAIN_FORMAT, "unloadable", (unloadable) ? "yes" : "no");
			printf(MAIN_FORMAT, "loader", G_OBJECT_TYPE_NAME(loader));

			g_object_unref(G_OBJECT(loader));
		}
		if(verbosity > 0)
			printf(MAIN_FORMAT, "description", STR_OR_EMPTY(description));
		if(verbosity > 2) {
			printf(MAIN_FORMAT_NEL, "dependencies");
			if(dependencies) {
				for(i = 0; dependencies[i]; i++) {
					if(i > 0)
						printf("                 ");
					printf("%s\n", STR_OR_EMPTY(dependencies[i]));
				}
			} else {
				printf("\n");
			}
		}

		g_free(name);
		g_free(version);
		g_free(license_id);
		g_free(license_text);
		g_free(license_url);
		g_free(icon_name);
		g_free(summary);
		g_free(description);
		g_free(category);
		g_strfreev(authors);
		g_free(website);
		g_strfreev(dependencies);
		g_object_unref(G_OBJECT(info));

		if(first)
			first = FALSE;
	}

	g_slist_free_full(plugins, g_object_unref);

	return TRUE;
}

static gboolean
output_plugins(GList *plugins)
{
	GList *l = NULL;
	gboolean ret = TRUE, first = TRUE;

	for(l = plugins; l; l = l->next) {
		if(!first)
			printf("\n");

		if(!output_plugin(l->data))
			ret = FALSE;

		if(first)
			first = FALSE;
	}

	return ret;
}

/******************************************************************************
 * Main Stuff
 *****************************************************************************/
/* clang-format off */
static GOptionEntry entries[] = {
	{
		"internal", 'i', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
		internal_cb, N_("Show internal plugins"),
		NULL,
	}, {
		"verbose", 'v', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
		verbosity_cb, N_("Increase verbosity"),
		NULL,
	}, {
		"full-verbose", 'V', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
		full_verbosity_cb, N_("Increase verbosity to eleven"),
		NULL,
	}, {
		"version", 0, G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
		version_cb, N_("Display the version and exit"),
		NULL,
	}, {
		"list", 'L', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
		list_cb, N_("Display all search paths and exit"),
		NULL,
	}, {
		NULL, 0, 0, 0, NULL, NULL, NULL,
	}
};
/* clang-format on */

gint
main(gint argc, gchar **argv)
{
	GPluginManager *manager = NULL;
	GError *error = NULL;
	GOptionContext *ctx = NULL;
	GOptionGroup *group = NULL;
	gint i = 0, ret = 0;

	ctx = g_option_context_new("PLUGIN-ID...");
	g_option_context_set_summary(ctx, _("Query installed plugins"));
	g_option_context_set_translation_domain(ctx, GETTEXT_PACKAGE);
	g_option_context_add_main_entries(ctx, entries, NULL);

	group = gplugin_get_option_group();
	g_option_context_add_group(ctx, group);

	g_option_context_parse(ctx, &argc, &argv, &error);
	g_option_context_free(ctx);

	if(error) {
		fprintf(stderr, "%s\n", error->message);

		g_error_free(error);

		gplugin_uninit();

		return EXIT_FAILURE;
	}

	/* This is just for consistency, but the gplugins-option will init the
	 * library for us.
	 */
	gplugin_init(GPLUGIN_CORE_FLAGS_NONE);

	manager = gplugin_manager_get_default();

	gplugin_manager_prepend_paths_from_environment(
		manager,
		"GPLUGIN_PLUGIN_PATH");
	if(output_paths) {
		GList *path = NULL;

		for(path = gplugin_manager_get_paths(manager); path;
			path = path->next) {
			printf("%s\n", (gchar *)path->data);
		}

		exit_early = TRUE;
	}

	if(exit_early) {
		gplugin_uninit();

		return 0;
	}

	gplugin_manager_refresh(manager);

	/* check if the user gave us at least one plugin, and output them */
	if(argc > 1) {
		GQueue *plugins = g_queue_new();

		for(i = 1; i < argc; i++) {
			if(!argv[i])
				continue;

			if(strlen(argv[i]) == 0)
				continue;

			g_queue_push_tail(plugins, argv[i]);
		}

		if(!output_plugins(plugins->head))
			ret = EXIT_FAILURE;

		g_queue_free(plugins);
	} else {
		GList *plugins = gplugin_manager_list_plugins(manager);

		if(!output_plugins(plugins))
			ret = EXIT_FAILURE;

		g_list_free(plugins);
	}

	gplugin_uninit();

	return ret;
}
