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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#include <glib/gi18n-lib.h>

#include "purplebuddypresence.h"

#include "purpleprivate.h"

struct _PurpleBuddyPresence {
	PurplePresence parent;

	PurpleBuddy *buddy;

	GList *statuses;
};

enum {
	PROP_0,
	PROP_BUDDY,
	N_PROPERTIES
};
static GParamSpec *properties[N_PROPERTIES];

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
purple_buddy_presence_set_buddy(PurpleBuddyPresence *presence,
                                PurpleBuddy *buddy)
{
	if(g_set_object(&presence->buddy, buddy)) {
		g_object_notify_by_pspec(G_OBJECT(presence), properties[PROP_BUDDY]);
	}
}

static int
purple_buddy_presence_compute_score(PurpleBuddyPresence *buddy_presence)
{
	GList *l;
	int score = 0;
	PurplePresence *presence = PURPLE_PRESENCE(buddy_presence);
	PurpleBuddy *b = purple_buddy_presence_get_buddy(buddy_presence);
	int *primitive_scores = _purple_statuses_get_primitive_scores();
	int offline_score = purple_prefs_get_int("/purple/status/scores/offline_msg");
	int idle_score = purple_prefs_get_int("/purple/status/scores/idle");

	for (l = purple_presence_get_statuses(presence); l != NULL; l = l->next) {
		PurpleStatus *status = (PurpleStatus *)l->data;
		PurpleStatusType *type = purple_status_get_status_type(status);

		if (purple_status_is_active(status)) {
			score += primitive_scores[purple_status_type_get_primitive(type)];
			if (!purple_status_is_online(status)) {
				if (b && purple_account_supports_offline_message(purple_buddy_get_account(b), b))
					score += offline_score;
			}
		}
	}
	score += purple_account_get_int(purple_buddy_get_account(b), "score", 0);
	if (purple_presence_is_idle(presence))
		score += idle_score;
	return score;
}

gint
purple_buddy_presence_compare(PurpleBuddyPresence *buddy_presence1,
		PurpleBuddyPresence *buddy_presence2)
{
	PurplePresence *presence1 = PURPLE_PRESENCE(buddy_presence1);
	PurplePresence *presence2 = PURPLE_PRESENCE(buddy_presence2);
	GDateTime *now = NULL;
	GTimeSpan idle1, idle2;
	int score1 = 0, score2 = 0;
	int idle_time_score = purple_prefs_get_int("/purple/status/scores/idle_time");

	if (presence1 == presence2)
		return 0;
	else if (presence1 == NULL)
		return 1;
	else if (presence2 == NULL)
		return -1;

	if (purple_presence_is_online(presence1) &&
			!purple_presence_is_online(presence2))
		return -1;
	else if (purple_presence_is_online(presence2) &&
			!purple_presence_is_online(presence1))
		return 1;

	/* Compute the score of the first set of statuses. */
	score1 = purple_buddy_presence_compute_score(buddy_presence1);

	/* Compute the score of the second set of statuses. */
	score2 = purple_buddy_presence_compute_score(buddy_presence2);

	now = g_date_time_new_now_local();
	idle1 = g_date_time_difference(now,
	                               purple_presence_get_idle_time(presence1));
	idle2 = g_date_time_difference(now,
	                               purple_presence_get_idle_time(presence2));
	g_date_time_unref(now);

	if (idle1 > idle2) {
		score1 += idle_time_score;
	} else if (idle1 < idle2) {
		score2 += idle_time_score;
	}

	if (score1 < score2)
		return 1;
	else if (score1 > score2)
		return -1;

	return 0;
}

/******************************************************************************
 * PurplePresence Implementation
 *****************************************************************************/
