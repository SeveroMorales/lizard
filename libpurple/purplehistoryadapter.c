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

#include "purplehistoryadapter.h"

#include "purpleprivate.h"

typedef struct {
	gchar *id;
	gchar *name;
} PurpleHistoryAdapterPrivate;

enum {
	PROP_0,
	PROP_ID,
	PROP_NAME,
	N_PROPERTIES,
};
static GParamSpec *properties[N_PROPERTIES] = {NULL, };

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(PurpleHistoryAdapter,
                                    purple_history_adapter, G_TYPE_OBJECT)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
purple_history_adapter_set_id(PurpleHistoryAdapter *adapter, const gchar *id) {
	PurpleHistoryAdapterPrivate *priv = NULL;

	priv = purple_history_adapter_get_instance_private(adapter);

	g_free(priv->id);
	priv->id = g_strdup(id);

	g_object_notify_by_pspec(G_OBJECT(adapter), properties[PROP_ID]);
}

static void
purple_history_adapter_set_name(PurpleHistoryAdapter *adapter,
                                const gchar *name)
{
	PurpleHistoryAdapterPrivate *priv = NULL;

	priv = purple_history_adapter_get_instance_private(adapter);

	g_free(priv->name);
	priv->name = g_strdup(name);

	g_object_notify_by_pspec(G_OBJECT(adapter), properties[PROP_NAME]);
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
purple_history_adapter_get_property(GObject *obj, guint param_id,
                                    GValue *value, GParamSpec *pspec)
{
	PurpleHistoryAdapter *adapter = PURPLE_HISTORY_ADAPTER(obj);

	switch(param_id) {
		case PROP_ID:
			g_value_set_string(value,
			                   purple_history_adapter_get_id(adapter));
			break;
		case PROP_NAME:
			g_value_set_string(value,
			                   purple_history_adapter_get_name(adapter));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_history_adapter_set_property(GObject *obj, guint param_id,
                                    const GValue *value, GParamSpec *pspec)
{
	PurpleHistoryAdapter *adapter = PURPLE_HISTORY_ADAPTER(obj);

	switch(param_id) {
		case PROP_ID:
			purple_history_adapter_set_id(adapter,
			                              g_value_get_string(value));
			break;
		case PROP_NAME:
			purple_history_adapter_set_name(adapter,
			                                g_value_get_string(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_history_adapter_finalize(GObject *obj) {
	PurpleHistoryAdapter *adapter = NULL;
	PurpleHistoryAdapterPrivate *priv = NULL;

	adapter = PURPLE_HISTORY_ADAPTER(obj);
	priv = purple_history_adapter_get_instance_private(adapter);

	g_clear_pointer(&priv->id, g_free);
	g_clear_pointer(&priv->name, g_free);

	G_OBJECT_CLASS(purple_history_adapter_parent_class)->finalize(obj);
}

static void
purple_history_adapter_init(G_GNUC_UNUSED PurpleHistoryAdapter *adapter) {
}

static void
purple_history_adapter_class_init(PurpleHistoryAdapterClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->get_property = purple_history_adapter_get_property;
	obj_class->set_property = purple_history_adapter_set_property;
	obj_class->finalize = purple_history_adapter_finalize;

	/**
	 * PurpleHistoryAdapter::id:
	 *
	 * The ID of the adapter.  Used for preferences and other things that need
	 * to address it.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_ID] = g_param_spec_string(
		"id", "id", "The identifier of the adapter",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS
	);

	/**
	 * PurpleHistoryAdapter::name:
	 *
	 * The name of the adapter.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_NAME] = g_param_spec_string(
		"name", "name", "The name of the adapter",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS
	);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

/******************************************************************************
 * Private API
 *****************************************************************************/
gboolean
purple_history_adapter_activate(PurpleHistoryAdapter *adapter, GError **error)
{
	PurpleHistoryAdapterClass *klass = NULL;

	g_return_val_if_fail(PURPLE_IS_HISTORY_ADAPTER(adapter), FALSE);

	klass = PURPLE_HISTORY_ADAPTER_GET_CLASS(adapter);
	if(klass != NULL && klass->activate != NULL) {
		return klass->activate(adapter, error);
	}

	g_set_error(error, PURPLE_HISTORY_ADAPTER_DOMAIN, 0,
	            "%s does not implement the activate function.",
	            G_OBJECT_TYPE_NAME(G_OBJECT(adapter)));

	return FALSE;
}

gboolean
purple_history_adapter_deactivate(PurpleHistoryAdapter *adapter,
                                  GError **error)
{
	PurpleHistoryAdapterClass *klass = NULL;

	g_return_val_if_fail(PURPLE_IS_HISTORY_ADAPTER(adapter), FALSE);

	klass = PURPLE_HISTORY_ADAPTER_GET_CLASS(adapter);
	if(klass != NULL && klass->deactivate != NULL) {
		return klass->deactivate(adapter, error);
	}

	g_set_error(error, PURPLE_HISTORY_ADAPTER_DOMAIN, 0,
	            "%s does not implement the deactivate function.",
	            G_OBJECT_TYPE_NAME(G_OBJECT(adapter)));

	return FALSE;
}

/******************************************************************************
 * Public API
 *****************************************************************************/
const gchar *
purple_history_adapter_get_id(PurpleHistoryAdapter *adapter) {
	PurpleHistoryAdapterPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_HISTORY_ADAPTER(adapter), NULL);

	priv = purple_history_adapter_get_instance_private(adapter);

	return priv->id;
}

const gchar *
purple_history_adapter_get_name(PurpleHistoryAdapter *adapter) {
	PurpleHistoryAdapterPrivate  *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_HISTORY_ADAPTER(adapter), NULL);

	priv = purple_history_adapter_get_instance_private(adapter);

	return priv->name;
}

GList *
purple_history_adapter_query(PurpleHistoryAdapter *adapter,
                             const gchar *query,
                             GError **error)
{
	PurpleHistoryAdapterClass *klass = NULL;

	g_return_val_if_fail(PURPLE_IS_HISTORY_ADAPTER(adapter), NULL);
	g_return_val_if_fail(query != NULL, NULL);

	klass = PURPLE_HISTORY_ADAPTER_GET_CLASS(adapter);
	if(klass != NULL && klass->query != NULL) {
		return klass->query(adapter, query, error);
	}

	g_set_error(error, PURPLE_HISTORY_ADAPTER_DOMAIN, 0,
	            "%s does not implement the query function.",
	            G_OBJECT_TYPE_NAME(G_OBJECT(adapter)));

	return NULL;
}

gboolean
purple_history_adapter_remove(PurpleHistoryAdapter *adapter,
                              const gchar *query,
                              GError **error)
{
	PurpleHistoryAdapterClass *klass = NULL;

	g_return_val_if_fail(PURPLE_IS_HISTORY_ADAPTER(adapter), FALSE);
	klass = PURPLE_HISTORY_ADAPTER_GET_CLASS(adapter);

	if(klass != NULL && klass->remove != NULL) {
		return klass->remove(adapter, query, error);
	}

	g_set_error(error, PURPLE_HISTORY_ADAPTER_DOMAIN, 0,
	            "%s does not implement the remove function.",
	            G_OBJECT_TYPE_NAME(G_OBJECT(adapter)));

	return FALSE;
}

gboolean
purple_history_adapter_write(PurpleHistoryAdapter *adapter,
                             PurpleConversation *conversation,
                             PurpleMessage *message,
                             GError **error)
{
	PurpleHistoryAdapterClass *klass = NULL;

	g_return_val_if_fail(PURPLE_IS_HISTORY_ADAPTER(adapter), FALSE);
	g_return_val_if_fail(PURPLE_IS_MESSAGE(message), FALSE);
	g_return_val_if_fail(PURPLE_IS_CONVERSATION(conversation), FALSE);

	klass = PURPLE_HISTORY_ADAPTER_GET_CLASS(adapter);
	if(klass != NULL && klass->write != NULL) {
		return klass->write(adapter, conversation, message, error);
	}

	g_set_error(error, PURPLE_HISTORY_ADAPTER_DOMAIN, 0,
	            "%s does not implement the write function.",
	            G_OBJECT_TYPE_NAME(G_OBJECT(adapter)));

	return FALSE;
}
