/*
 * purple
 *
 * Purple is the legal property of its developers, whose names are too numerous
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
 *
 */

#include "idle.h"

#include "connection.h"
#include "conversations.h"
#include "debug.h"
#include "eventloop.h"
#include "prefs.h"
#include "purpleaccountmanager.h"
#include "savedstatuses.h"
#include "signals.h"

typedef enum
{
	PURPLE_IDLE_NOT_AWAY = 0,
	PURPLE_IDLE_AUTO_AWAY,
	PURPLE_IDLE_AWAY_BUT_NOT_AUTO_AWAY

} PurpleAutoAwayState;

static PurpleIdleUi *idle_ui = NULL;

/*
 * This is needed for the I'dle Mak'er plugin to work correctly.  We
 * use it to determine if we're the ones who set our accounts idle
 * or if someone else did it (the I'dle Mak'er plugin, for example).
 * Basically we just keep track of which accounts were set idle by us,
 * and then we'll only set these specific accounts unidle when the
 * user returns.
 */
static GList *idled_accts = NULL;

static guint idle_timer = 0;

static time_t last_active_time = 0;

static void
set_account_idle(PurpleAccount *account, int time_idle)
{
	PurpleContactInfo *info = PURPLE_CONTACT_INFO(account);
	PurplePresence *presence;
	GDateTime *idle_since = NULL;
	GDateTime *now = NULL;

	presence = purple_account_get_presence(account);

	if(purple_presence_is_idle(presence)) {
		/* This account is already idle! */
		return;
	}

	purple_debug_info("idle", "Setting %s idle %d seconds\n",
	                  purple_contact_info_get_username(info),
	                  time_idle);

	now = g_date_time_new_now_local();
	idle_since = g_date_time_add_seconds(now, -1 * time_idle);
	g_date_time_unref(now);

	purple_presence_set_idle(presence, TRUE, idle_since);
	g_date_time_unref(idle_since);

	idled_accts = g_list_prepend(idled_accts, account);
}

static void
set_account_unidle(PurpleAccount *account)
{
	PurpleContactInfo *info = PURPLE_CONTACT_INFO(account);
	PurplePresence *presence;

	presence = purple_account_get_presence(account);

	idled_accts = g_list_remove(idled_accts, account);

	if(!purple_presence_is_idle(presence)) {
		/* This account is already unidle! */
		return;
	}

	purple_debug_info("idle", "Setting %s unidle\n",
	                  purple_contact_info_get_username(info));
	purple_presence_set_idle(presence, FALSE, 0);
}


static gboolean no_away = FALSE;
static gint time_until_next_idle_event;
/*
 * This function should be called when you think your idle state
 * may have changed.  Maybe you're over the 10-minute mark and
 * Purple should start reporting idle time to the server.  Maybe
 * you've returned from being idle.  Maybe your auto-away message
 * should be set.
 *
 * There is no harm to calling this many many times, other than
 * it will be kinda slow.  This is called by a timer set when
 * Purple starts.  It is also called when you send an IM, a chat, etc.
 *
 * This function has 3 sections.
 * 1. Get your idle time.  It will query XScreenSaver or Windows
 *    or use the Purple idle time.  Whatever.
 * 2. Set or unset your auto-away message.
 * 3. Report your current idle time to the IM server.
 */

static void
check_idleness(void)
{
	time_t time_idle = 0;
	gboolean auto_away;
	const gchar *idle_reporting;
	gboolean report_idle = TRUE;
	gint away_seconds = 0;
	gint idle_recheck_interval = 0;
	gint idle_poll_seconds = purple_prefs_get_int("/purple/away/mins_before_away") * 60;
	gboolean set = FALSE;

	purple_signal_emit(purple_blist_get_handle(), "update-idle");

	idle_reporting = purple_prefs_get_string("/purple/away/idle_reporting");
	auto_away = purple_prefs_get_bool("/purple/away/away_when_idle");

	if (purple_strequal(idle_reporting, "system") &&
		PURPLE_IS_IDLE_UI(idle_ui))
	{
		time_t new_idle = purple_idle_ui_get_idle_time(idle_ui);

		if(new_idle > 0) {
			/* Use system idle time (mouse or keyboard movement, etc.) */
			time_idle = new_idle;
			idle_recheck_interval = 1;
			set = TRUE;
		}
	}

	if(!set && purple_strequal(idle_reporting, "purple")) {
		/* Use 'Purple idle' */
		time_idle = time(NULL) - last_active_time;
		idle_recheck_interval = 0;
	} else {
		/* Don't report idle time */
		report_idle = FALSE;

		/* If we're not reporting idle, we can still do auto-away.
		 * First try "system" and if that isn't possible, use "purple" */
		if(auto_away) {
			if(PURPLE_IS_IDLE_UI(idle_ui)) {
				time_t new_idle = purple_idle_ui_get_idle_time(idle_ui);

				if(new_idle > 0) {
					time_idle = new_idle;
					idle_recheck_interval = 1;

					set = TRUE;
				}
			}

			if(!set) {
				time_idle = time(NULL) - last_active_time;
				idle_recheck_interval = 0;
			}
		} else {
			if(!no_away) {
				no_away = TRUE;
				purple_savedstatus_set_idleaway(FALSE);
			}
			time_until_next_idle_event = 0;
			return;
		}
	}

	time_until_next_idle_event = idle_poll_seconds - time_idle;
	if (time_until_next_idle_event < 0)
	{
		/* If we're already idle, check again as appropriate. */
		time_until_next_idle_event = idle_recheck_interval;
	}

	if (auto_away || !no_away)
		away_seconds = 60 * purple_prefs_get_int("/purple/away/mins_before_away");

	if (auto_away && time_idle > away_seconds)
	{
		purple_savedstatus_set_idleaway(TRUE);
		no_away = FALSE;
	}
	else if (purple_savedstatus_is_idleaway() && time_idle < away_seconds)
	{
		purple_savedstatus_set_idleaway(FALSE);
		if (time_until_next_idle_event == 0 || (away_seconds - time_idle) < time_until_next_idle_event)
			time_until_next_idle_event = away_seconds - time_idle;
	}

	/* Idle reporting stuff */
	if(report_idle && (time_idle >= idle_poll_seconds)) {
		PurpleAccountManager *manager = NULL;
		GList *accounts = NULL;

		manager = purple_account_manager_get_default();
		accounts = purple_account_manager_get_connected(manager);
		while(accounts != NULL) {
			set_account_idle(accounts->data, time_idle);

			accounts = g_list_delete_link(accounts, accounts);
		}
	}
	else if (!report_idle || (time_idle < idle_poll_seconds ))
	{
		while (idled_accts != NULL)
			set_account_unidle(idled_accts->data);
	}
}

