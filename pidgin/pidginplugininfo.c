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
#include "pidginplugininfo.h"

#include "pidgincore.h"

enum {
	PROP_0,
	PROP_GTK_CONFIG_FRAME,
	N_PROPERTIES,
};
static GParamSpec *properties[N_PROPERTIES] = {NULL, };

struct _PidginPluginInfo {
	PurplePluginInfo parent;

	PidginPluginInfoGetConfigFrameFunc get_config_frame;
};

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_TYPE(PidginPluginInfo, pidgin_plugin_info, PURPLE_TYPE_PLUGIN_INFO)

static void
pidgin_plugin_info_get_property(GObject *obj, guint param_id, GValue *value,
                                GParamSpec *pspec)
{
	PidginPluginInfo *info = PIDGIN_PLUGIN_INFO(obj);

	switch(param_id) {
		case PROP_GTK_CONFIG_FRAME:
			g_value_set_pointer(value, info->get_config_frame);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
pidgin_plugin_info_set_property(GObject *obj, guint param_id,
                                const GValue *value, GParamSpec *pspec)
{
	PidginPluginInfo *info = PIDGIN_PLUGIN_INFO(obj);

	switch(param_id) {
		case PROP_GTK_CONFIG_FRAME:
			info->get_config_frame = g_value_get_pointer(value);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
pidgin_plugin_info_init(G_GNUC_UNUSED PidginPluginInfo *info) {
}

static void
pidgin_plugin_info_class_init(PidginPluginInfoClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->get_property = pidgin_plugin_info_get_property;
	obj_class->set_property = pidgin_plugin_info_set_property;

	/**
	 * PidginPluginInfo:gtk-config-frame-cb:
	 *
	 * A function to call to create the configuration widget for the plugin.
	 */
	properties[PROP_GTK_CONFIG_FRAME] = g_param_spec_pointer(
		"gtk-config-frame-cb", "gtk-config-frame-cb",
		"The callback function to create a configuration widget.",
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

/******************************************************************************
 * API
 *****************************************************************************/
GPluginPluginInfo *
pidgin_plugin_info_new(const char *first_property, ...) {
	GObject *info;
	va_list var_args;

	/* at least ID is required */
	if (!first_property) {
		return NULL;
	}

	va_start(var_args, first_property);
	info = g_object_new_valist(PIDGIN_TYPE_PLUGIN_INFO, first_property,
	                           var_args);
	va_end(var_args);

	return GPLUGIN_PLUGIN_INFO(info);
}
