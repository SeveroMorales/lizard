/*
 * Purple - Internet Messaging Library
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <https://www.gnu.org/licenses/>.
 */

#if !defined(PURPLE_GLOBAL_HEADER_INSIDE) && !defined(PURPLE_COMPILATION)
# error "only <purple.h> may be included directly"
#endif

#ifndef PURPLE_PATH_H
#define PURPLE_PATH_H

#include <glib.h>

G_BEGIN_DECLS

/**
 * purple_home_dir:
 *
 * Gets the user's home directory.
 *
 * Returns: The user's home directory.
 */
const gchar *purple_home_dir(void);

/**
 * purple_cache_dir:
 *
 * Returns the purple cache directory according to XDG Base Directory Specification.
 * This is usually $HOME/.cache/purple.
 * If custom user dir was specified then this is cache
 * sub-directory of DIR argument passed to -c option.
 *
 * Returns: The purple cache directory.
 */
const gchar *purple_cache_dir(void);

/**
 * purple_config_dir:
 *
 * Returns the purple configuration directory according to XDG Base Directory Specification.
 * This is usually $HOME/.config/purple.
 * If custom user dir was specified then this is config
 * sub-directory of DIR argument passed to -c option.
 *
 * Returns: The purple configuration directory.
 */
const gchar *purple_config_dir(void);

/**
 * purple_data_dir:
 *
 * Returns the purple data directory according to XDG Base Directory Specification.
 * This is usually $HOME/.local/share/purple.
 * If custom user dir was specified then this is data
 * sub-directory of DIR argument passed to -c option.
 *
 * Returns: The purple data directory.
 */
const gchar *purple_data_dir(void);

/**
 * purple_util_set_user_dir:
 * @dir: The custom settings directory
 *
 * Define a custom purple settings directory, overriding the default (user's home directory/.purple)
 */
void purple_util_set_user_dir(const gchar *dir);

G_END_DECLS

#endif /* PURPLE_PATH_H */
