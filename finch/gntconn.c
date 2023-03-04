/*
 * finch
 *
 * Finch is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 */

#include <glib/gi18n-lib.h>

#include <purple.h>

#include "gntaccount.h"
#include "gntconn.h"
#include "libfinch.h"

#define INITIAL_RECON_DELAY_MIN 8
#define INITIAL_RECON_DELAY_MAX 60

#define MAX_RECON_DELAY 600

typedef struct {
	int delay;
	guint timeout;
} FinchAutoRecon;

/*
 * Contains accounts that are auto-reconnecting.
 * The key is a pointer to the PurpleAccount and the
 * value is a pointer to a FinchAutoRecon.
 */
static GHashTable *hash = NULL;

static void
free_auto_recon(gpointer data)
{
	FinchAutoRecon *info = data;

	if (info->timeout != 0)
		g_source_remove(info->timeout);

	g_free(info);
}


static gboolean
do_signon(gpointer data)
{
	PurpleAccount *account = data;
	FinchAutoRecon *info;
	PurpleStatus *status;

	purple_debug_info("autorecon", "do_signon called\n");
	g_return_val_if_fail(account != NULL, FALSE);
	info = g_hash_table_lookup(hash, account);

	if (info)
		info->timeout = 0;

	status = purple_account_get_active_status(account);
	if (purple_status_is_online(status))
	{
		purple_debug_info("autorecon", "calling purple_account_connect\n");
		purple_account_connect(account);
		purple_debug_info("autorecon", "done calling purple_account_connect\n");
	}

	return FALSE;
}

static void
finch_connection_report_disconnect(PurpleConnection *gc,
                                   PurpleConnectionError reason,
                                   G_GNUC_UNUSED const char *text)
{
	FinchAutoRecon *info;
	PurpleAccount *account = purple_connection_get_account(gc);

	if (!purple_connection_error_is_fatal(reason)) {
		info = g_hash_table_lookup(hash, account);

		if (info == NULL) {
			info = g_new0(FinchAutoRecon, 1);
			g_hash_table_insert(hash, account, info);
			info->delay = g_random_int_range(INITIAL_RECON_DELAY_MIN, INITIAL_RECON_DELAY_MAX);
		} else {
			info->delay = MIN(2 * info->delay, MAX_RECON_DELAY);
			if (info->timeout != 0)
				g_source_remove(info->timeout);
		}
		info->timeout = g_timeout_add_seconds(info->delay, do_signon, account);
	} else {
		purple_account_set_enabled(account, FALSE);
	}
}

static void
account_removed_cb(G_GNUC_UNUSED PurpleAccountManager *manager,
                   PurpleAccount *account, G_GNUC_UNUSED gpointer data)
{
	g_hash_table_remove(hash, account);
}

static PurpleConnectionUiOps ops = {
	.report_disconnect = finch_connection_report_disconnect,
};

PurpleConnectionUiOps *
finch_connections_get_ui_ops(void)
{
	return &ops;
}

void
finch_connections_init(void)
{
	PurpleAccountManager *manager = purple_account_manager_get_default();

	hash = g_hash_table_new_full(
							g_direct_hash, g_direct_equal,
							NULL, free_auto_recon);

	g_signal_connect(manager, "removed", G_CALLBACK(account_removed_cb), NULL);
}

void
finch_connections_uninit(void)
{
	PurpleAccountManager *manager = purple_account_manager_get_default();

	g_signal_handlers_disconnect_by_func(manager,
	                                     G_CALLBACK(account_removed_cb), NULL);

	g_hash_table_destroy(hash);
}
