/*
 * Pidgin - Universal Chat Client
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

#if !defined(PIDGIN_GLOBAL_HEADER_INSIDE) && !defined(PIDGIN_COMPILATION)
# error "only <pidgin.h> may be included directly"
#endif

#ifndef PIDGIN_IDLE_H
#define PIDGIN_IDLE_H

#include <purple.h>

G_BEGIN_DECLS

/**
 * PidginIdle:
 *
 * An implementation of [iface@Purple.IdleUi] for Pidgin.
 *
 * Since: 3.0.0
 */

#define PIDGIN_TYPE_IDLE (pidgin_idle_get_type())
G_DECLARE_FINAL_TYPE(PidginIdle, pidgin_idle, PIDGIN, IDLE, GObject)

/**
 * pidgin_idle_new:
 *
 * Creates a new [class@Idle].
 *
 * Returns: (transfer full): The new instance.
 *
 * Since: 3.0.0
 */
PurpleIdleUi *pidgin_idle_new(void);

G_END_DECLS

#endif /* PIDGIN_IDLE_H */
