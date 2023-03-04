/*
 * Purple - Internet Messaging Library
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
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

#if !defined(PURPLE_GLOBAL_HEADER_INSIDE) && !defined(PURPLE_COMPILATION)
# error "only <purple.h> may be included directly"
#endif

#ifndef PURPLE_PRESENCE_H
#define PURPLE_PRESENCE_H

#include <glib.h>
#include <glib-object.h>

/**
 * PurplePresence:
 *
 * A PurplePresence is like a collection of PurpleStatuses (plus some other
 * random info).  For any buddy, or for any one of your accounts, or for any
 * person with which you're chatting, you may know various amounts of
 * information.  This information is all contained in one PurplePresence.  If
 * one of your buddies is away and idle, then the presence contains the
 * #PurpleStatus for their awayness, and it contains their current idle time.
 * #PurplePresence's are never saved to disk.  The information they contain is
 * only relevant for the current Purple session.
 *
 * Note: When a presence is destroyed with the last g_object_unref(), all
 * statuses added to this list will be destroyed along with the presence.
 */

typedef struct _PurplePresence PurplePresence;

#include "status.h"

/**
 * PurplePresencePrimitive:
 * @PURPLE_PRESENCE_PRIMITIVE_OFFLINE: The presence is offline or otherwise
 *                                     unknown.
 * @PURPLE_PRESENCE_PRIMITIVE_AVAILABLE: The presence is online or available.
 * @PURPLE_PRESENCE_PRIMITIVE_IDLE: The presence is online but is idle. This
 *                                  state is typically set by the client
 *                                  directly and not the user.
 * @PURPLE_PRESENCE_PRIMITIVE_INVISIBLE: The presence is online, but not
 *                                       visible to others.
 * @PURPLE_PRESENCE_PRIMITIVE_AWAY: The presence is online, but the user is
 *                                  away from their device.
 * @PURPLE_PRESENCE_PRIMITIVE_EXTENDED_AWAY: Similar to
 *                                           @PURPLE_PRESENCE_PRIMITIVE_AWAY,
 *                                           but typically means the user does
 *                                           not want to be disturbed.
 * @PURPLE_PRESENCE_PRIMITIVE_STREAMING: The presence is online but is
 *                                       streaming.
 *
 * An enum that is used to determine the type of a [class@Purple.Presence].
 *
 * Since: 3.0.0
 */
typedef enum {
	PURPLE_PRESENCE_PRIMITIVE_OFFLINE,
	PURPLE_PRESENCE_PRIMITIVE_AVAILABLE,
	PURPLE_PRESENCE_PRIMITIVE_IDLE,
	PURPLE_PRESENCE_PRIMITIVE_INVISIBLE,
	PURPLE_PRESENCE_PRIMITIVE_AWAY,
	PURPLE_PRESENCE_PRIMITIVE_EXTENDED_AWAY,
	PURPLE_PRESENCE_PRIMITIVE_STREAMING,
} PurplePresencePrimitive;

G_BEGIN_DECLS

/**
 * PurplePresenceClass:
 * @update_idle: Updates the logs and the UI when the idle state or time of the
 *               presence changes.
 *
 * The base class for all #PurplePresence's.
 */
struct _PurplePresenceClass {
	/*< private >*/
	GObjectClass parent;

	/*< public >*/
	void (*update_idle)(PurplePresence *presence, gboolean old_idle);
	GList *(*get_statuses)(PurplePresence *presence);

	/*< private >*/
	gpointer reserved[4];
};

#define PURPLE_TYPE_PRESENCE purple_presence_get_type()
G_DECLARE_DERIVABLE_TYPE(PurplePresence, purple_presence, PURPLE, PRESENCE,
                         GObject)

/**
 * purple_presence_new:
 *
 * Creates a new presence instance.
 *
 * Returns: (transfer full): The new instance.
 *
 * Since: 3.0.0
 */
PurplePresence *purple_presence_new(void);

/**
 * purple_presence_set_status_active:
 * @presence: The #PurplePresence instance.
 * @status_id: The ID of the status.
 * @active: The active state.
 *
 * Sets the active state of a status in a presence.
 *
 * Only independent statuses can be set inactive. Normal statuses can only
 * be set active, so if you wish to disable a status, set another
 * non-independent status to active, or use purple_presence_switch_status().
 */
void purple_presence_set_status_active(PurplePresence *presence, const gchar *status_id, gboolean active);

/**
 * purple_presence_switch_status:
 * @presence: The #PurplePresence instance.
 * @status_id: The status ID to switch to.
 *
 * Switches the active status in a presence.
 *
 * This is similar to purple_presence_set_status_active(), except it won't
 * activate independent statuses.
 */
void purple_presence_switch_status(PurplePresence *presence, const gchar *status_id);

/**
 * purple_presence_set_idle:
 * @presence: The #PurplePresence instance.
 * @idle: The idle state.
 * @idle_time: (transfer none): The idle time, if @idle is %TRUE.  This is the
 *             time at which the user became idle. If this value is unknown
 *             then %NULL should be used.
 *
 * Sets the idle state and time of @presence.
 */
