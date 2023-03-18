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

#ifndef PIDGIN_PLUGINS_MENU_H
#define PIDGIN_PLUGINS_MENU_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

/**
 * PidginPluginsMenu:
 *
 * #PidginPluginsMenu is a [class@Gio.MenuModel] that automatically updates
 * itself when plugins are loaded and unloaded.
 *
 * Since: 3.0.0
 */

#define PIDGIN_TYPE_PLUGINS_MENU (pidgin_plugins_menu_get_type())
G_DECLARE_FINAL_TYPE(PidginPluginsMenu, pidgin_plugins_menu, PIDGIN,
                     PLUGINS_MENU, GMenuModel)

/**
 * pidgin_plugins_menu_new:
 *
 * Creates a new #PidginPluginsMenu instance that keeps itself up to date.
 *
 * Returns: (transfer full): The new #PidginPluginsMenu instance.
 */
GMenuModel *pidgin_plugins_menu_new(void);

G_END_DECLS

#endif /* PIDGIN_PLUGINS_MENU_H */
