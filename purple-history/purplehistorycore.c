/*
 * Purple - Internet Messaging Library
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
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

#include <stdio.h>

#include <glib.h>
#include <glib/gstdio.h>
#include <glib/gi18n-lib.h>

#include <purple.h>

#define PURPLE_COMPILATION
#include "../libpurple/purpleprivate.h"
#undef PURPLE_COMPILATION

/******************************************************************************
 * Helpers
 *****************************************************************************/
static gboolean
purple_history_query(const gchar *query, GError **error) {
	PurpleHistoryManager *manager = purple_history_manager_get_default();
	GList *results = NULL;

	results = purple_history_manager_query(manager, query, error);

	if(error != NULL) {
		return FALSE;
	}

	while(results != NULL) {
		PurpleMessage *message = PURPLE_MESSAGE(results->data);

		g_printf("%s: %s\n", purple_message_get_author(message),
		         purple_message_get_contents(message));

		g_clear_object(&message);
		results = g_list_delete_link(results, results);
	}

	return TRUE;
}

G_GNUC_UNUSED
static gboolean
purple_history_remove(const gchar *query, GError **error) {
	PurpleHistoryManager *manager = purple_history_manager_get_default();
	gboolean success = FALSE;

	success = purple_history_manager_remove(manager, query, error);

	if(!success) {
		return FALSE;
	}

	if(error) {
		g_printf("Remove successful\n");
	}

	return TRUE;
}

/******************************************************************************
 * Main
 *****************************************************************************/
gint
main(gint argc, gchar *argv[]) {
	GError *error = NULL;
	GOptionContext *ctx = NULL;
	GOptionGroup *group = NULL;
	gint exit_code = EXIT_SUCCESS;

	ctx = g_option_context_new(_("QUERY"));
	g_option_context_set_help_enabled(ctx, TRUE);
	g_option_context_set_summary(ctx, _("Query purple message history"));
	g_option_context_set_translation_domain(ctx, GETTEXT_PACKAGE);

	group = purple_get_option_group();
	g_option_context_add_group(ctx, group);

	g_option_context_parse(ctx, &argc, &argv, &error);
	g_option_context_free(ctx);

	if(error != NULL) {
		g_fprintf(stderr, "%s\n", error->message);

		g_clear_error(&error);

		return EXIT_FAILURE;
	}

	purple_history_manager_startup();

	for(gint i = 1; i < argc; i++) {
		if(argv[i] == NULL || *argv[i] == '\0') {
			continue;
		}

		if(!purple_history_query(argv[i], &error)) {
			fprintf(stderr, "query failed: %s\n",
			        error ? error->message : "unknown error");

			g_clear_error(&error);

			exit_code = EXIT_FAILURE;
		}
	}

	purple_history_manager_shutdown();

	return exit_code;
}

