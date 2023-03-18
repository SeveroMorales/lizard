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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301 USA
 */

#if !defined(PIDGIN_GLOBAL_HEADER_INSIDE) && !defined(PIDGIN_COMPILATION)
# error "only <pidgin.h> may be included directly"
#endif

#ifndef PIDGIN_PROTOCOL_CHOOSER_H
#define PIDGIN_PROTOCOL_CHOOSER_H

#include <gtk/gtk.h>
#include <adwaita.h>

#include <purple.h>

G_BEGIN_DECLS

#define PIDGIN_TYPE_PROTOCOL_CHOOSER (pidgin_protocol_chooser_get_type())

G_DECLARE_FINAL_TYPE(PidginProtocolChooser, pidgin_protocol_chooser, PIDGIN,
                     PROTOCOL_CHOOSER, AdwComboRow)

/**
 * pidgin_protocol_chooser_new:
 *
 * Creates a drop down for a user to select a protocol from.
 *
 * Returns: (transfer full): The protocol chooser drop down.
 *
 * Since: 3.0.0
 */
GtkWidget *pidgin_protocol_chooser_new(void);

/**
 * pidgin_protocol_chooser_get_protocol:
 * @chooser: The #PidginProtocolChooser instance.
 *
 * Gets the currently selected protocol from @chooser.
 *
 * Returns: (transfer none): The selected [class@Purple.Protocol] or %NULL if
 *          nothing is selected.
 *
 * Since: 3.0.0
 */
PurpleProtocol *pidgin_protocol_chooser_get_protocol(PidginProtocolChooser *chooser);

/**
 * pidgin_protocol_chooser_set_protocol:
 * @chooser: The #PidginProtocolChooser instance.
 * @protocol: (transfer none): The protocol to select.
 *
 * Sets the currently selected protocol of @chooser to the given
 * [class@Purple.Protocol].
 *
 * Since: 3.0.0
 */
void pidgin_protocol_chooser_set_protocol(PidginProtocolChooser *chooser, PurpleProtocol *protocol);

G_END_DECLS

#endif /* PIDGIN_PROTOCOL_CHOOSER_H */