static void
purple_buddy_presence_update_idle(PurplePresence *presence, gboolean old_idle)
{
	PurpleBuddy *buddy = purple_buddy_presence_get_buddy(PURPLE_BUDDY_PRESENCE(presence));
	GDateTime *current_time = g_date_time_new_now_utc();
	gboolean idle = purple_presence_is_idle(presence);

	if (old_idle != idle)
		purple_signal_emit(purple_blist_get_handle(), "buddy-idle-changed", buddy,
		                 old_idle, idle);

	purple_meta_contact_invalidate_priority_buddy(purple_buddy_get_contact(buddy));

	/* Should this be done here? It'd perhaps make more sense to
	 * connect to buddy-[un]idle signals and update from there
	 */

	purple_blist_update_node(purple_blist_get_default(),
	                         PURPLE_BLIST_NODE(buddy));

	g_date_time_unref(current_time);
}

static GList *
purple_buddy_presence_get_statuses(PurplePresence *presence) {
	PurpleBuddyPresence *buddy_presence = NULL;

	buddy_presence = PURPLE_BUDDY_PRESENCE(presence);

	/* We cache purple_protocol_get_statuses because it creates all new
	 * statuses which loses at least the active attribute, which breaks all
	 * sorts of things.
	 */
	if(buddy_presence->statuses == NULL) {
		PurpleAccount *account = NULL;

		account = purple_buddy_get_account(buddy_presence->buddy);

		buddy_presence->statuses = purple_protocol_get_statuses(account,
		                                                        presence);
	}

	return buddy_presence->statuses;
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_TYPE(PurpleBuddyPresence, purple_buddy_presence, PURPLE_TYPE_PRESENCE)

static void
purple_buddy_presence_set_property(GObject *obj, guint param_id,
                                   const GValue *value, GParamSpec *pspec)
{
	PurpleBuddyPresence *presence = PURPLE_BUDDY_PRESENCE(obj);

	switch(param_id) {
		case PROP_BUDDY:
			purple_buddy_presence_set_buddy(presence,
			                                g_value_get_object(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_buddy_presence_get_property(GObject *obj, guint param_id, GValue *value,
                                   GParamSpec *pspec)
{
	PurpleBuddyPresence *presence = PURPLE_BUDDY_PRESENCE(obj);

	switch(param_id) {
		case PROP_BUDDY:
			g_value_set_object(value,
			                   purple_buddy_presence_get_buddy(presence));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_buddy_presence_finalize(GObject *obj) {
	PurpleBuddyPresence *presence = PURPLE_BUDDY_PRESENCE(obj);

	g_clear_object(&presence->buddy);
	g_list_free_full(presence->statuses, g_object_unref);

	G_OBJECT_CLASS(purple_buddy_presence_parent_class)->finalize(obj);
}

static void
purple_buddy_presence_init(G_GNUC_UNUSED PurpleBuddyPresence *presence) {
}

static void
purple_buddy_presence_class_init(PurpleBuddyPresenceClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	PurplePresenceClass *presence_class = PURPLE_PRESENCE_CLASS(klass);

	obj_class->get_property = purple_buddy_presence_get_property;
	obj_class->set_property = purple_buddy_presence_set_property;
	obj_class->finalize = purple_buddy_presence_finalize;

	presence_class->update_idle = purple_buddy_presence_update_idle;
	presence_class->get_statuses = purple_buddy_presence_get_statuses;

	properties[PROP_BUDDY] = g_param_spec_object(
		"buddy", "Buddy",
		"The buddy for this presence.",
		PURPLE_TYPE_BUDDY,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
PurpleBuddyPresence *
purple_buddy_presence_new(PurpleBuddy *buddy) {
	g_return_val_if_fail(PURPLE_IS_BUDDY(buddy), NULL);

	return g_object_new(
		PURPLE_TYPE_BUDDY_PRESENCE,
		"buddy", buddy,
		NULL);
}

PurpleBuddy *
purple_buddy_presence_get_buddy(PurpleBuddyPresence *presence) {
	g_return_val_if_fail(PURPLE_IS_BUDDY_PRESENCE(presence), NULL);

	return presence->buddy;
}
