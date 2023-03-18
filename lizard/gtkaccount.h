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

#ifndef _PIDGINACCOUNT_H_
#define _PIDGINACCOUNT_H_

#include <purple.h>

G_BEGIN_DECLS

/**
 * pidgin_accounts_init:
 *
 * Initializes the GTK account system
 */
void pidgin_accounts_init(void);

/**
 * pidgin_accounts_uninit:
 *
 * Uninitializes the GTK account system
 */
void pidgin_accounts_uninit(void);

G_END_DECLS

#endif /* _PIDGINACCOUNT_H_ */
