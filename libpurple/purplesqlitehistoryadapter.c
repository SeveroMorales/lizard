/*
 * Purple - Internet Messaging Library
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
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

#include <glib/gi18n-lib.h>

#include <sqlite3.h>

#include "purplesqlitehistoryadapter.h"

#include "account.h"
#include "purpleprivate.h"
#include "purplesqlite3.h"

struct _PurpleSqliteHistoryAdapter {
	PurpleHistoryAdapter parent;

	gchar *filename;
	sqlite3 *db;
};

enum {
	PROP_0,
	PROP_FILENAME,
	N_PROPERTIES,
};
static GParamSpec *properties[N_PROPERTIES] = {NULL, };

G_DEFINE_TYPE(PurpleSqliteHistoryAdapter, purple_sqlite_history_adapter,
              PURPLE_TYPE_HISTORY_ADAPTER)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
purple_sqlite_history_adapter_set_filename(PurpleSqliteHistoryAdapter *adapter,
                                           const gchar *filename)
{
	g_free(adapter->filename);
	adapter->filename = g_strdup(filename);

	g_object_notify_by_pspec(G_OBJECT(adapter), properties[PROP_FILENAME]);
}

static gboolean
purple_sqlite_history_adapter_run_migrations(PurpleSqliteHistoryAdapter *adapter,
                                             GError **error)
{
	const char *path = "/im/pidgin/libpurple/sqlitehistoryadapter";
	const char *migrations[] = {
		"01-schema.sql",
		NULL
	};

	return purple_sqlite3_run_migrations_from_resources(adapter->db, path,
	                                                    migrations, error);
}

static gchar *
purple_sqlite_history_adapter_get_content_type(PurpleMessageContentType content_type) {
	switch(content_type) {
		case PURPLE_MESSAGE_CONTENT_TYPE_PLAIN:
			return "plain";
			break;
		case PURPLE_MESSAGE_CONTENT_TYPE_HTML:
			return "html";
			break;
		case PURPLE_MESSAGE_CONTENT_TYPE_XHTML:
			return "xhtml";
			break;
		case PURPLE_MESSAGE_CONTENT_TYPE_MARKDOWN:
			return "markdown";
			break;
		default:
			return "";
			break;
	}
}

static PurpleMessageContentType
purple_sqlite_history_adapter_get_content_type_enum(const gchar *content_type)
{
	if(purple_strequal(content_type, "plain")) {
		return PURPLE_MESSAGE_CONTENT_TYPE_PLAIN;
	}
	if(purple_strequal(content_type, "html")) {
		return PURPLE_MESSAGE_CONTENT_TYPE_HTML;
	}
	if(purple_strequal(content_type, "xhtml")) {
		return PURPLE_MESSAGE_CONTENT_TYPE_XHTML;
	}
	if(purple_strequal(content_type, "markdown")) {
		return PURPLE_MESSAGE_CONTENT_TYPE_MARKDOWN;
	}
	return PURPLE_MESSAGE_CONTENT_TYPE_PLAIN;
}

static sqlite3_stmt *
purple_sqlite_history_adapter_build_query(PurpleSqliteHistoryAdapter *adapter,
                                          const gchar * search_query,
                                          gboolean remove,
                                          GError **error)
{
	gchar **split = NULL;
	gint i = 0;
	GList *ins = NULL;
	GList *froms = NULL;
	GList *keywords = NULL;
	GString *query = NULL;
	GList *iter = NULL;
	gboolean first = FALSE;
	sqlite3_stmt *prepared_statement = NULL;
	gint index = 1;
	gint query_items = 0;

	split = g_strsplit(search_query, " ", -1);
	for(i = 0; split[i] != NULL; i++) {
		if(g_str_has_prefix(split[i], "in:")) {
			if(split[i][3] == '\0') {
				continue;
			}
			ins = g_list_prepend(ins, g_strdup(split[i]+3));
			query_items++;
		} else if(g_str_has_prefix(split[i], "from:")) {
			if(split[i][5] == '\0') {
				continue;
			}
			froms = g_list_prepend(froms, g_strdup(split[i]+5));
			query_items++;
		} else {
			if(split[i][0] == '\0') {
				continue;
			}
			keywords = g_list_prepend(keywords,
			                          g_strdup_printf("%%%s%%", split[i]));
			query_items++;
		}
	}

	g_clear_pointer(&split, g_strfreev);

	if(remove) {
		if(query_items != 0) {
			query = g_string_new("DELETE FROM message_log WHERE TRUE\n");
		} else {
			g_set_error(error, PURPLE_HISTORY_ADAPTER_DOMAIN, 0,
			            "Attempting to remove messages without "
			            "query parameters.");

			return NULL;
		}
	} else {
		query = g_string_new("SELECT "
		                     "message_id, author, author_name_color, "
		                     "author_alias, recipient, content_type, "
		                     "content, client_timestamp "
		                     "FROM message_log WHERE TRUE\n");
	}

	if(ins != NULL) {
		first = TRUE;
		g_string_append(query, "AND (conversation_id IN (");
		for(iter = ins; iter != NULL; iter = iter->next) {
			if(!first) {
				g_string_append(query, ", ");
			}
			first = FALSE;
			g_string_append(query, "?");
		}
		g_string_append(query, "))");
	}

	if(froms != NULL) {
		first = TRUE;
		g_string_append(query, "AND (author IN (");
		for(iter = froms; iter != NULL; iter = iter->next) {
			if(!first) {
				g_string_append(query, ", ");
			}
			first = FALSE;
			g_string_append(query, "?");
		}
		g_string_append(query, "))");
	}

	if(keywords != NULL) {
		first = TRUE;
		g_string_append(query, "AND (");
		for(iter = keywords; iter != NULL; iter = iter->next) {
			if(!first) {
				g_string_append(query, " OR ");
			}
			first = FALSE;
			g_string_append(query, " content LIKE ? ");
		}
		g_string_append(query, ")");
	}
	g_string_append(query, ";");

	sqlite3_prepare_v2(adapter->db, query->str, -1, &prepared_statement, NULL);

	g_string_free(query, TRUE);

	if(prepared_statement == NULL) {
		g_set_error(error, PURPLE_HISTORY_ADAPTER_DOMAIN, 0,
		            "Error creating the prepared statement: %s",
		            sqlite3_errmsg(adapter->db));

		g_list_free_full(ins, g_free);
		g_list_free_full(froms, g_free);
		g_list_free_full(keywords, g_free);

		return NULL;
	}

	while(ins != NULL) {
		sqlite3_bind_text(prepared_statement, index++,
		                  (const char *)ins->data, -1, g_free);
		ins = g_list_delete_link(ins, ins);
	}

	while(froms != NULL) {
		sqlite3_bind_text(prepared_statement, index++,
		                  (const char *)froms->data, -1, g_free);
		froms = g_list_delete_link(froms, froms);
	}

	while(keywords != NULL) {
		sqlite3_bind_text(prepared_statement, index++,
		                  (const char *)keywords->data, -1, g_free);
		keywords = g_list_delete_link(keywords, keywords);
	}

	return prepared_statement;
}

/******************************************************************************
 * PurpleHistoryAdapter Implementation
 *****************************************************************************/
