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

#include "pidgin/pidginpresenceicon.h"

#include "pidgin/pidginiconname.h"

struct _PidginPresenceIcon {
	GtkBox parent;

	GtkWidget *icon;

	PurplePresence *presence;
	gchar *fallback;
};

enum {
	PROP_0,
	PROP_PRESENCE,
	PROP_FALLBACK,
	PROP_ICON_SIZE,
	N_PROPERTIES
};
static GParamSpec *properties[N_PROPERTIES] = { NULL, };

G_DEFINE_TYPE(PidginPresenceIcon, pidgin_presence_icon, GTK_TYPE_BOX)

/******************************************************************************
 * Implementation
 *****************************************************************************/
static void
pidgin_presence_icon_update(PidginPresenceIcon *icon) {
	const gchar *icon_name = NULL;

	icon_name = pidgin_icon_name_from_presence(icon->presence, icon->fallback);

	gtk_image_set_from_icon_name(GTK_IMAGE(icon->icon), icon_name);
}

static void
pidgin_presence_icon_active_status_changed_cb(G_GNUC_UNUSED GObject *obj,
                                              G_GNUC_UNUSED GParamSpec *pspec,
                                              gpointer data)
{
	pidgin_presence_icon_update(PIDGIN_PRESENCE_ICON(data));
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
pidgin_presence_icon_get_property(GObject *obj, guint param_id, GValue *value,
                                  GParamSpec *pspec)
{
	PidginPresenceIcon *icon = PIDGIN_PRESENCE_ICON(obj);

	switch(param_id) {
		case PROP_PRESENCE:
			g_value_set_object(value, pidgin_presence_icon_get_presence(icon));
			break;
		case PROP_FALLBACK:
			g_value_set_string(value, pidgin_presence_icon_get_fallback(icon));
			break;
		case PROP_ICON_SIZE:
			g_value_set_enum(value, pidgin_presence_icon_get_icon_size(icon));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
pidgin_presence_icon_set_property(GObject *obj, guint param_id,
                                  const GValue *value, GParamSpec *pspec)
{
	PidginPresenceIcon *icon = PIDGIN_PRESENCE_ICON(obj);

	switch(param_id) {
		case PROP_PRESENCE:
			pidgin_presence_icon_set_presence(icon, g_value_get_object(value));
			break;
		case PROP_FALLBACK:
			pidgin_presence_icon_set_fallback(icon, g_value_get_string(value));
			break;
		case PROP_ICON_SIZE:
			pidgin_presence_icon_set_icon_size(icon, g_value_get_enum(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
pidgin_presence_icon_finalize(GObject *obj) {
	PidginPresenceIcon *icon = PIDGIN_PRESENCE_ICON(obj);

	g_clear_object(&icon->presence);
	g_clear_pointer(&icon->fallback, g_free);

	G_OBJECT_CLASS(pidgin_presence_icon_parent_class)->finalize(obj);
}

static void
pidgin_presence_icon_init(PidginPresenceIcon *presenceicon) {
	gtk_widget_init_template(GTK_WIDGET(presenceicon));
}

static void
pidgin_presence_icon_class_init(PidginPresenceIconClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	obj_class->finalize = pidgin_presence_icon_finalize;
	obj_class->get_property = pidgin_presence_icon_get_property;
	obj_class->set_property = pidgin_presence_icon_set_property;

	/**
	 * PidginPresenceIcon:presence:
	 *
	 * The presence that this icon will be representing.
	 */
	properties[PROP_PRESENCE] = g_param_spec_object(
		"presence", "presence",
		"The presence that this icon is representing",
		PURPLE_TYPE_PRESENCE,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

	/**
	 * PidginPresenceIcon:fallback:
	 *
	 * The name of the icon to use as a fallback when no presence is set.
	 */
	properties[PROP_FALLBACK] = g_param_spec_string(
		"fallback", "fallback",
		"The name of the icon to use as a fallback",
		"user-invisible",
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

	/**
	 * PidginPresenceIcon:icon-size:
	 *
	 * The size of the icon that should be used.
	 */
	properties[PROP_ICON_SIZE] = g_param_spec_enum(
		"icon-size", "icon-size",
		"The GtkIconSize to use",
		GTK_TYPE_ICON_SIZE,
		GTK_ICON_SIZE_NORMAL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);

	gtk_widget_class_set_template_from_resource(
	    widget_class,
	    "/im/pidgin/Pidgin3/presenceicon.ui"
	);

	gtk_widget_class_bind_template_child(widget_class, PidginPresenceIcon,
	                                     icon);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
GtkWidget *
pidgin_presence_icon_new(PurplePresence *presence, const gchar *fallback,
                         GtkIconSize icon_size)
{
	return g_object_new(
		PIDGIN_TYPE_PRESENCE_ICON,
		"presence", presence,
		"fallback", fallback,
		"icon_size", icon_size,
		NULL);
}

PurplePresence *
pidgin_presence_icon_get_presence(PidginPresenceIcon *icon) {
	g_return_val_if_fail(PIDGIN_IS_PRESENCE_ICON(icon), NULL);

	return icon->presence;
}

void
pidgin_presence_icon_set_presence(PidginPresenceIcon *icon,
                                  PurplePresence *presence)
{
	PurplePresence *old = NULL;

	g_return_if_fail(PIDGIN_IS_PRESENCE_ICON(icon));

	/* We only want to disconnect signal handlers if the presence was changed,
	 * but to do that, we need to keep a reference to the old presence.
	 */
	if(PURPLE_IS_PRESENCE(icon->presence)) {
		old = g_object_ref(icon->presence);
	}

	if(g_set_object(&icon->presence, presence)) {
		if(G_IS_OBJECT(old)) {
			/* If we previously had a presence, disconnect our signal handlers.
			 */
			g_signal_handlers_disconnect_by_data(old, icon);
		}

		if(PURPLE_IS_PRESENCE(icon->presence)) {
			g_signal_connect(icon->presence, "notify::active-status",
			                 G_CALLBACK(pidgin_presence_icon_active_status_changed_cb),
			                 icon);
		}

		g_object_freeze_notify(G_OBJECT(icon));

		pidgin_presence_icon_update(icon);

		g_object_notify_by_pspec(G_OBJECT(icon), properties[PROP_PRESENCE]);

		g_object_thaw_notify(G_OBJECT(icon));
	}

	g_clear_object(&old);
}

const gchar *
pidgin_presence_icon_get_fallback(PidginPresenceIcon *icon) {
	g_return_val_if_fail(PIDGIN_IS_PRESENCE_ICON(icon), NULL);

	return icon->fallback;
}

void
pidgin_presence_icon_set_fallback(PidginPresenceIcon *icon,
                                  const gchar *fallback)
{
	g_return_if_fail(PIDGIN_IS_PRESENCE_ICON(icon));
	g_return_if_fail(fallback != NULL);

	g_free(icon->fallback);
	icon->fallback = g_strdup(fallback);

	g_object_freeze_notify(G_OBJECT(icon));

	pidgin_presence_icon_update(icon);

	g_object_notify_by_pspec(G_OBJECT(icon), properties[PROP_FALLBACK]);

	g_object_thaw_notify(G_OBJECT(icon));
}

GtkIconSize
pidgin_presence_icon_get_icon_size(PidginPresenceIcon *icon) {
	g_return_val_if_fail(PIDGIN_IS_PRESENCE_ICON(icon), GTK_ICON_SIZE_INHERIT);

	return gtk_image_get_icon_size(GTK_IMAGE(icon->icon));
}

void
pidgin_presence_icon_set_icon_size(PidginPresenceIcon *icon,
                                   GtkIconSize icon_size)
{
	g_return_if_fail(PIDGIN_IS_PRESENCE_ICON(icon));

	gtk_image_set_icon_size(GTK_IMAGE(icon->icon), icon_size);

	g_object_freeze_notify(G_OBJECT(icon));

	pidgin_presence_icon_update(icon);

	g_object_notify_by_pspec(G_OBJECT(icon), properties[PROP_ICON_SIZE]);

	g_object_thaw_notify(G_OBJECT(icon));
}
