/*
 * Pidgin - Internet Messenger
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#if !defined(PIDGIN_GLOBAL_HEADER_INSIDE) && !defined(PIDGIN_COMPILATION)
# error "only <pidgin.h> may be included directly"
#endif

#ifndef PIDGIN_PRIVATE_H
#define PIDGIN_PRIVATE_H

#include <glib.h>

#include <purple.h>

G_BEGIN_DECLS

/*
 * pidgin_commands_init:
 *
 * Initializes the default conversation commands for Pidgin.
 *
 * This should only be called internally from Pidgin.
 *
 * Since: 3.0.0
 */
void pidgin_commands_init(void);

/*
 * pidgin_commands_uninit:
 *
 * Uninitializes the default conversation commands for Pidgin.
 *
 * This should only be called internally from Pidgin.
 *
 * Since: 3.0.0
 */
void pidgin_commands_uninit(void);

G_END_DECLS

#endif /* PIDGIN_PRIVATE_H */
