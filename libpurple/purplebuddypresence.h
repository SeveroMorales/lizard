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

#if !defined(PURPLE_GLOBAL_HEADER_INSIDE) && !defined(PURPLE_COMPILATION)
# error "only <purple.h> may be included directly"
#endif

#ifndef PURPLE_BUDDY_PRESENCE_H
#define PURPLE_BUDDY_PRESENCE_H

#include "buddylist.h"
#include "purplepresence.h"

G_BEGIN_DECLS

/**
 * PurpleBuddyPresence:
 *
 * A #PurpleBuddyPresence represent the #PurplePresence for a specific
 * #PurpleBuddy.
 */

#define PURPLE_TYPE_BUDDY_PRESENCE (purple_buddy_presence_get_type())
G_DECLARE_FINAL_TYPE(PurpleBuddyPresence, purple_buddy_presence, PURPLE,
                     BUDDY_PRESENCE, PurplePresence)

/**
 * purple_buddy_presence_new:
 * @buddy: The buddy to associate with the presence.
 *
 * Creates a presence for a buddy.
 *
 * Returns: The new presence.
 *
 * Since: 3.0.0
 */
PurpleBuddyPresence *purple_buddy_presence_new(PurpleBuddy *buddy);

/**
 * purple_buddy_presence_get_buddy:
 * @presence: The presence.
 *
 * Returns the buddy presence's buddy.
 *
 * Returns: (transfer none): The presence's buddy.
 */
PurpleBuddy *purple_buddy_presence_get_buddy(PurpleBuddyPresence *presence);

/**
 * purple_buddy_presence_compare:
 * @buddy_presence1: The first presence.
 * @buddy_presence2: The second presence.
 *
 * Compares two buddy presences for availability.
 *
 * Returns: -1 if @buddy_presence1 is more available than @buddy_presence2.
 *           0 if @buddy_presence1 is equal to @buddy_presence2.
 *           1 if @buddy_presence1 is less available than @buddy_presence2.
 */
gint purple_buddy_presence_compare(PurpleBuddyPresence *buddy_presence1,
                                   PurpleBuddyPresence *buddy_presence2);

G_END_DECLS

#endif /* PURPLE_BUDDY_PRESENCE_H */