static gboolean
purple_sqlite_history_adapter_activate(PurpleHistoryAdapter *adapter,
                                       GError **error)
{
	PurpleSqliteHistoryAdapter *sqlite_adapter = NULL;
	gint rc = 0;

	sqlite_adapter = PURPLE_SQLITE_HISTORY_ADAPTER(adapter);

	if(sqlite_adapter->db != NULL) {
		g_set_error_literal(error, PURPLE_HISTORY_ADAPTER_DOMAIN, 0,
		                    _("Adapter has already been activated"));

		return FALSE;
	}

	if(sqlite_adapter->filename == NULL) {
		g_set_error_literal(error, PURPLE_HISTORY_ADAPTER_DOMAIN, 0,
		                    _("No filename specified"));

		return FALSE;
	}

	rc = sqlite3_open(sqlite_adapter->filename, &sqlite_adapter->db);
	if(rc != SQLITE_OK) {
		g_set_error(error, PURPLE_HISTORY_ADAPTER_DOMAIN, 0,
		            _("Error opening database in purplesqlitehistoryadapter for file %s"),
		            sqlite_adapter->filename);
		g_clear_pointer(&sqlite_adapter->db, sqlite3_close);

		return FALSE;
	}

	if(!purple_sqlite_history_adapter_run_migrations(sqlite_adapter, error)) {
		g_clear_pointer(&sqlite_adapter->db, sqlite3_close);

		return FALSE;
	}

	return TRUE;
}

