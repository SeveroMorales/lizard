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

#ifndef PURPLE_PLUGIN_INFO_H
#define PURPLE_PLUGIN_INFO_H

#include <glib.h>

#include <gio/gio.h>

#include <gplugin.h>
#include <gplugin-native.h>

#include "pluginpref.h"

#define PURPLE_TYPE_PLUGIN_INFO (purple_plugin_info_get_type())

/**
 * PurplePluginInfo:
 *
 * #PurplePluginInfo is a #GPluginPluginInfo subclass that adds additional
 * libpurple specific properties.
 *
 * Since: 3.0.0
 */

G_DECLARE_DERIVABLE_TYPE(PurplePluginInfo, purple_plugin_info, PURPLE,
                         PLUGIN_INFO, GPluginPluginInfo)

#include "plugins.h"

/**
 * PurplePluginInfoClass:
 *
 * An opaque type representing the class of a #PurplePluginInfo.
 *
 * Since: 3.0.0
 */
struct _PurplePluginInfoClass {
	/*< private >*/
	GPluginPluginInfoClass parent;

	gpointer reserved[4];
};

/**
 * PurplePluginPrefFrameCb:
 * @plugin: the plugin associated with this callback.
 *
 * Returns the preferences frame for the plugin.
 *
 * Returns: Preference frame.
 */
typedef PurplePluginPrefFrame *(*PurplePluginPrefFrameCb)(PurplePlugin *plugin);

/**
 * PurplePrefRequestCb:
 *
 * Returns the preferences request handle for a plugin.
 *
 * Returns: Preferences request handle.
 */
typedef gpointer (*PurplePluginPrefRequestCb)(PurplePlugin *plugin);

/**
 * PurplePluginInfoFlags:
 * @PURPLE_PLUGIN_INFO_FLAGS_INTERNAL:  Plugin is not shown in UI lists
 * @PURPLE_PLUGIN_INFO_FLAGS_AUTO_LOAD: Auto-load the plugin
 *
 * Flags that can be used to treat plugins differently.
 *
 * Since: 3.0.0
 */
typedef enum /*< flags >*/
{
	PURPLE_PLUGIN_INFO_FLAGS_INTERNAL  = 1 << 1,
	PURPLE_PLUGIN_INFO_FLAGS_AUTO_LOAD = 1 << 2,
} PurplePluginInfoFlags;

/**
 * PURPLE_PLUGIN_ABI_VERSION:
 * @major: The major version of libpurple to target.
 * @minor: The minor version of libpurple to target.
 *
 * Packs @major and @minor into an integer to be used as an abi version for
 * gplugin.
 *
 * Note: The lower six nibbles represent the ABI version for libpurple, the
 *       rest are required by GPlugin.
 *
 * Returns: An ABI version to set in plugins using major and minor versions.
 */
#define PURPLE_PLUGIN_ABI_VERSION(major,minor) \
	(0x01000000 | ((major) << 16) | (minor))

/**
 * PURPLE_PLUGIN_ABI_MAJOR_VERSION:
 * @abi: The abi version.
 *
 * Extracts the purple major version from @abi.
 *
 * Returns: The major version from an ABI version
 */
#define PURPLE_PLUGIN_ABI_MAJOR_VERSION(abi) \
	((abi >> 16) & 0xff)

/**
 * PURPLE_PLUGIN_ABI_MINOR_VERSION:
 * @abi: The abi version.
 *
 * Extracts the purple minor version from @abi.
 *
 * Returns: The minor version from an ABI version
 */
#define PURPLE_PLUGIN_ABI_MINOR_VERSION(abi) \
	(abi & 0xffff)

/**
 * PURPLE_ABI_VERSION:
 *
 * A convenience macro that returns an ABI version using PURPLE_MAJOR_VERSION
 * and PURPLE_MINOR_VERSION
 */
#define PURPLE_ABI_VERSION PURPLE_PLUGIN_ABI_VERSION(PURPLE_MAJOR_VERSION, PURPLE_MINOR_VERSION)

G_BEGIN_DECLS

