/*
 * purple
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
 *
 * Purple is the legal property of its developers, whose names are too numerous
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

#include <glib/gi18n-lib.h>

#include "purplepresence.h"

#include "debug.h"
#include "purpleenums.h"
#include "purpleprivate.h"

typedef struct {
	gboolean idle;
	GDateTime *idle_time;
	GDateTime *login_time;

	GHashTable *status_table;

	PurpleStatus *active_status;
} PurplePresencePrivate;

enum {
	PROP_0,
	PROP_IDLE,
	PROP_IDLE_TIME,
	PROP_LOGIN_TIME,
	PROP_ACTIVE_STATUS,
	PROP_PRIMITIVE,
	PROP_MESSAGE,
	N_PROPERTIES
};
static GParamSpec *properties[N_PROPERTIES];

G_DEFINE_TYPE_WITH_PRIVATE(PurplePresence, purple_presence, G_TYPE_OBJECT)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
purple_presence_set_active_status(PurplePresence *presence,
                                  PurpleStatus *status)
{
	PurplePresencePrivate *priv = NULL;

	priv = purple_presence_get_instance_private(presence);

	if(g_set_object(&priv->active_status, status)) {
		GObject *obj = G_OBJECT(presence);

		g_object_freeze_notify(obj);
		g_object_notify_by_pspec(obj, properties[PROP_ACTIVE_STATUS]);
		g_object_notify_by_pspec(obj, properties[PROP_PRIMITIVE]);
		g_object_notify_by_pspec(obj, properties[PROP_MESSAGE]);
		g_object_thaw_notify(obj);
	}
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
purple_presence_set_property(GObject *obj, guint param_id, const GValue *value,
                             GParamSpec *pspec)
{
	PurplePresence *presence = PURPLE_PRESENCE(obj);

	switch (param_id) {
		case PROP_IDLE:
			purple_presence_set_idle(presence, g_value_get_boolean(value),
			                         NULL);
			break;
		case PROP_IDLE_TIME:
			purple_presence_set_idle(presence, TRUE, g_value_get_boxed(value));
			break;
		case PROP_LOGIN_TIME:
			purple_presence_set_login_time(presence, g_value_get_boxed(value));
			break;
		case PROP_ACTIVE_STATUS:
			purple_presence_set_active_status(presence,
			                                  g_value_get_object(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_presence_get_property(GObject *obj, guint param_id, GValue *value,
                             GParamSpec *pspec)
{
	PurplePresence *presence = PURPLE_PRESENCE(obj);

	switch (param_id) {
		case PROP_IDLE:
			g_value_set_boolean(value, purple_presence_is_idle(presence));
			break;
		case PROP_IDLE_TIME:
			g_value_set_boxed(value, purple_presence_get_idle_time(presence));
			break;
		case PROP_LOGIN_TIME:
			g_value_set_boxed(value, purple_presence_get_login_time(presence));
			break;
		case PROP_ACTIVE_STATUS:
			g_value_set_object(value, purple_presence_get_active_status(presence));
			break;
		case PROP_PRIMITIVE:
			g_value_set_enum(value, purple_presence_get_primitive(presence));
			break;
		case PROP_MESSAGE:
			g_value_set_string(value, purple_presence_get_message(presence));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_presence_init(PurplePresence *presence) {
	PurplePresencePrivate *priv = NULL;

	priv = purple_presence_get_instance_private(presence);

	priv->status_table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free,
	                                           NULL);
}

static void
purple_presence_finalize(GObject *obj) {
	PurplePresencePrivate *priv = NULL;

	priv = purple_presence_get_instance_private(PURPLE_PRESENCE(obj));

	g_hash_table_destroy(priv->status_table);
	g_clear_object(&priv->active_status);

	g_clear_pointer(&priv->idle_time, g_date_time_unref);
	g_clear_pointer(&priv->login_time, g_date_time_unref);

	G_OBJECT_CLASS(purple_presence_parent_class)->finalize(obj);
}

static void
purple_presence_class_init(PurplePresenceClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->get_property = purple_presence_get_property;
	obj_class->set_property = purple_presence_set_property;
	obj_class->finalize = purple_presence_finalize;

	/**
	 * PurplePresence:idle:
	 *
	 * Whether or not the presence is in an idle state.
	 */
	properties[PROP_IDLE] = g_param_spec_boolean("idle", "Idle",
				"Whether the presence is in idle state.", FALSE,
				G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurplePresence:idle-time:
	 *
	 * The time when the presence went idle.
	 */
	properties[PROP_IDLE_TIME] = g_param_spec_boxed(
				"idle-time", "Idle time",
				"The idle time of the presence",
				G_TYPE_DATE_TIME,
				G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurplePresence:login-time:
	 *
	 * The login-time of the presence.
	 */
	properties[PROP_LOGIN_TIME] = g_param_spec_boxed(
		"login-time", "Login time",
		"The login time of the presence.",
		G_TYPE_DATE_TIME,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurplePresence:active-status:
	 *
	 * The currently active status of the presence.
	 */
	properties[PROP_ACTIVE_STATUS] = g_param_spec_object("active-status",
				"Active status",
				"The active status for the presence.", PURPLE_TYPE_STATUS,
				G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurplePresence:primitive:
	 *
	 * The [enum@Purple.StatusPrimitive] for this presence.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_PRIMITIVE] = g_param_spec_enum(
		"primitive", "primitive",
		"The primitive for the presence",
		PURPLE_TYPE_STATUS_PRIMITIVE,
		PURPLE_STATUS_UNSET,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurplePresence:message:
	 *
	 * The status message of the presence.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_MESSAGE] = g_param_spec_string(
		"message", "message",
		"The status message",
		NULL,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
PurplePresence *
purple_presence_new(void) {
	return g_object_new(PURPLE_TYPE_PRESENCE, NULL);
}

void
purple_presence_set_status_active(PurplePresence *presence,
                                  const gchar *status_id, gboolean active)
{
	PurpleStatus *status = NULL;

	g_return_if_fail(PURPLE_IS_PRESENCE(presence));
	g_return_if_fail(status_id != NULL);

	status = purple_presence_get_status(presence, status_id);

	g_return_if_fail(PURPLE_IS_STATUS(status));
	/* TODO: Should we do the following? */
	/* g_return_if_fail(active == status->active); */

	if(purple_status_is_exclusive(status)) {
		if(!active) {
			purple_debug_warning("presence",
					"Attempted to set a non-independent status "
					"(%s) inactive. Only independent statuses "
					"can be specifically marked inactive.",
					status_id);
			return;
		}
	}

	purple_status_set_active(status, active);
}

void
purple_presence_switch_status(PurplePresence *presence, const gchar *status_id)
{
	purple_presence_set_status_active(presence, status_id, TRUE);
}

void
purple_presence_set_idle(PurplePresence *presence, gboolean idle,
                         GDateTime *idle_time)
{
	PurplePresencePrivate *priv = NULL;
	PurplePresenceClass *klass = NULL;
	gboolean old_idle;
	GObject *obj = NULL;

	g_return_if_fail(PURPLE_IS_PRESENCE(presence));

	priv = purple_presence_get_instance_private(presence);
	klass = PURPLE_PRESENCE_GET_CLASS(presence);

	if (priv->idle == idle && priv->idle_time == idle_time) {
		return;
	}

	old_idle = priv->idle;
	priv->idle = idle;

	g_clear_pointer(&priv->idle_time, g_date_time_unref);
	if(idle && idle_time != NULL) {
		priv->idle_time = g_date_time_ref(idle_time);
	}

	obj = G_OBJECT(presence);
	g_object_freeze_notify(obj);
	g_object_notify_by_pspec(obj, properties[PROP_IDLE]);
	g_object_notify_by_pspec(obj, properties[PROP_IDLE_TIME]);
	g_object_thaw_notify(obj);

	if(klass->update_idle) {
		klass->update_idle(presence, old_idle);
	}
}

void
purple_presence_set_login_time(PurplePresence *presence, GDateTime *login_time)
{
	PurplePresencePrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_PRESENCE(presence));

	priv = purple_presence_get_instance_private(presence);

	if(priv->login_time != NULL && login_time != NULL) {
		if(g_date_time_equal(priv->login_time, login_time)) {
			return;
		}
	}

	if(priv->login_time != NULL) {
		g_date_time_unref(priv->login_time);
	}

	if(login_time != NULL) {
		priv->login_time = g_date_time_ref(login_time);
	} else {
		priv->login_time = NULL;
	}

	g_object_notify_by_pspec(G_OBJECT(presence), properties[PROP_LOGIN_TIME]);
}

GList *
purple_presence_get_statuses(PurplePresence *presence) {
	PurplePresenceClass *klass = NULL;

	g_return_val_if_fail(PURPLE_IS_PRESENCE(presence), NULL);

	klass = PURPLE_PRESENCE_GET_CLASS(presence);
	if(klass && klass->get_statuses) {
		return klass->get_statuses(presence);
	}

	return NULL;
}

PurpleStatus *
purple_presence_get_status(PurplePresence *presence, const gchar *status_id) {
	PurplePresencePrivate *priv = NULL;
	PurpleStatus *status = NULL;
	GList *l = NULL;

	g_return_val_if_fail(PURPLE_IS_PRESENCE(presence), NULL);
	g_return_val_if_fail(status_id != NULL, NULL);

	priv = purple_presence_get_instance_private(presence);

	/* What's the purpose of this hash table? */
	status = (PurpleStatus *)g_hash_table_lookup(priv->status_table,
	                                             status_id);

	if(status == NULL) {
		for(l = purple_presence_get_statuses(presence);
			l != NULL && status == NULL; l = l->next)
		{
			PurpleStatus *temp_status = l->data;

			if (purple_strequal(status_id, purple_status_get_id(temp_status))) {
				status = temp_status;
			}
		}

		if(status != NULL) {
			g_hash_table_insert(priv->status_table,
								g_strdup(purple_status_get_id(status)), status);
		}
	}

	return status;
}

PurpleStatus *
purple_presence_get_active_status(PurplePresence *presence) {
	PurplePresencePrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_PRESENCE(presence), NULL);

	priv = purple_presence_get_instance_private(presence);

	return priv->active_status;
}

gboolean
purple_presence_is_available(PurplePresence *presence) {
	PurpleStatus *status = NULL;

	g_return_val_if_fail(PURPLE_IS_PRESENCE(presence), FALSE);

	status = purple_presence_get_active_status(presence);

	return ((status != NULL && purple_status_is_available(status)) &&
			!purple_presence_is_idle(presence));
}

gboolean
purple_presence_is_online(PurplePresence *presence) {
	PurpleStatus *status = NULL;

	g_return_val_if_fail(PURPLE_IS_PRESENCE(presence), FALSE);

	if((status = purple_presence_get_active_status(presence)) == NULL) {
		return FALSE;
	}

	return purple_status_is_online(status);
}

gboolean
purple_presence_is_status_active(PurplePresence *presence,
                                 const gchar *status_id)
{
	PurpleStatus *status = NULL;

	g_return_val_if_fail(PURPLE_IS_PRESENCE(presence), FALSE);
	g_return_val_if_fail(status_id != NULL, FALSE);

	status = purple_presence_get_status(presence, status_id);

	return (status != NULL && purple_status_is_active(status));
}

gboolean
purple_presence_is_status_primitive_active(PurplePresence *presence,
                                           PurpleStatusPrimitive primitive)
{
	GList *l = NULL;

	g_return_val_if_fail(PURPLE_IS_PRESENCE(presence), FALSE);
	g_return_val_if_fail(primitive != PURPLE_STATUS_UNSET, FALSE);

	for(l = purple_presence_get_statuses(presence); l != NULL; l = l->next) {
		PurpleStatus *temp_status = l->data;
		PurpleStatusType *type = purple_status_get_status_type(temp_status);

		if(purple_status_type_get_primitive(type) == primitive &&
		    purple_status_is_active(temp_status))
		{
			return TRUE;
		}
	}

	return FALSE;
}

gboolean
purple_presence_is_idle(PurplePresence *presence) {
	PurplePresencePrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_PRESENCE(presence), FALSE);

	if(!purple_presence_is_online(presence)) {
		return FALSE;
	}

	priv = purple_presence_get_instance_private(presence);

	return priv->idle;
}

GDateTime *
purple_presence_get_idle_time(PurplePresence *presence) {
	PurplePresencePrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_PRESENCE(presence), NULL);

	priv = purple_presence_get_instance_private(presence);

	return priv->idle_time;
}