void purple_presence_set_idle(PurplePresence *presence, gboolean idle, GDateTime *idle_time);

/**
 * purple_presence_set_login_time:
 * @presence: The #PurplePresence instance.
 * @login_time: (transfer none): The login time.
 *
 * Sets the login time on a presence.
 */
void purple_presence_set_login_time(PurplePresence *presence, GDateTime *login_time);

/**
 * purple_presence_get_statuses:
 * @presence: The #PurplePresence instance.
 *
 * Gets a list of all the statuses in @presence.
 *
 * Returns: (element-type PurpleStatus) (transfer none): The statuses.
 */
GList *purple_presence_get_statuses(PurplePresence *presence);

/**
 * purple_presence_get_status:
 * @presence: The #PurplePresence instance.
 * @status_id: The ID of the status.
 *
 * Gets the status with the specified ID from @presence.
 *
 * Returns: (transfer none): The #PurpleStatus if found, or %NULL.
 */
PurpleStatus *purple_presence_get_status(PurplePresence *presence, const gchar *status_id);

/**
 * purple_presence_get_active_status:
 * @presence: The #PurplePresence instance.
 *
 * Gets the active exclusive status from @presence.
 *
 * Returns: (transfer none): The active exclusive status.
 */
PurpleStatus *purple_presence_get_active_status(PurplePresence *presence);

/**
 * purple_presence_is_available:
 * @presence: The #PurplePresence instance.
 *
 * Gets whether or not @presence is available.
 *
 * Available presences are online and possibly invisible, but not away or idle.
 *
 * Returns: %TRUE if the presence is available, or %FALSE otherwise.
 */
gboolean purple_presence_is_available(PurplePresence *presence);

/**
 * purple_presence_is_online:
 * @presence: The #PurplePresence instance.
 *
 * Gets whether or not @presence is online.
 *
 * Returns: %TRUE if the presence is online, or %FALSE otherwise.
 */
gboolean purple_presence_is_online(PurplePresence *presence);

/**
 * purple_presence_is_status_active:
 * @presence: The #PurplePresence instance.
 * @status_id: The ID of the status.
 *
 * Gets whether or not a status in @presence is active.
 *
 * A status is active if itself or any of its sub-statuses are active.
 *
 * Returns: %TRUE if the status is active, or %FALSE.
 */
gboolean purple_presence_is_status_active(PurplePresence *presence, const gchar *status_id);

/**
 * purple_presence_is_status_primitive_active:
 * @presence: The #PurplePresence instance.
 * @primitive: The status primitive.
 *
 * Gets whether or not a status with the specified primitive type in @presence
 * is active.
 *
 * A status is active if itself or any of its sub-statuses are active.
 *
 * Returns: %TRUE if the status is active, or %FALSE.
 */
gboolean purple_presence_is_status_primitive_active(PurplePresence *presence, PurpleStatusPrimitive primitive);

/**
 * purple_presence_is_idle:
 * @presence: The #PurplePresence instance.
 *
 * Gets whether or not @presence is idle.
 *
 * Returns: %TRUE if the presence is idle, or %FALSE otherwise.  If the
 *          presence is offline (purple_presence_is_online() returns %FALSE)
 *          then %FALSE is returned.
 */
gboolean purple_presence_is_idle(PurplePresence *presence);

/**
 * purple_presence_get_idle_time:
 * @presence: The #PurplePresence instance.
 *
 * Gets the idle time of @presence. This can be %NULL if the protocol doesn't
 * support idle times or if the presence isn't in an idle state.
 *
 * Returns: (nullable): The idle time of @presence or %NULL.
 */
GDateTime *purple_presence_get_idle_time(PurplePresence *presence);

/**
 * purple_presence_get_login_time:
 * @presence: The #PurplePresence instance.
 *
 * Gets the login time of @presence.
 *
 * Returns: (transfer none): The login time of @presence.
 */
GDateTime *purple_presence_get_login_time(PurplePresence *presence);

/**
 * purple_presence_compare:
 * @presence1: The first presence.
 * @presence2: The second presence.
 *
 * Compares the presences for availability.
 *
 * Returns: -1 if @presence1 is more available than @presence2.
 *           0 if @presence1 is equal to @presence2.
 *           1 if @presence1 is less available than @presence2.
 *
 * Since: 3.0.0
 */
gint purple_presence_compare(PurplePresence *presence1, PurplePresence *presence2);

/**
 * purple_presence_get_primitive:
 * @presence: The instance.
 *
 * Gets the [enum@Purple.StatusPrimitive] for @presence.
 *
 * Returns: The current primitive.
 *
 * Since: 3.0.0
 */
PurpleStatusPrimitive purple_presence_get_primitive(PurplePresence *presence);

/**
 * purple_presence_get_message:
 * @presence: The instance.
 *
 * Gets the status message for @presence if one is set.
 *
 * Returns: (nullable): The status message of @presence.
 *
 * Since: 3.0.0
 */
const char *purple_presence_get_message(PurplePresence *presence);

G_END_DECLS

#endif /* PURPLE_PRESENCE_H */
