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

#if !defined(PURPLE_GLOBAL_HEADER_INSIDE) && !defined(PURPLE_COMPILATION)
# error "only <pidgin.h> may be included directly"
#endif

#ifndef PURPLE_SQLITE3_H
#define PURPLE_SQLITE3_H

#include <glib.h>
#include <gio/gio.h>

#include <sqlite3.h>

G_BEGIN_DECLS

/**
 * PURPLE_SQLITE3_ERROR:
 *
 * An error domain for sqlite3 errors.
 *
 * Since: 3.0.0
 */
#define PURPLE_SQLITE3_DOMAIN (g_quark_from_static_string("sqlite3"))

/**
 * PurpleSqlite3:
 *
 * A sqlite3 connection.
 *
 * This type alias exists for introspection purposes, and is no different from
 * the `sqlite3` type.
 *
 * Since: 3.0.0
 */
#ifdef __GI_SCANNER__
typedef gpointer PurpleSqlite3;
#else
typedef sqlite3 PurpleSqlite3;
#endif

/**
 * purple_sqlite3_get_schema_version:
 * @db: The sqlite3 connection.
 * @error: Return address for a #GError, or %NULL.
 *
 * Attempts to read the result of `PRAGMA user_version` which this API uses to
 * store the schema version.
 *
 * Returns: %TRUE on success, or %FALSE on error with @error set.
 *
 * Since: 3.0.0
 */
int purple_sqlite3_get_schema_version(PurpleSqlite3 *db, GError **error);

/**
 * purple_sqlite3_run_migrations_from_strings:
 * @db: The sqlite3 connection.
 * @migrations: (array zero-terminated=1): A list of SQL statements, each item
 *              being its own migration.
 * @error: Return address for a #GError, or %NULL.
 *
 * Runs the given migrations in the order they are given. The index of each
 * migration plus 1 is assumed is to be the version number of the migration,
 * which means that you can not change the order of the migrations. The
 * reasoning for the addition of 1 is because `PRAGMA user_version` defaults to
 * 0.
 *
 * This expects each string in @migrations to be a complete migration. That is,
 * each string in the array should contain all of the SQL for that migration.
 * For example, if you're expecting to have 2 migrations, the initial creating
 * two tables, and then adding a column to one of the existing tables, you
 * would have something like the following code.
 *
 * ```c
 * const char *migrations[] = {
 *     // Our initial migration that creates user and session tables.
 *     "CREATE TABLE user(id INTEGER PRIMARY KEY, name TEXT);"
 *     "CREATE TABLE session(user INTEGER, token TEXT) FOREIGN KEY(user) REFERENCES user(id);",
 *     // Begin our second migration that will add a display name to the user
 *     // table. Note the ',' at the end of the previous line.
 *     "ALTER TABLE user ADD COLUMN(display_name TEXT);",
 *     NULL
 * };
 * ```
 *
 * Also, this function will run each migration in its own transaction so you
 * don't need to worry about them. This is done to make sure that the database
 * stays at a known version and an incomplete migration will not be saved.
 *
 * Returns: %TRUE on success, or %FALSE on error potentially with @error set.
 *
 * Since: 3.0.0
 */
gboolean purple_sqlite3_run_migrations_from_strings(PurpleSqlite3 *db, const char *migrations[], GError **error);

/**
 * purple_sqlite3_run_migrations_from_resources:
 * @db: The sqlite3 connection.
 * @path: The base path in @resource to use.
 * @migrations: (array zero-terminated=1): The list of migrations in the order
 *              to run them.
 * @error: Return address for a #GError, or %NULL.
 *
 * Runs the given migrations in the order they are given. The index of each
 * migration plus 1 is assumed to be the version number of the migration, which
 * means that you can not change the order of the migrations. The reasoning for
 * the addition of 1 is because `PRAGMA user_version` defaults to 0.
 *
 * This will attempt to load the migrations via
 * [func@Gio.resources_open_stream] by concatenating @path and the individual
 * items of @migrations. Each migration will be ran in a transaction that
 * includes updating the schema version, which is stored in
 * `PRAGMA user_version`. This means you can't use `PRAGMA user_version` for
 * other things.
 *
 * Returns: %TRUE on success, or %FALSE on error potentially with @error set.
 *
 * Since: 3.0.0
 */
gboolean purple_sqlite3_run_migrations_from_resources(PurpleSqlite3 *db, const char *path, const char *migrations[], GError **error);

G_END_DECLS

#endif /* PURPLE_SQLITE3_H */
