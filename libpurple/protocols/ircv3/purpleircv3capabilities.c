/*
 * Purple - Internet Messaging Library
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
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

#include "purpleircv3capabilities.h"

#include "purpleircv3connection.h"
#include "purpleircv3core.h"
#include "purpleircv3sasl.h"

#define PURPLE_IRCV3_CAPABILITIES_VERSION "302"

enum {
	PROP_0,
	PROP_CONNECTION,
	N_PROPERTIES,
};
static GParamSpec *properties[N_PROPERTIES] = {NULL, };

/* Windows is does something weird with signal handling that includes defining
 * SIG_ACK. We don't care about that here, so we undef it if we find it.
 * See https://learn.microsoft.com/en-us/cpp/c-runtime-library/signal-action-constants?view=msvc-170
 */
#ifdef SIG_ACK
# undef SIG_ACK
#endif /* SIG_ACK */

enum {
	SIG_READY,
	SIG_ACK,
	SIG_NAK,
	SIG_DONE,
	SIG_NEW,
	SIG_DEL,
	N_SIGNALS,
};
static guint signals[N_SIGNALS] = {0, };

struct _PurpleIRCv3Capabilities {
	GObject parent;

	PurpleIRCv3Connection *connection;

	GHashTable *caps;
	GPtrArray *requests;

	gatomicrefcount wait_counters;
};

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
purple_ircv3_capabilities_set_connection(PurpleIRCv3Capabilities *capabilities,
                                         PurpleIRCv3Connection *connection)
{
	g_return_if_fail(PURPLE_IRCV3_IS_CAPABILITIES(capabilities));

	if(g_set_object(&capabilities->connection, connection)) {
		g_object_notify_by_pspec(G_OBJECT(capabilities),
		                         properties[PROP_CONNECTION]);
	}
}

static void
purple_ircv3_capabilities_finish(PurpleIRCv3Capabilities *capabilities) {
	purple_ircv3_connection_writef(capabilities->connection,
	                               "CAP END");

	g_signal_emit(capabilities, signals[SIG_DONE], 0);
}

static void
purple_ircv3_capabilities_add(PurpleIRCv3Capabilities *capabilities,
                              const char *capability)
{
	char *equals = g_strstr_len(capability, -1, "=");

	if(equals != NULL) {
		char *key = g_strndup(capability, equals - capability);
		char *value = g_strdup(equals + 1);

		g_hash_table_insert(capabilities->caps, key, value);
	} else {
		g_hash_table_insert(capabilities->caps, g_strdup(capability), NULL);
	}
}

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static void
ircv3_capabilities_message_tags_ack_cb(PurpleIRCv3Capabilities *capabilities,
                                       G_GNUC_UNUSED const char *capability,
                                       G_GNUC_UNUSED gpointer data)
{
	/* We have message tags so add the stuff we support that depends on it. */
	purple_ircv3_capabilities_lookup_and_request(capabilities, "msgid");
}

/******************************************************************************
 * PurpleIRCv3Capabilities Implementation
 *****************************************************************************/