static gboolean
purple_sqlite_history_adapter_deactivate(PurpleHistoryAdapter *adapter,
                                         G_GNUC_UNUSED GError **error)
{
	PurpleSqliteHistoryAdapter *sqlite_adapter = NULL;

	sqlite_adapter = PURPLE_SQLITE_HISTORY_ADAPTER(adapter);
	g_clear_pointer(&sqlite_adapter->db, sqlite3_close);

	return TRUE;
}

static GList*
purple_sqlite_history_adapter_query(PurpleHistoryAdapter *adapter,
                                    const gchar *query, GError **error)
{
	PurpleSqliteHistoryAdapter *sqlite_adapter = NULL;
	sqlite3_stmt *prepared_statement = NULL;
	GList *results = NULL;

	sqlite_adapter = PURPLE_SQLITE_HISTORY_ADAPTER(adapter);

	if(sqlite_adapter->db == NULL) {
		g_set_error_literal(error, PURPLE_HISTORY_ADAPTER_DOMAIN, 0,
		                    _("Adapter has not been activated"));

		return FALSE;
	}

	prepared_statement = purple_sqlite_history_adapter_build_query(sqlite_adapter,
	                                                               query,
	                                                               FALSE,
	                                                               error);

	if(prepared_statement == NULL) {
		return NULL;
	}

	while(sqlite3_step(prepared_statement) == SQLITE_ROW) {
		PurpleMessage *message = NULL;
		PurpleMessageContentType ct;
		GDateTime *g_date_time = NULL;
		const gchar *message_id = NULL;
		const gchar *author = NULL;
		const gchar *author_name_color = NULL;
		const gchar *author_alias = NULL;
		const gchar *recipient = NULL;
		const gchar *content = NULL;
		const gchar *content_type = NULL;
		const gchar *timestamp = NULL;

		message_id = (const gchar *)sqlite3_column_text(prepared_statement, 0);
		author = (const gchar *)sqlite3_column_text(prepared_statement, 1);
		author_name_color = (const gchar *)sqlite3_column_text(prepared_statement, 2);
		author_alias = (const gchar *)sqlite3_column_text(prepared_statement, 3);
		recipient = (const gchar *)sqlite3_column_text(prepared_statement, 4);
		content_type = (const gchar *)sqlite3_column_text(prepared_statement, 5);
		ct = purple_sqlite_history_adapter_get_content_type_enum(content_type);
		content = (const gchar *)sqlite3_column_text(prepared_statement, 6);
		timestamp = (const gchar *)sqlite3_column_text(prepared_statement, 7);
		g_date_time = g_date_time_new_from_iso8601(timestamp, NULL);

		message = g_object_new(PURPLE_TYPE_MESSAGE,
		                       "id", message_id,
		                       "author", author,
		                       "author_name_color", author_name_color,
		                       "author_alias", author_alias,
		                       "recipient", recipient,
		                       "contents", content,
		                       "content_type", ct,
		                       "timestamp", g_date_time,
		                       NULL);

		results = g_list_prepend(results, message);
	}

	results = g_list_reverse(results);

	sqlite3_finalize(prepared_statement);

	return results;
}

