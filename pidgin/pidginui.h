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

#ifndef PIDGIN_UI_H
#define PIDGIN_UI_H

#include <glib.h>

#include <purple.h>

G_BEGIN_DECLS

/**
 * PidginUi:
 *
 * Is a subclass of [class@Purple.Ui] that identifies Pidgin to libpurple.
 *
 * Since: 3.0.0
 */

#define PIDGIN_TYPE_UI (pidgin_ui_get_type())
G_DECLARE_FINAL_TYPE(PidginUi, pidgin_ui, PIDGIN, UI, PurpleUi)

/**
 * pidgin_ui_new:
 *
 * Creates the new [class@Pidgin.Ui] instance.
 *
 * Note: there's not much use for this outside of Pidgin's internal code.
 *
 * Returns: (transfer full): The new instance.
 */
PurpleUi *pidgin_ui_new(void);

G_END_DECLS

#endif /* PIDGIN_UI_H */