GDateTime *
purple_presence_get_login_time(PurplePresence *presence) {
	PurplePresencePrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_PRESENCE(presence), 0);

	priv = purple_presence_get_instance_private(presence);

	return priv->login_time;
}

gint
purple_presence_compare(PurplePresence *presence1, PurplePresence *presence2) {
	GDateTime *idle1 = NULL;
	GDateTime *idle2 = NULL;
	GDateTime *now = NULL;
	GTimeSpan diff1 = 0;
	GTimeSpan diff2 = 0;

	if(presence1 == presence2) {
		return 0;
	} else if (presence1 == NULL) {
		return 1;
	} else if (presence2 == NULL) {
		return -1;
	}

	if(purple_presence_is_online(presence1) &&
	   !purple_presence_is_online(presence2))
	{
		return -1;
	} else if(purple_presence_is_online(presence2) &&
	          !purple_presence_is_online(presence1))
	{
		return 1;
	}

	idle1 = purple_presence_get_idle_time(presence1);
	idle2 = purple_presence_get_idle_time(presence2);

	if(idle1 == NULL && idle2 == NULL) {
		return 0;
	} else if(idle1 == NULL && idle2 != NULL) {
		return -1;
	} else if(idle1 != NULL && idle2 == NULL) {
		return 1;
	}

	now = g_date_time_new_now_local();
	diff1 = g_date_time_difference(now, idle1);
	diff2 = g_date_time_difference(now, idle2);
	g_date_time_unref(now);

	if(diff1 > diff2) {
		return 1;
	} else if (diff1 < diff2) {
		return -1;
	}

	return 0;
}

PurpleStatusPrimitive
purple_presence_get_primitive(PurplePresence *presence) {
	PurplePresencePrivate *priv = NULL;
	PurpleStatusType *type = NULL;

	g_return_val_if_fail(PURPLE_IS_PRESENCE(presence), PURPLE_STATUS_UNSET);

	priv = purple_presence_get_instance_private(presence);

	type = purple_status_get_status_type(priv->active_status);
	if(type != NULL) {
		return purple_status_type_get_primitive(type);
	}

	return PURPLE_STATUS_UNSET;
}

const char *
purple_presence_get_message(PurplePresence *presence) {
	PurplePresencePrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_PRESENCE(presence), NULL);

	priv = purple_presence_get_instance_private(presence);

	return purple_status_get_attr_string(priv->active_status, "message");
}
