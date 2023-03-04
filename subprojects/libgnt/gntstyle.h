/*
 * GNT - The GLib Ncurses Toolkit
 *
 * GNT is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, see <https://www.gnu.org/licenses/>.
 */

#if !defined(GNT_GLOBAL_HEADER_INSIDE) && !defined(GNT_COMPILATION)
# error "only <gnt.h> may be included directly"
#endif

#ifndef GNT_STYLE_H
#define GNT_STYLE_H

#include "gntwm.h"

/**
 * GntStyle:
 * @GNT_STYLE_SHADOW: Whether to show a shadow. Boolean.
 * @GNT_STYLE_COLOR: Whether to show colors. Boolean.
 * @GNT_STYLE_MOUSE: Whether to enable the mouse. Boolean.
 * @GNT_STYLE_WM: Which window manager to use. String.
 * @GNT_STYLE_REMPOS: Whether to remember the positions of windows. Boolean.
 * @GNT_STYLES: The number of style settings.
 *
 * The type of style that a user might set. For boolean options, use
 * gnt_style_get_bool(). For string options, use gnt_style_get().
 */
typedef enum
{
	GNT_STYLE_SHADOW = 0,
	GNT_STYLE_COLOR = 1,
	GNT_STYLE_MOUSE = 2,
	GNT_STYLE_WM = 3,
	GNT_STYLE_REMPOS = 4,
	GNT_STYLES
} GntStyle;

/**
 * gnt_style_read_configure_file:
 * @filename:  The filename to read configuration from.
 *
 * Read configuration from a file.
 */
void gnt_style_read_configure_file(const char *filename);

/**
 * gnt_style_get:
 * @style:  The style.
 *
 * Get the user-setting for a style.
 *
 * Returns:  The user-setting, or %NULL.
 */
const char *gnt_style_get(GntStyle style);

/**
 * gnt_style_get_from_name:
 * @group:   The name of the group in the keyfile. If %NULL, the prgname
 *                will be used first, if available. Otherwise, "general" will be used.
 * @key:     The key
 *
 * Get the value of a preference in ~/.gntrc.
 *
 * Returns:  The value of the setting as a string, or %NULL
 *
 * Since: 2.1.0
 */
char *gnt_style_get_from_name(const char *group, const char *key);

/**
 * gnt_style_get_string_list:
 * @group:  The name of the group in the keyfile. If %NULL, the prgname
 *          will be used first, if available. Otherwise, "general" will be used.
 * @key:    The key
 * @length: Return location for the number of strings returned, or NULL
 *
 * Get the value of a preference in ~/.gntrc.
 *
 * Returns: (transfer full): %NULL terminated string array. The array should be
 *          freed with g_strfreev().
 *
 * Since: 2.4.0
 */
char **gnt_style_get_string_list(const char *group, const char *key, gsize *length);

/**
 * gnt_style_get_color:
 * @group:   The name of the group in the keyfile. If %NULL, the prgname
 *                will be used first, if available. Otherwise, "general" will be used.
 * @key:     The key
 *
 * Get the value of a color pair in ~/.gntrc.
 *
 * Returns:  The value of the color as an int, or 0 on error.
 *
 * Since: 2.4.0
 */
int gnt_style_get_color(char *group, char *key);

/**
 * gnt_style_parse_bool:
 * @value:   The value of the boolean setting as a string
 *
 * Parse a boolean preference. For example, if 'value' is "false" (ignoring case)
 * or "0", the return value will be %FALSE, otherwise %TRUE.
 *
 * Returns:    The boolean value
 *
 * Since: 2.1.0
 */
gboolean gnt_style_parse_bool(const char *value);

/**
 * gnt_style_get_bool:
 * @style:  The style.
 * @def:    The default value (i.e, the value if the user didn't define
 *               any value)
 *
 * Get the boolean value for a user-setting.
 *
 * Returns:  The value of the setting.
 */
gboolean gnt_style_get_bool(GntStyle style, gboolean def);

/**
 * gnt_style_read_actions:
 * @type:  The type from which the setting name is derived.
 * @klass: The class on which to register bindings.
 *
 * Read user-defined actions defined for @type and register bindings on
 * the @klass.
 *
 * User-defined actions should be defined in the config file in a group
 * with a name determined by the name of the type with "::binding"
 * appended.
 */
void gnt_style_read_actions(GType type, GntBindableClass *klass);

/**
 * gnt_style_read_menu_accels:
 * @name:  The name of the window.
 * @table: The hastable to store the accel information.
 *
 * Read menu-accels from ~/.gntrc
 *
 * Returns:  %TRUE if some accels were read, %FALSE otherwise.
 */
gboolean gnt_style_read_menu_accels(const char *name, GHashTable *table);

/**
 * gnt_init_styles:
 *
 * Initialize style settings.
 */
void gnt_init_styles(void);

/**
 * gnt_uninit_styles:
 *
 * Uninitialize style settings.
 */
void gnt_uninit_styles(void);

#endif /* GNT_STYLE_H */