/*
 * Check idle and set the timer to fire at the next idle-worth event
 */
static gboolean
check_idleness_timer(G_GNUC_UNUSED gpointer data) {
	check_idleness();
	if (time_until_next_idle_event == 0) {
		idle_timer = 0;
	} else {
		/* +1 for the boundary,
		 * +1 more for g_timeout_add_seconds rounding. */
		idle_timer = g_timeout_add_seconds(time_until_next_idle_event + 2,
		                                   G_SOURCE_FUNC(check_idleness_timer),
		                                   NULL);
	}
	return G_SOURCE_REMOVE;
}

static void
im_msg_sent_cb(G_GNUC_UNUSED PurpleAccount *account,
               G_GNUC_UNUSED PurpleMessage *msg, G_GNUC_UNUSED gpointer data)
{
	/* Check our idle time after an IM is sent */
	check_idleness();
}

static void
signing_on_cb(G_GNUC_UNUSED PurpleConnection *connection,
              G_GNUC_UNUSED gpointer data)
{
	/* When signing on a new account, check if the account should be idle */
	check_idleness();
}

static void
signing_off_cb(PurpleConnection *gc, G_GNUC_UNUSED gpointer data)
{
	PurpleAccount *account;

	account = purple_connection_get_account(gc);
	set_account_unidle(account);
}

static void
idle_reporting_cb(G_GNUC_UNUSED const char *name,
                  G_GNUC_UNUSED PurplePrefType type,
                  G_GNUC_UNUSED gconstpointer val, G_GNUC_UNUSED gpointer data)
{
	if (idle_timer)
		g_source_remove(idle_timer);
	idle_timer = 0;
	check_idleness_timer(NULL);
}

void
purple_idle_touch(void)
{
	time(&last_active_time);
	if (!no_away)
	{
		if (idle_timer)
			g_source_remove(idle_timer);
		idle_timer = 0;
		check_idleness_timer(NULL);
	}
}

void
purple_idle_set(time_t time)
{
	last_active_time = time;
}

void
purple_idle_set_ui(PurpleIdleUi *ui) {
	g_return_if_fail(ui == NULL || PURPLE_IS_IDLE_UI(ui));

	g_clear_object(&idle_ui);
	idle_ui = ui;
}

PurpleIdleUi *
purple_idle_get_ui(void) {
	return idle_ui;
}

static void *
purple_idle_get_handle(void)
{
	static int handle;

	return &handle;
}

static gboolean
_do_purple_idle_touch_cb(G_GNUC_UNUSED gpointer data)
{
	int idle_poll_minutes = purple_prefs_get_int("/purple/away/mins_before_away");

	 /* +1 more for g_timeout_add_seconds rounding. */
	idle_timer = g_timeout_add_seconds((idle_poll_minutes * 60) + 2,
	                                   G_SOURCE_FUNC(check_idleness_timer),
	                                   NULL);

	purple_idle_touch();

	return FALSE;
}


void
purple_idle_init(void)
{
	purple_signal_connect(purple_conversations_get_handle(), "sent-im-msg",
						purple_idle_get_handle(),
						G_CALLBACK(im_msg_sent_cb), NULL);
	purple_signal_connect(purple_connections_get_handle(), "signing-on",
						purple_idle_get_handle(),
						G_CALLBACK(signing_on_cb), NULL);
	purple_signal_connect(purple_connections_get_handle(), "signing-off",
						purple_idle_get_handle(),
						G_CALLBACK(signing_off_cb), NULL);

	purple_prefs_connect_callback(purple_idle_get_handle(), "/purple/away/idle_reporting",
	                              idle_reporting_cb, NULL);

	/* Initialize the idleness asynchronously so it doesn't check idleness,
	 * and potentially try to change the status before the UI is initialized */
	g_timeout_add(0, _do_purple_idle_touch_cb, NULL);

}

void
purple_idle_uninit(void)
{
	purple_signals_disconnect_by_handle(purple_idle_get_handle());
	purple_prefs_disconnect_by_handle(purple_idle_get_handle());

	/* Remove the idle timer */
	if (idle_timer > 0)
		g_source_remove(idle_timer);
	idle_timer = 0;

	g_clear_object(&idle_ui);
}
