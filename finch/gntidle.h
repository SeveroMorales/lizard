/*
 * Finch - Universal Text Chat Client
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
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

#ifndef FINCH_IDLE_H
#define FINCH_IDLE_H

#include <purple.h>

/**
 * FinchIdle:
 *
 * An implementation of [iface@Purple.IdleUi] for Finch.
 *
 * Since: 3.0.0
 */

#define FINCH_TYPE_IDLE (finch_idle_get_type())
G_DECLARE_FINAL_TYPE(FinchIdle, finch_idle, FINCH, IDLE, GObject)

/**
 * finch_idle_new:
 *
 * Creates a new [class@Idle].
 *
 * Returns: (transfer full): The new instance.
 *
 * Since: 3.0.0
 */
PurpleIdleUi *finch_idle_new(void);

#endif /* FINCH_IDLE_H */

