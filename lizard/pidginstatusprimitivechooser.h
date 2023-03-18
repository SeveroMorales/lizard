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

#ifndef PIDGIN_STATUS_PRIMITIVE_CHOOSER_H
#define PIDGIN_STATUS_PRIMITIVE_CHOOSER_H

#include <gtk/gtk.h>

#include <adwaita.h>

#include <purple.h>

G_BEGIN_DECLS

/**
 * PidginStatusPrimitiveChooser:
 *
 * A [class@Gtk.ComboBox] for presenting [enum@Purple.StatusPrimitive]'s to a
 * user.
 *
 * Since: 3.0.0
 */

#define PIDGIN_TYPE_STATUS_PRIMITIVE_CHOOSER (pidgin_status_primitive_chooser_get_type())
G_DECLARE_FINAL_TYPE(PidginStatusPrimitiveChooser,
                     pidgin_status_primitive_chooser, PIDGIN,
                     STATUS_PRIMITIVE_CHOOSER, AdwBin)

/**
 * pidgin_status_primitive_chooser_new:
 *
 * Creates a new combo box that contains all of the available status
 * primitives in purple.
 *
 * Returns: (transfer full): The new combo box.
 *
 * Since: 3.0.0
 */
GtkWidget *pidgin_status_primitive_chooser_new(void);

/**
 * pidgin_status_primitive_chooser_get_selected:
 * @chooser: The instance.
 *
 * Gets the [enum@Purple.StatusPrimitive] that is selected.
 *
 * Returns: The selected primitive status.
 *
 * Since: 3.0.0
 */
PurpleStatusPrimitive pidgin_status_primitive_chooser_get_selected(PidginStatusPrimitiveChooser *chooser);

/**
 * pidgin_status_primitive_chooser_set_selected:
 * @chooser: The instance.
 * @primitive: The [enum@Purple.StatusPrimitive] to set.
 *
 * Sets @primitive as the active item.
 *
 * Since: 3.0.0
 */
void pidgin_status_primitive_chooser_set_selected(PidginStatusPrimitiveChooser *chooser, PurpleStatusPrimitive primitive);

G_END_DECLS

#endif /* PIDGIN_STATUS_PRIMITIVE_CHOOSER_H */
