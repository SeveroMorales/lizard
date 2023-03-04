/* pidgin
 *
 * Pidgin is the legal property of its developers, whose names are too numerous
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

#if !defined(PIDGIN_GLOBAL_HEADER_INSIDE) && !defined(PIDGIN_COMPILATION)
# error "only <pidgin.h> may be included directly"
#endif

#ifndef _PIDGINBLIST_H_
#define _PIDGINBLIST_H_

#include <purple.h>

#define PIDGIN_TYPE_BUDDY_LIST (pidgin_buddy_list_get_type())

typedef struct _PidginBuddyList PidginBuddyList;

/**************************************************************************
 * Structures
 **************************************************************************/
/**
 * PidginBuddyList:
 *
 * The remnants of the buddy list, soon to be lost to the wind.
 */
struct _PidginBuddyList {
	PurpleBuddyList parent;
};

G_BEGIN_DECLS

/**************************************************************************
 * GTK Buddy List API
 **************************************************************************/

G_DECLARE_FINAL_TYPE(PidginBuddyList, pidgin_buddy_list, PIDGIN, BUDDY_LIST,
                     PurpleBuddyList)

/**
 * pidgin_blist_joinchat_is_showable:
 *
 * Determines if showing the join chat dialog is a valid action.
 *
 * Returns: Returns TRUE if there are accounts online capable of
 *         joining chat rooms.  Otherwise returns FALSE.
 */
gboolean pidgin_blist_joinchat_is_showable(void);

/**
 * pidgin_blist_joinchat_show:
 *
 * Shows the join chat dialog.
 */
void pidgin_blist_joinchat_show(void);

G_END_DECLS

#endif /* _PIDGINBLIST_H_ */
