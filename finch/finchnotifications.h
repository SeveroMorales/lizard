/*
 * Finch - Universal Text Chat Client
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#if !defined(FINCH_GLOBAL_HEADER_INSIDE) && !defined(FINCH_COMPILATION)
# error "only <finch.h> may be included directly"
#endif

#ifndef FINCH_NOTIFICATIONS_H
#define FINCH_NOTIFICATIONS_H

#include <purple.h>

G_BEGIN_DECLS

/**
 * finch_notifications_init:
 *
 * Perform necessary initializations.
 *
 * Since: 3.0.0
 */
void finch_notifications_init(void);

/**
 * finch_notifications_uninit:
 *
 * Perform necessary uninitialization.
 */
void finch_notifications_uninit(void);

/**
 * finch_notifications_window_show:
 *
 * Show the notifications window.
 */
void finch_notifications_window_show(void);

G_END_DECLS

#endif /* FINCH_NOTIFICATIONS_H */

