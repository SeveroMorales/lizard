/* pidgin
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 */

#if !defined(PIDGIN_GLOBAL_HEADER_INSIDE) && !defined(PIDGIN_COMPILATION)
# error "only <pidgin.h> may be included directly"
#endif

#ifndef PIDGIN_PLUGIN_INFO_H
#define PIDGIN_PLUGIN_INFO_H

#include <gtk/gtk.h>

#include <gplugin.h>

#include <purple.h>

G_BEGIN_DECLS

/**
 * PidginPluginInfo:
 *
 * A #GPluginPluginInfo subclass that adds some Pidgin specific properties.
 *
 * Since: 3.0.0
 */

#define PIDGIN_TYPE_PLUGIN_INFO (pidgin_plugin_info_get_type())
G_DECLARE_FINAL_TYPE(PidginPluginInfo, pidgin_plugin_info, PIDGIN, PLUGIN_INFO, PurplePluginInfo)

/**
 * PidginPluginInfoGetConfigFrameFunc:
 * @plugin: The instance of the plugin.
 *
 * Gets the [class@Gtk.Widget] for configruing @plugin.
 *
 * Returns: (transfer full): The widget used to configure @plugin.
 *
 * Since: 3.0.0
 */
typedef GtkWidget *(*PidginPluginInfoGetConfigFrameFunc)(PurplePlugin *plugin);

/**
 * pidgin_plugin_info_new:
 * @first_property:  The first property name
 * @...:  The value of the first property, followed optionally by more
 *        name/value pairs, followed by %NULL
 *
 * Creates a new #PidginPluginInfo instance to be returned from
 * #plugin_query of a pidgin plugin, using the provided name/value
 * pairs.
 *
 * See purple_plugin_info_new() for a list of available property names.
 * Additionally, you can provide the property
 * <literal>"gtk-config-frame-cb"</literal>, which should be a callback that
 * returns a #GtkWidget for the plugin's configuration
 * (see #PidginPluginConfigFrameCb).
 *
 * See purple_plugin_info_new().
 *
 * Returns: A new #PidginPluginInfo instance.
 *
 * Since: 3.0.0
 */
GPluginPluginInfo *pidgin_plugin_info_new(const char *first_property, ...) G_GNUC_NULL_TERMINATED;

G_END_DECLS

#endif /* PIDGIN_PLUGIN_INFO_H */