static gboolean
purple_sqlite_history_adapter_remove(PurpleHistoryAdapter *adapter,
                                     const gchar *query, GError **error)
{
	PurpleSqliteHistoryAdapter *sqlite_adapter = NULL;
	sqlite3_stmt * prepared_statement = NULL;
	gint result = 0;

	sqlite_adapter = PURPLE_SQLITE_HISTORY_ADAPTER(adapter);

	if(sqlite_adapter->db == NULL) {
		g_set_error_literal(error, PURPLE_HISTORY_ADAPTER_DOMAIN, 0,
		                    _("Adapter has not been activated"));

		return FALSE;
	}

	prepared_statement = purple_sqlite_history_adapter_build_query(sqlite_adapter,
	                                                               query,
	                                                               TRUE,
	                                                               error);

	if(prepared_statement == NULL) {
		return FALSE;
	}

	result = sqlite3_step(prepared_statement);

	if(result != SQLITE_DONE) {
		g_set_error(error, PURPLE_HISTORY_ADAPTER_DOMAIN, 0,
		            "Error removing from the database: %s",
		            sqlite3_errmsg(sqlite_adapter->db));

		sqlite3_finalize(prepared_statement);

		return FALSE;
	}

	sqlite3_finalize(prepared_statement);

	return TRUE;
}

