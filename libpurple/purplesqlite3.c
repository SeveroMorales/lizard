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

#include <gio/gio.h>

#include "purplesqlite3.h"

/******************************************************************************
 * Helpers
 *****************************************************************************/
static gboolean
purple_sqlite3_run_migration(sqlite3 *db, int version, const char *migration,
                             GError **error)
{
	char *errmsg = NULL;
	char *str = NULL;
	gboolean success = TRUE;

	str = g_strdup_printf("BEGIN;%s;PRAGMA user_version=%d;COMMIT;", migration,
	                      version);

	sqlite3_exec(db, str, NULL, NULL, &errmsg);
	if(errmsg != NULL) {
		g_set_error(error, PURPLE_SQLITE3_DOMAIN, 0,
		            "failed to run migration: %s", errmsg);

		sqlite3_free(errmsg);

		sqlite3_exec(db, "ROLLBACK", NULL, NULL, &errmsg);
		if(errmsg != NULL) {
			g_error("failed to rollback transaction: %s", errmsg);

			sqlite3_free(errmsg);
		}

		success = FALSE;
	}

	g_free(str);

	return success;
}

/******************************************************************************
 * Public API
 *****************************************************************************/
int
purple_sqlite3_get_schema_version(sqlite3 *db, GError **error) {
	sqlite3_stmt *stmt = NULL;
	int version = -1;

	g_return_val_if_fail(db != NULL, -1);

	sqlite3_prepare_v2(db, "PRAGMA user_version", -1, &stmt, NULL);

	if(stmt == NULL) {
		g_set_error(error, PURPLE_SQLITE3_DOMAIN, 0,
		            "error while creating prepared statement: %s",
		            sqlite3_errmsg(db));

		return -1;
	}

	if(sqlite3_step(stmt) == SQLITE_ROW) {
		version = sqlite3_column_int(stmt, 0);
	} else {
		g_set_error_literal(error, PURPLE_SQLITE3_DOMAIN, 0,
		                    "'PRAGMA user_version' didn't return a row");

		sqlite3_finalize(stmt);

		return -1;
	}

	sqlite3_finalize(stmt);

	return version;
}

gboolean
purple_sqlite3_run_migrations_from_strings(sqlite3 *db,
                                           const char *migrations[],
                                           GError **error)
{
	int current_version = 0;
	guint n_migrations = 0;

	g_return_val_if_fail(db != NULL, FALSE);
	g_return_val_if_fail(migrations != NULL, FALSE);

	/* Get the current version or bail if it failed. */
	current_version = purple_sqlite3_get_schema_version(db, error);
	if(current_version == -1) {
		return FALSE;
	}

	n_migrations = g_strv_length((char **)migrations);
	if((guint)current_version > n_migrations) {
		g_set_error(error, PURPLE_SQLITE3_DOMAIN, 0,
		            "schema version %u is higher than known migrations %u",
		            (guint)current_version, n_migrations);

		return FALSE;
	}

	for(int i = current_version; migrations[i] != NULL; i++) {
		int version = i + 1;

		if(!purple_sqlite3_run_migration(db, version, migrations[i], error)) {
			return FALSE;
		}
	}

	return TRUE;
}

gboolean
purple_sqlite3_run_migrations_from_resources(sqlite3 *db, const char *path,
                                             const char *migrations[],
                                             GError **error)
{
	GError *local_error = NULL;
	int current_version = 0;
	guint n_migrations = 0;

	g_return_val_if_fail(db != NULL, FALSE);
	g_return_val_if_fail(path != NULL, FALSE);
	g_return_val_if_fail(migrations != NULL, FALSE);

	/* Get the current version or bail if it failed. */
	current_version = purple_sqlite3_get_schema_version(db, error);
	if(current_version == -1) {
		return FALSE;
	}

	n_migrations = g_strv_length((char **)migrations);
	if((guint)current_version > n_migrations) {
		g_set_error(error, PURPLE_SQLITE3_DOMAIN, 0,
		            "schema version %u is higher than known migrations %u",
		            (guint)current_version, n_migrations);

		return FALSE;
	}

	/* `PRAGMA user_version` starts at 0, so write our version as i + 1. We
	 * start iterating the list of migrations at the current version of the
	 * database. If the database is already up to date, then current_version
	 * will point us at the null terminator in the list of migrations, which
	 * will short circuit the for loop.
	 */
	for(int i = current_version; migrations[i] != NULL; i++) {
		GBytes *data = NULL;
		char *full_path = NULL;
		const gchar *migration = NULL;
		int version = i + 1;

		/* Get the data from the resource */
		full_path = g_build_path("/", path, migrations[i], NULL);

		data = g_resources_lookup_data(full_path, G_RESOURCE_LOOKUP_FLAGS_NONE,
		                               &local_error);
		if(data == NULL || local_error != NULL) {
			if(local_error == NULL) {
				local_error = g_error_new(PURPLE_SQLITE3_DOMAIN, 0,
				                          "failed to load resource %s",
				                          full_path);
			}

			g_propagate_error(error, local_error);

			g_clear_pointer(&data, g_bytes_unref);
			g_free(full_path);

			return FALSE;
		}

		g_free(full_path);

		migration = (const char *)g_bytes_get_data(data, NULL);
		if(!purple_sqlite3_run_migration(db, version, migration, error)) {
			g_bytes_unref(data);

			return FALSE;
		}

		g_bytes_unref(data);
	}

	return TRUE;
}
