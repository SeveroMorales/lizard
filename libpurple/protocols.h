/* purple
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
 */

#if !defined(PURPLE_GLOBAL_HEADER_INSIDE) && !defined(PURPLE_COMPILATION)
# error "only <purple.h> may be included directly"
#endif

#ifndef PURPLE_PROTOCOLS_H
#define PURPLE_PROTOCOLS_H

/**************************************************************************/
/* Basic Protocol Information                                             */
/**************************************************************************/

#include "media.h"
#include "purpleprotocol.h"
#include "status.h"

G_BEGIN_DECLS

/**************************************************************************/
/* Protocol API                                                           */
/**************************************************************************/

/**
 * purple_protocol_got_user_idle:
 * @account:   The account the user is on.
 * @name:      The name of the buddy.
 * @idle:      The user's idle state.
 * @idle_time: The user's idle time.  This is the time at
 *                  which the user became idle, in seconds since
 *                  the epoch.  If the protocol does not know this value
 *                  then it should pass 0.
 *
 * Notifies Purple that a buddy's idle state and time have changed.
 *
 * This is meant to be called from protocols.
 */
void purple_protocol_got_user_idle(PurpleAccount *account, const char *name,
                                   gboolean idle, time_t idle_time);

/**
 * purple_protocol_got_user_status:
 * @account: The account the user is on.
 * @name: The name of the buddy.
 * @status_id: The status ID.
 * @...: A NULL-terminated list of attribute IDs and values.
 *
 * Notifies Purple that a buddy's status has been activated.
 *
 * This is meant to be called from protocols.
 */
void purple_protocol_got_user_status(PurpleAccount *account, const char *name,
                                     const char *status_id, ...)
                                     G_GNUC_NULL_TERMINATED;

/**
 * purple_protocol_got_user_status_with_attributes: (rename-to purple_protocol_got_user_status):
 * @account: The account the user is on.
 * @name: The name of the buddy.
 * @status_id: The status ID.
 * @attributes: (element-type utf8 gpointer): A hash table of attribute IDs and
 *              their corresponding values.
 *
 * Notifies Purple that a buddy's status has been activated.
 *
 * This is meant to be called from protocols.
 *
 * Since: 3.0.0
 */
void purple_protocol_got_user_status_with_attributes(PurpleAccount *account, const gchar *name, const gchar *status_id, GHashTable *attributes);

/**
 * purple_protocol_got_user_status_deactive:
 * @account:   The account the user is on.
 * @name:      The name of the buddy.
 * @status_id: The status ID.
 *
 * Notifies libpurple that a buddy's status has been deactivated
 *
 * This is meant to be called from protocols.
 */
void purple_protocol_got_user_status_deactive(PurpleAccount *account,
                                              const char *name,
                                              const char *status_id);

/**
 * purple_protocol_change_account_status:
 * @account:    The account the user is on.
 * @old_status: The previous status.
 * @new_status: The status that was activated, or deactivated
 *                   (in the case of independent statuses).
 *
 * Informs the server that our account's status changed.
 */
void purple_protocol_change_account_status(PurpleAccount *account,
                                           PurpleStatus *old_status,
                                           PurpleStatus *new_status);

/**
 * purple_protocol_get_statuses:
 * @account: The account the user is on.
 * @presence: The presence for which we're going to get statuses
 *
 * Retrieves the list of stock status types from a protocol.
 *
 * Returns: (transfer full) (element-type PurpleStatus): List of statuses
 */
GList *purple_protocol_get_statuses(PurpleAccount *account,
                                    PurplePresence *presence);

/**
 * purple_protocol_get_media_caps:
 * @account: The account the user is on.
 * @who: The name of the contact to check capabilities for.
 *
 * Determines if the contact supports the given media session type.
 *
 * Returns: The media caps the contact supports.
 */
PurpleMediaCaps purple_protocol_get_media_caps(PurpleAccount *account,
                                               const char *who);

/**
 * purple_protocol_initiate_media:
 * @account: The account the user is on.
 * @who: The name of the contact to start a session with.
 * @type: The type of media session to start.
 *
 * Initiates a media session with the given contact.
 *
 * Returns: TRUE if the call succeeded else FALSE. (Doesn't imply the media
 *          session or stream will be successfully created)
 */
gboolean purple_protocol_initiate_media(PurpleAccount *account,
                                        const char *who,
                                        PurpleMediaSessionType type);

/**
 * purple_protocol_got_media_caps:
 * @account: The account the user is on.
 * @who: The name of the contact for which capabilities have been received.
 *
 * Signals that the protocol received capabilities for the given contact.
 *
 * This function is intended to be used only by protocols.
 */
void purple_protocol_got_media_caps(PurpleAccount *account, const char *who);

G_END_DECLS

#endif /* PURPLE_PROTOCOLS_H */