static gboolean
purple_sqlite_history_adapter_write(PurpleHistoryAdapter *adapter,
                                    PurpleConversation *conversation,
                                    PurpleMessage *message, GError **error)
{
	PurpleAccount *account = NULL;
	PurpleContactInfo *info = NULL;
	PurpleSqliteHistoryAdapter *sqlite_adapter = NULL;
	sqlite3_stmt *prepared_statement = NULL;
	gchar *timestamp = NULL;
	gchar *content_type = NULL;
	const gchar * message_id = NULL;
	const gchar *script = NULL;
	gint result = 0;

	script = "INSERT INTO message_log(protocol, account, conversation_id, "
			 "message_id, author, author_name_color, author_alias, "
			 "recipient, content_type, content, client_timestamp) "
	         "VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

	sqlite_adapter = PURPLE_SQLITE_HISTORY_ADAPTER(adapter);

	if(sqlite_adapter->db == NULL) {
		g_set_error_literal(error, PURPLE_HISTORY_ADAPTER_DOMAIN, 0,
		                    _("Adapter has not been activated"));

		return FALSE;
	}

	sqlite3_prepare_v2(sqlite_adapter->db, script, -1, &prepared_statement, NULL);

	if(prepared_statement == NULL) {
		g_set_error(error, PURPLE_HISTORY_ADAPTER_DOMAIN, 0,
		            "Error creating the prepared statement: %s",
		            sqlite3_errmsg(sqlite_adapter->db));
		return FALSE;
	}

	account = purple_conversation_get_account(conversation);
	info = PURPLE_CONTACT_INFO(account);

	sqlite3_bind_text(prepared_statement,
	                  1, purple_account_get_protocol_name(account), -1,
	                  SQLITE_STATIC);
	sqlite3_bind_text(prepared_statement,
	                  2, purple_contact_info_get_username(info), -1,
	                  SQLITE_STATIC);
	sqlite3_bind_text(prepared_statement,
	                  3, purple_conversation_get_name(conversation), -1,
	                  SQLITE_STATIC);
	message_id = purple_message_get_id(message);
	if(message_id != NULL) {
		sqlite3_bind_text(prepared_statement, 4, message_id, -1,
		                  SQLITE_STATIC);
	} else {
		sqlite3_bind_text(prepared_statement, 4, g_uuid_string_random(), -1,
		                  g_free);
	}
	sqlite3_bind_text(prepared_statement,
	                  5, purple_message_get_author(message), -1,
	                  SQLITE_STATIC);
	sqlite3_bind_text(prepared_statement,
	                  6, purple_message_get_author_name_color(message), -1,
	                  SQLITE_STATIC);
	sqlite3_bind_text(prepared_statement,
	                  7, purple_message_get_author_alias(message), -1,
	                  SQLITE_STATIC);
	sqlite3_bind_text(prepared_statement,
	                  8, purple_message_get_recipient(message), -1,
	                  SQLITE_STATIC);
	content_type = purple_sqlite_history_adapter_get_content_type(purple_message_get_content_type(message));
	sqlite3_bind_text(prepared_statement,
	                  9, content_type, -1, SQLITE_STATIC);
	sqlite3_bind_text(prepared_statement,
	                  10, purple_message_get_contents(message), -1,
	                  SQLITE_STATIC);
	timestamp = g_date_time_format_iso8601(purple_message_get_timestamp(message));
	sqlite3_bind_text(prepared_statement, 11, timestamp, -1, g_free);

	result = sqlite3_step(prepared_statement);

	if(result != SQLITE_DONE) {
		g_set_error(error, PURPLE_HISTORY_ADAPTER_DOMAIN, 0,
		            "Error writing to the database: %s",
		            sqlite3_errmsg(sqlite_adapter->db));

		sqlite3_finalize(prepared_statement);

		return FALSE;
	}

	sqlite3_finalize(prepared_statement);

	return TRUE;
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
purple_sqlite_history_adapter_get_property(GObject *obj, guint param_id,
                                           GValue *value, GParamSpec *pspec)
{
	PurpleSqliteHistoryAdapter *adapter = PURPLE_SQLITE_HISTORY_ADAPTER(obj);

	switch(param_id) {
		case PROP_FILENAME:
			g_value_set_string(value,
			                   purple_sqlite_history_adapter_get_filename(adapter));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_sqlite_history_adapter_set_property(GObject *obj, guint param_id,
                                           const GValue *value,
                                           GParamSpec *pspec)
{
	PurpleSqliteHistoryAdapter *adapter = PURPLE_SQLITE_HISTORY_ADAPTER(obj);

	switch(param_id) {
		case PROP_FILENAME:
			purple_sqlite_history_adapter_set_filename(adapter,
			                                           g_value_get_string(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_sqlite_history_adapter_finalize(GObject *obj) {
	PurpleSqliteHistoryAdapter *adapter = NULL;

	adapter = PURPLE_SQLITE_HISTORY_ADAPTER(obj);

	g_clear_pointer(&adapter->filename, g_free);

	if(adapter->db != NULL) {
		g_warning("PurpleSqliteHistoryAdapter was finalized before being "
		          "deactivated");

		g_clear_pointer(&adapter->db, sqlite3_close);
	}

	G_OBJECT_CLASS(purple_sqlite_history_adapter_parent_class)->finalize(obj);
}

static void
purple_sqlite_history_adapter_init(G_GNUC_UNUSED PurpleSqliteHistoryAdapter *adapter)
{
}

static void
purple_sqlite_history_adapter_class_init(PurpleSqliteHistoryAdapterClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	PurpleHistoryAdapterClass *adapter_class = PURPLE_HISTORY_ADAPTER_CLASS(klass);

	obj_class->get_property = purple_sqlite_history_adapter_get_property;
	obj_class->set_property = purple_sqlite_history_adapter_set_property;
	obj_class->finalize = purple_sqlite_history_adapter_finalize;

	adapter_class->activate = purple_sqlite_history_adapter_activate;
	adapter_class->deactivate = purple_sqlite_history_adapter_deactivate;
	adapter_class->query = purple_sqlite_history_adapter_query;
	adapter_class->remove = purple_sqlite_history_adapter_remove;
	adapter_class->write = purple_sqlite_history_adapter_write;

	/**
	 * PurpleHistoryAdapter::filename:
	 *
	 * The filename that the sqlite database will store data to.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_FILENAME] = g_param_spec_string(
		"filename", "filename", "The filename of the sqlite database",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS
	);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
PurpleHistoryAdapter *
purple_sqlite_history_adapter_new(const gchar *filename) {
	return g_object_new(
		PURPLE_TYPE_SQLITE_HISTORY_ADAPTER,
		"filename", filename,
		"id", "sqlite-adapter",
		"name", N_("SQLite Adapter"),
		NULL);
}

const gchar *
purple_sqlite_history_adapter_get_filename(PurpleSqliteHistoryAdapter *adapter)
{
	PurpleSqliteHistoryAdapter *sqlite_adapter = NULL;

	g_return_val_if_fail(PURPLE_IS_SQLITE_HISTORY_ADAPTER(adapter), NULL);

	sqlite_adapter = PURPLE_SQLITE_HISTORY_ADAPTER(adapter);

	return sqlite_adapter->filename;
}