/**
 * purple_plugin_info_new:
 * @first_property:  The first property name
 * @...:  The value of the first property, followed optionally by more
 *        name/value pairs, followed by %NULL
 *
 * Creates a new #PurplePluginInfo instance to be returned from
 * #plugin_query of a plugin, using the provided name/value pairs.
 *
 * All properties except <literal>"id"</literal> and
 * <literal>"purple-abi"</literal> are optional.
 *
 * Valid property names are:
 * <informaltable frame='none'>
 *   <tgroup cols='2'><tbody>
 *   <row><entry><literal>"id"</literal></entry>
 *     <entry>(string) The ID of the plugin.</entry>
 *   </row>
 *   <row><entry><literal>"abi-version"</literal></entry>
 *     <entry>(<type>guint32</type>) The ABI version required by the
 *       plugin.</entry>
 *   </row>
 *   <row><entry><literal>"name"</literal></entry>
 *     <entry>(string) The translated name of the plugin.</entry>
 *   </row>
 *   <row><entry><literal>"version"</literal></entry>
 *     <entry>(string) Version of the plugin.</entry>
 *   </row>
 *   <row><entry><literal>"category"</literal></entry>
 *     <entry>(string) Primary category of the plugin.</entry>
 *   </row>
 *   <row><entry><literal>"summary"</literal></entry>
 *     <entry>(string) Brief summary of the plugin.</entry>
 *   </row>
 *   <row><entry><literal>"description"</literal></entry>
 *     <entry>(string) Full description of the plugin.</entry>
 *   </row>
 *   <row><entry><literal>"authors"</literal></entry>
 *     <entry>(<type>const gchar * const *</type>) A %NULL-terminated list of
 *       plugin authors. format: First Last &lt;user\@domain.com&gt;</entry>
 *   </row>
 *   <row><entry><literal>"website"</literal></entry>
 *     <entry>(string) Website of the plugin.</entry>
 *   </row>
 *   <row><entry><literal>"icon"</literal></entry>
 *     <entry>(string) Path to a plugin's icon.</entry>
 *   </row>
 *   <row><entry><literal>"license-id"</literal></entry>
 *     <entry>(string) Short name of the plugin's license. This should
 *       either be an identifier of the license from
 *       <ulink url="http://dep.debian.net/deps/dep5/#license-specification">
 *       DEP5</ulink> or "Other" for custom licenses.</entry>
 *   </row>
 *   <row><entry><literal>"license-text"</literal></entry>
 *     <entry>(string) The text of the plugin's license, if unlisted on
 *       DEP5.</entry>
 *   </row>
 *   <row><entry><literal>"license-url"</literal></entry>
 *     <entry>(string) The plugin's license URL, if unlisted on DEP5.</entry>
 *   </row>
 *   <row><entry><literal>"dependencies"</literal></entry>
 *     <entry>(<type>const gchar * const *</type>) A %NULL-terminated list of
 *       plugin IDs required by the plugin.</entry>
 *   </row>
 *   <row><entry><literal>"pref-frame-cb"</literal></entry>
 *     <entry>(#PurplePluginPrefFrameCb) Callback that returns a
 *       preferences frame for the plugin.</entry>
 *   </row>
 *   <row><entry><literal>"pref-request-cb"</literal></entry>
 *     <entry>(#PurplePluginPrefRequestCb) Callback that returns a
 *       preferences request handle for the plugin.</entry>
 *   </row>
 *   <row><entry><literal>"flags"</literal></entry>
 *     <entry>(#PurplePluginInfoFlags) The flags for a plugin.</entry>
 *   </row>
 *   </tbody></tgroup>
 * </informaltable>
 *
 * See #PURPLE_PLUGIN_ABI_VERSION,
 *     <link linkend="chapter-plugin-ids">Plugin IDs</link>.
 *
 * Returns: A new #PurplePluginInfo instance.
 *
 * Since: 3.0.0
 */
GPluginPluginInfo *purple_plugin_info_new(const char *first_property, ...) G_GNUC_NULL_TERMINATED;

/**
 * purple_plugin_info_get_pref_frame_cb:
 * @info: The plugin info to get the callback from.
 *
 * Returns the callback that retrieves the preferences frame for a plugin, set
 * via the "pref-frame-cb" property of the plugin info.
 *
 * Returns: The callback that returns the preferences frame.
 *
 * Since: 3.0.0
 */
PurplePluginPrefFrameCb purple_plugin_info_get_pref_frame_cb(PurplePluginInfo *info);

/**
 * purple_plugin_info_get_pref_request_cb:
 * @info: The plugin info to get the callback from.
 *
 * Returns the callback that retrieves the preferences request handle for a
 * plugin, set via the "pref-request-cb" property of the plugin info.
 *
 * Returns: (transfer none): The callback that returns the preferences request handle.
 *
 * Since: 3.0.0
 */
PurplePluginPrefRequestCb purple_plugin_info_get_pref_request_cb(PurplePluginInfo *info);

/**
 * purple_plugin_info_get_flags:
 * @info: The plugin's info instance.
 *
 * Returns the plugin's flags.
 *
 * Returns: The flags of the plugin.
 *
 * Since: 3.0.0
 */
PurplePluginInfoFlags purple_plugin_info_get_flags(PurplePluginInfo *info);

/**
 * purple_plugin_info_get_error:
 * @info: The plugin info.
 *
 * Returns an error in the plugin info that would prevent the plugin from being
 * loaded.
 *
 * Returns: The plugin info error, or %NULL.
 *
 * Since: 3.0.0
 */
const gchar *purple_plugin_info_get_error(PurplePluginInfo *info);

/**
 * purple_plugin_info_get_unloaded:
 * @info: The #PurplePluginInfo instance.
 *
 * Gets whether or not the plugin has been unloaded.
 *
 * Returns: %TRUE if the plugin has been unloaded previously, or %FALSE if not.
 *
 * Since: 3.0.0
 */
gboolean purple_plugin_info_get_unloaded(PurplePluginInfo *info);

/**
 * purple_plugin_info_set_unloaded:
 * @info: The #PurplePluginInfo instance.
 * @unloaded: %TRUE to say the plugin has been unloaded.
 *
 * Sets the unloaded state of @info to @unloaded.
 *
 * Since: 3.0.0
 */
void purple_plugin_info_set_unloaded(PurplePluginInfo *info, gboolean unloaded);

/**
 * purple_plugin_info_get_action_group:
 * @info: The instance.
 *
 * Gets the [class:Gio.ActionGroup] from @info if one is set.
 *
 * Returns: (transfer full): The action group.
 *
 * Since: 3.0.0
 */
GActionGroup *purple_plugin_info_get_action_group(PurplePluginInfo *info);

/**
 * purple_plugin_info_get_action_menu:
 * @info: The instance.
 *
 * Gets the [class:Gio.MenuModel] from @info if one is set.
 *
 * Returns: (transfer full): The menu model.
 *
 * Since: 3.0.0
 */
GMenuModel *purple_plugin_info_get_action_menu(PurplePluginInfo *info);

G_END_DECLS

#endif /* PURPLE_PLUGIN_INFO_H */