static void
purple_ircv3_capabilities_default_ready_cb(PurpleIRCv3Capabilities *capabilities)
{
	PurpleAccount *account = NULL;
	PurpleConnection *purple_connection = NULL;

	purple_connection = PURPLE_CONNECTION(capabilities->connection);
	account = purple_connection_get_account(purple_connection);

	/* Don't request the sasl capability unless the user has selected the
	 * require-password option.
	 */
	if(purple_account_get_require_password(account)) {
		gboolean found = FALSE;

		purple_ircv3_capabilities_lookup(capabilities, "sasl", &found);

		if(found) {
			purple_ircv3_sasl_request(capabilities);
		}
	}

	/* cap-notify is implied when we use CAP LS 302, so this is really just to
	 * make sure it's requested.
	 */
	purple_ircv3_capabilities_lookup_and_request(capabilities, "cap-notify");

	/* message-tags is used for a lot of stuff so we need to tell everyone we
	 * do in fact support it.
	 */
	if(purple_ircv3_capabilities_lookup_and_request(capabilities,
	                                                "message-tags"))
	{
		g_signal_connect(capabilities, "ack::message-tags",
		                 G_CALLBACK(ircv3_capabilities_message_tags_ack_cb),
		                 NULL);
	}
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_DYNAMIC_TYPE(PurpleIRCv3Capabilities, purple_ircv3_capabilities,
                      G_TYPE_OBJECT)

static void
purple_ircv3_capabilities_get_property(GObject *obj, guint param_id,
                                       GValue *value, GParamSpec *pspec)
{
	PurpleIRCv3Capabilities *capabilities = PURPLE_IRCV3_CAPABILITIES(obj);

	switch(param_id) {
		case PROP_CONNECTION:
			g_value_set_object(value,
			                   purple_ircv3_capabilities_get_connection(capabilities));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_ircv3_capabilities_set_property(GObject *obj, guint param_id,
                                       const GValue *value, GParamSpec *pspec)
{
	PurpleIRCv3Capabilities *capabilities = PURPLE_IRCV3_CAPABILITIES(obj);

	switch(param_id) {
		case PROP_CONNECTION:
			purple_ircv3_capabilities_set_connection(capabilities,
			                                         g_value_get_object(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_ircv3_capabilities_dispose(GObject *obj) {
	PurpleIRCv3Capabilities *capabilities = PURPLE_IRCV3_CAPABILITIES(obj);

	g_clear_object(&capabilities->connection);

	G_OBJECT_CLASS(purple_ircv3_capabilities_parent_class)->dispose(obj);
}

static void
purple_ircv3_capabilities_finalize(GObject *obj) {
	PurpleIRCv3Capabilities *capabilities = PURPLE_IRCV3_CAPABILITIES(obj);

	g_hash_table_destroy(capabilities->caps);
	g_ptr_array_free(capabilities->requests, TRUE);

	G_OBJECT_CLASS(purple_ircv3_capabilities_parent_class)->finalize(obj);
}

static void
purple_ircv3_capabilities_init(PurpleIRCv3Capabilities *capabilities) {
	capabilities->caps = g_hash_table_new_full(g_str_hash, g_str_equal, g_free,
	                                           g_free);
	capabilities->requests = g_ptr_array_new_full(0, g_free);

	g_atomic_ref_count_init(&capabilities->wait_counters);
}

static void
purple_ircv3_capabilities_class_finalize(G_GNUC_UNUSED PurpleIRCv3CapabilitiesClass *klass) {
}

static void
purple_ircv3_capabilities_class_init(PurpleIRCv3CapabilitiesClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->dispose = purple_ircv3_capabilities_dispose;
	obj_class->finalize = purple_ircv3_capabilities_finalize;
	obj_class->get_property = purple_ircv3_capabilities_get_property;
	obj_class->set_property = purple_ircv3_capabilities_set_property;

	/**
	 * PurpleIRCv3Capabilities:connection:
	 *
	 * The PurpleIRCv3Connection object that this capabilities was created
	 * with.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_CONNECTION] = g_param_spec_object(
		"connection", "connection",
		"The connection this capabilities was created for.",
		PURPLE_IRCV3_TYPE_CONNECTION,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);

	/**
	 * PurpleIRCv3Capabilities::ready:
	 * @capabilities: The instance.
	 *
	 * Emitted when @capabilities has finished receiving the list of
	 * capabilities from the server at startup.
	 *
	 * For dynamically added capabilities see the `added` and `removed`
	 * signals.
	 *
	 * Since: 3.0.0
	 */
	signals[SIG_READY] = g_signal_new_class_handler(
		"ready",
		G_OBJECT_CLASS_TYPE(klass),
		G_SIGNAL_RUN_LAST,
		G_CALLBACK(purple_ircv3_capabilities_default_ready_cb),
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		0);

	/**
	 * PurpleIRCv3Capabilities::ack:
	 * @capabilities: The instance.
	 * @capability: The capability string.
	 *
	 * Emitted when the server has acknowledged a `CAP REQ` call from
	 * purple_ircv3_capabilities_request.
	 *
	 * The value of @capability will be the same as the one that was requested.
	 *
	 * Since: 3.0.0
	 */
	signals[SIG_ACK] = g_signal_new_class_handler(
		"ack",
		G_OBJECT_CLASS_TYPE(klass),
		G_SIGNAL_DETAILED | G_SIGNAL_RUN_LAST,
		NULL,
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		1,
		G_TYPE_STRING);

	/**
	 * PurpleIRCv3Capabilities::nak:
	 * @capabilities: The instance.
	 * @capability: The capability string.
	 *
	 * Emitted when the server has nacked a `CAP REQ` call from
	 * purple_ircv3_capabilities_request.
	 *
	 * The value of @capability will be the same as the one that was requested.
	 *
	 * Since: 3.0.0
	 */
	signals[SIG_NAK] = g_signal_new_class_handler(
		"nak",
		G_OBJECT_CLASS_TYPE(klass),
		G_SIGNAL_DETAILED | G_SIGNAL_RUN_LAST,
		NULL,
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		1,
		G_TYPE_STRING);

	/**
	 * PurpleIRCv3Capabilities::done:
	 * @capabilities: The instance.
	 *
	 * Emitted when all of the requested capabilities have been either ack'd or
	 * nak'd by the server.
	 *
	 * Since: 3.0.0
	 */
	signals[SIG_DONE] = g_signal_new_class_handler(
		"done",
		G_OBJECT_CLASS_TYPE(klass),
		G_SIGNAL_RUN_LAST,
		NULL,
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		0);

	/**
	 * PurpleIRCv3Capabilities::new:
	 * @capabilities: The instance.
	 * @added: The newly added capabilities.
	 *
	 * Emitted when the server sends the `CAP NEW` command. @added is a
	 * [type@GLib.Strv] of the new capabilities the server added.
	 *
	 * There are two approaches to how you can use this signal. You can check
	 * each item in @added for the values you need and parsing their values, or
	 * you can call #purple_ircv3_capabilities_lookup to see if the
	 * capabilities you're interested in have been added.
	 *
	 * Since: 3.0.0
	 */
	signals[SIG_NEW] = g_signal_new_class_handler(
		"new",
		G_OBJECT_CLASS_TYPE(klass),
		G_SIGNAL_RUN_LAST,
		NULL,
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		1,
		G_TYPE_STRV);

	/**
	 * PurpleIRCv3Capabilities::del:
	 * @capabilities: The instance.
	 * @removed: The capabilities that were removed.
	 *
	 * Emitted when the server sends the `CAP DEL` command. @removed is a
	 * [type@GLib.Strv] of the capabilities that the server removed.
	 *
	 * There are two approaches to how you can use this signal. You can check
	 * each item in @removed for the values you care about, or you can call
	 * #purple_ircv3_capabilities_lookup to see if the capabilities you're
	 * interested in have been removed.
	 *
	 * Since: 3.0.0
	 */
	signals[SIG_DEL] = g_signal_new_class_handler(
		"del",
		G_OBJECT_CLASS_TYPE(klass),
		G_SIGNAL_RUN_LAST,
		NULL,
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		1,
		G_TYPE_STRV);
}

/******************************************************************************
 * Command handlers
 *****************************************************************************/
static gboolean
purple_ircv3_capabilities_handle_list(PurpleIRCv3Capabilities *capabilities,
                                      guint n_params,
                                      GStrv params,
                                      G_GNUC_UNUSED GError **error)
{
	gboolean done = TRUE;
	gchar **parts = NULL;

	/* Check if we have more messages coming. */
	if(n_params > 1 && purple_strequal(params[0], "*")) {
		parts = g_strsplit(params[1], " ", -1);
		done = FALSE;
	} else {
		parts = g_strsplit(params[0], " ", -1);
	}

	/* Add each capability to our hash table, splitting the keys and values. */
	for(int i = 0; parts[i] != NULL; i++) {
		purple_ircv3_capabilities_add(capabilities, parts[i]);
	}

	g_strfreev(parts);

	if(done) {
		g_signal_emit(capabilities, signals[SIG_READY], 0, signals[SIG_READY]);

		/* If no capabilities were requested after we emitted the ready signal
		 * we're done with capability negotiation.
		 */
		if(capabilities->requests->len == 0) {
			purple_ircv3_capabilities_remove_wait(capabilities);
		}
	}

	return TRUE;
}

static gboolean
purple_ircv3_capabilities_handle_ack_nak(PurpleIRCv3Capabilities *capabilities,
                                         GStrv params,
                                         guint sig,
                                         const char *method,
                                         GError **error)
{
	char *caps = g_strjoinv(" ", params);
	guint index = 0;
	gboolean found = FALSE;
	gboolean ret = TRUE;

	g_signal_emit(capabilities, sig, g_quark_from_string(caps), caps);

	found = g_ptr_array_find_with_equal_func(capabilities->requests, caps,
	                                         g_str_equal, &index);
	if(found) {
		g_ptr_array_remove_index(capabilities->requests, index);
	} else {
		g_set_error(error, PURPLE_IRCV3_DOMAIN, 0,
		            "received CAP %s for unknown capability %s", method, caps);
		ret = FALSE;
	}
	g_free(caps);

	if(capabilities->requests->len == 0) {
		purple_ircv3_capabilities_remove_wait(capabilities);
	}

	return ret;
}

static gboolean
purple_ircv3_capabilities_handle_new(PurpleIRCv3Capabilities *capabilities,
                                     guint n_params,
                                     GStrv params,
                                     G_GNUC_UNUSED GError **error)
{
	for(guint i = 0; i < n_params; i++) {
		purple_ircv3_capabilities_add(capabilities, params[i]);
	}

	g_signal_emit(capabilities, signals[SIG_NEW], 0, params);

	return TRUE;
}

static gboolean
purple_ircv3_capabilities_handle_del(PurpleIRCv3Capabilities *capabilities,
                                     guint n_params,
                                     GStrv params,
                                     G_GNUC_UNUSED GError **error)
{
	for(guint i = 0; i < n_params; i++) {
		g_hash_table_remove(capabilities->caps, params[i]);
	}

	g_signal_emit(capabilities, signals[SIG_DEL], 0, params);

	return TRUE;
}

/******************************************************************************
 * Internal API
 *****************************************************************************/
void
purple_ircv3_capabilities_register(GPluginNativePlugin *plugin) {
	purple_ircv3_capabilities_register_type(G_TYPE_MODULE(plugin));
}

PurpleIRCv3Capabilities *
purple_ircv3_capabilities_new(PurpleIRCv3Connection *connection) {
	return g_object_new(
		PURPLE_IRCV3_TYPE_CAPABILITIES,
		"connection", connection,
		NULL);
}

void
purple_ircv3_capabilities_start(PurpleIRCv3Capabilities *capabilities) {
	purple_ircv3_connection_writef(capabilities->connection, "CAP LS %s",
	                               PURPLE_IRCV3_CAPABILITIES_VERSION);
}

gboolean
purple_ircv3_capabilities_message_handler(G_GNUC_UNUSED GHashTable *tags,
                                          G_GNUC_UNUSED const char *source,
                                          G_GNUC_UNUSED const char *command,
                                          guint n_params,
                                          GStrv params,
                                          GError **error,
                                          gpointer data)
{
	PurpleIRCv3Connection *connection = data;
	PurpleIRCv3Capabilities *capabilities = NULL;
	const char *subcommand = NULL;
	guint n_subparams = 0;
	GStrv subparams = NULL;

	if(n_params < 2) {
		return FALSE;
	}

	capabilities = purple_ircv3_connection_get_capabilities(connection);

	/* Initialize some variables to make it easier to call our sub command
	 * handlers.
	 *
	 * params[0] is the nick or * if it hasn't been negotiated yet, we don't
	 * have a need for this, so we ignore it.
	 *
	 * params[1] is the CAP subcommand sent from the server. We use it here
	 * purely for dispatching to our subcommand handlers.
	 *
	 * params[2] and higher are the parameters to the subcommand. To make the
	 * code a bit easier all around, we subtract 2 from n_params to remove
	 * references to the nick and subcommand name. Like wise, we add 2 to the
	 * params GStrv which will now point to the second item in the array again
	 * ignoring the nick and subcommand.
	 */
	subcommand = params[1];
	n_subparams = n_params - 2;
	subparams = params + 2;

	/* Dispatch the subcommand. */
	if(purple_strequal(subcommand, "LS") ||
	   purple_strequal(subcommand, "LIST"))
	{
		return purple_ircv3_capabilities_handle_list(capabilities, n_subparams,
		                                             subparams, error);
	} else if(purple_strequal(subcommand, "ACK")) {
		return purple_ircv3_capabilities_handle_ack_nak(capabilities,
		                                                subparams,
		                                                signals[SIG_ACK],
		                                                "ACK",
		                                                error);
	} else if(purple_strequal(subcommand, "NAK")) {
		return purple_ircv3_capabilities_handle_ack_nak(capabilities,
		                                                subparams,
		                                                signals[SIG_NAK],
		                                                "NAK",
		                                                error);
	} else if(purple_strequal(subcommand, "NEW")) {
		return purple_ircv3_capabilities_handle_new(capabilities, n_subparams,
		                                            subparams, error);
	} else if(purple_strequal(subcommand, "DEL")) {
		return purple_ircv3_capabilities_handle_del(capabilities, n_subparams,
		                                            subparams, error);
	}

	g_set_error(error, PURPLE_IRCV3_DOMAIN, 0,
	            "No handler for CAP subcommand %s", subcommand);

	return FALSE;
}

/******************************************************************************
 * Public API
 *****************************************************************************/
PurpleIRCv3Connection *
purple_ircv3_capabilities_get_connection(PurpleIRCv3Capabilities *capabilities)
{
	g_return_val_if_fail(PURPLE_IRCV3_IS_CAPABILITIES(capabilities), NULL);

	return capabilities->connection;
}

void
purple_ircv3_capabilities_request(PurpleIRCv3Capabilities *capabilities,
                                  const char *capability)
{
	g_return_if_fail(PURPLE_IRCV3_IS_CAPABILITIES(capabilities));
	g_return_if_fail(capability != NULL);

	g_ptr_array_add(capabilities->requests, g_strdup(capability));

	purple_ircv3_connection_writef(capabilities->connection, "CAP REQ :%s",
	                               capability);
}

const char *
purple_ircv3_capabilities_lookup(PurpleIRCv3Capabilities *capabilities,
                                 const char *name, gboolean *found)
{
	gpointer value = NULL;
	gboolean real_found = FALSE;

	g_return_val_if_fail(PURPLE_IRCV3_IS_CAPABILITIES(capabilities), NULL);
	g_return_val_if_fail(name != NULL, NULL);

	real_found = g_hash_table_lookup_extended(capabilities->caps, name, NULL,
	                                          &value);

	if(found != NULL) {
		*found = real_found;
	}

	return value;
}

gboolean
purple_ircv3_capabilities_lookup_and_request(PurpleIRCv3Capabilities *capabilities,
                                             const char *name)
{
	gboolean found = FALSE;

	g_return_val_if_fail(PURPLE_IRCV3_IS_CAPABILITIES(capabilities), FALSE);
	g_return_val_if_fail(name != NULL, FALSE);

	purple_ircv3_capabilities_lookup(capabilities, name, &found);
	if(found) {
		purple_ircv3_capabilities_request(capabilities, name);
	}

	return found;
}

void
purple_ircv3_capabilities_add_wait(PurpleIRCv3Capabilities *capabilities) {
	g_return_if_fail(PURPLE_IRCV3_IS_CAPABILITIES(capabilities));

	g_atomic_ref_count_inc(&capabilities->wait_counters);
}

void
purple_ircv3_capabilities_remove_wait(PurpleIRCv3Capabilities *capabilities) {
	g_return_if_fail(PURPLE_IRCV3_IS_CAPABILITIES(capabilities));

	if(g_atomic_ref_count_dec(&capabilities->wait_counters)) {
		purple_ircv3_capabilities_finish(capabilities);
	}
}
