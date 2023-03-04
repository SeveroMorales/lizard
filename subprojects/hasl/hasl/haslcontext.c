/*
 * Copyright (C) 2023 Hasl Developers
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

#include "haslcontext.h"

#include "haslmechanismexternal.h"
#include "haslmechanismplain.h"

enum {
	PROP_0,
	PROP_ALLOWED_MECHANISMS,
	PROP_AUTHZID,
	PROP_USERNAME,
	PROP_PASSWORD,
	PROP_TLS,
	PROP_ALLOW_CLEAR_TEXT,
	N_PROPERTIES,
};
static GParamSpec *properties[N_PROPERTIES] = {NULL, };

struct _HaslContext {
	GObject parent;

	char *allowed_mechanisms;
	char *authzid;
	char *username;
	char *password;

	gboolean tls;
	gboolean allow_clear_text;

	GHashTable *mechanisms;

	GStrv available_mechanisms;
	guint available_mechanisms_count;
	gint current_mechanism_index;
	HaslMechanism *current_mechanism;
};

G_DEFINE_TYPE(HaslContext, hasl_context, G_TYPE_OBJECT)

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
hasl_context_get_property(GObject *obj, guint param_id, GValue *value,
                          GParamSpec *pspec)
{
	HaslContext *ctx = HASL_CONTEXT(obj);

	switch(param_id) {
	case PROP_ALLOWED_MECHANISMS:
		g_value_set_string(value, hasl_context_get_allowed_mechanisms(ctx));
		break;
	case PROP_AUTHZID:
		g_value_set_string(value, hasl_context_get_authzid(ctx));
		break;
	case PROP_USERNAME:
		g_value_set_string(value, hasl_context_get_username(ctx));
		break;
	case PROP_PASSWORD:
		g_value_set_string(value, hasl_context_get_password(ctx));
		break;
	case PROP_TLS:
		g_value_set_boolean(value, hasl_context_get_tls(ctx));
		break;
	case PROP_ALLOW_CLEAR_TEXT:
		g_value_set_boolean(value, hasl_context_get_allow_clear_text(ctx));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
		break;
	}
}

static void
hasl_context_set_property(GObject *obj, guint param_id, const GValue *value,
                          GParamSpec *pspec)
{
	HaslContext *ctx = HASL_CONTEXT(obj);

	switch(param_id) {
	case PROP_ALLOWED_MECHANISMS:
		hasl_context_set_allowed_mechanisms(ctx, g_value_get_string(value));
		break;
	case PROP_AUTHZID:
		hasl_context_set_authzid(ctx, g_value_get_string(value));
		break;
	case PROP_USERNAME:
		hasl_context_set_username(ctx, g_value_get_string(value));
		break;
	case PROP_PASSWORD:
		hasl_context_set_password(ctx, g_value_get_string(value));
		break;
	case PROP_TLS:
		hasl_context_set_tls(ctx, g_value_get_boolean(value));
		break;
	case PROP_ALLOW_CLEAR_TEXT:
		hasl_context_set_allow_clear_text(ctx, g_value_get_boolean(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
		break;
	}
}

static void
hasl_context_init(HaslContext *ctx) {
	/* The mechanisms hash table is keyed on the name and has a value that is
	 * the GType of the HaslMechanism implementation.
	 */
	ctx->mechanisms = g_hash_table_new_full(g_str_hash, g_str_equal, g_free,
	                                        NULL);

	/* The current_mechanism_index needs to start at -1 so the first call to
	 * next will set it to 0.
	 */
	ctx->current_mechanism_index = -1;

	hasl_context_add_mechanism(ctx, "EXTERNAL", HASL_TYPE_MECHANISM_EXTERNAL);
	hasl_context_add_mechanism(ctx, "PLAIN", HASL_TYPE_MECHANISM_PLAIN);
}

static void
hasl_context_finalize(GObject *obj) {
	HaslContext *ctx = HASL_CONTEXT(obj);

	g_clear_pointer(&ctx->allowed_mechanisms, g_free);
	g_clear_pointer(&ctx->authzid, g_free);
	g_clear_pointer(&ctx->username, g_free);
	g_clear_pointer(&ctx->password, g_free);

	g_clear_pointer(&ctx->mechanisms, g_hash_table_unref);

	g_clear_pointer(&ctx->available_mechanisms, g_strfreev);
	g_clear_object(&ctx->current_mechanism);

	G_OBJECT_CLASS(hasl_context_parent_class)->finalize(obj);
}

static void
hasl_context_class_init(HaslContextClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->finalize = hasl_context_finalize;
	obj_class->get_property = hasl_context_get_property;
	obj_class->set_property = hasl_context_set_property;

	/**
	 * HaslContext:allowed-mechanisms:
	 *
	 * A list of mechanisms that are allowed to be used by this context. This
	 * list can be either white space or comma separated.
	 *
	 * Since: 0.1.0
	 */
	properties[PROP_ALLOWED_MECHANISMS] = g_param_spec_string(
		"allowed-mechanisms", "allowed-mechanism",
		"The list of mechanisms allowed in this context",
		NULL,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * HaslContext:authzid
	 *
	 * The identifier for the user who is authenticating. This are times when
	 * an administrator wants to authenticate as a user, so they property will
	 * be set to the username of the user to authenticate as, but the username
	 * and password fields will be proper values for the administrator.
	 *
	 * Since: 0.1.0
	 */
	properties[PROP_AUTHZID] = g_param_spec_string(
		"authzid", "authzid",
		"The id of the user to authenticate as",
		NULL,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * HaslContext:username:
	 *
	 * The username of the user that is authenticating for mechanisms that need
	 * it.
	 *
	 * Since: 0.1.0
	 */
	properties[PROP_USERNAME] = g_param_spec_string(
		"username", "username",
		"The username of the user authenticating",
		NULL,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * HaslContext:password:
	 *
	 * The password to use for mechanisms that need it.
	 *
	 * Since: 0.1.0
	 */
	properties[PROP_PASSWORD] = g_param_spec_string(
		"password", "password",
		"The password of the user authenticating",
		NULL,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * HaslContext:tls:
	 *
	 * Tells the context whether or not the connection is being ran on is over
	 * TLS or not.
	 *
	 * This is used in conjunction with
	 * [property@Hasl.Context:allow-clear-text] to determine if mechanisms that
	 * depend on the transport layer to secure credentials should be attempted.
	 *
	 * Since: 0.1.0
	 */
	properties[PROP_TLS] = g_param_spec_boolean(
		"tls", "tls",
		"Whether or not the connection is over TLS",
		FALSE,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * HaslContext:allow-clear-text:
	 *
	 * Determines where or not mechanisms that depend on transport layer
	 * encryption like TLS are allowed to be used on a non-TLS connection.
	 *
	 * If [property@Hasl.Context:tls] is %FALSE, then these mechanisms, like
	 * the `PLAIN` mechanism will only be attempted if this property is %TRUE.
	 */
	properties[PROP_ALLOW_CLEAR_TEXT] = g_param_spec_boolean(
		"allow-clear-text", "allow-clear-text",
		"Whether or not to allow mechanisms that use clear text without TLS",
		FALSE,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
HaslContext *
hasl_context_new(void) {
	return g_object_new(HASL_TYPE_CONTEXT, NULL);
}

const char *
hasl_context_get_allowed_mechanisms(HaslContext *ctx) {
	g_return_val_if_fail(HASL_IS_CONTEXT(ctx), NULL);

	return ctx->allowed_mechanisms;
}

void
hasl_context_set_allowed_mechanisms(HaslContext *ctx,
                                    const char *allowed_mechanisms)
{
	g_return_if_fail(HASL_IS_CONTEXT(ctx));
	g_return_if_fail(allowed_mechanisms != NULL);

	if(g_strcmp0(allowed_mechanisms, ctx->allowed_mechanisms) != 0) {
		g_free(ctx->allowed_mechanisms);
		ctx->allowed_mechanisms = g_strdup(allowed_mechanisms);

		ctx->available_mechanisms = g_strsplit_set(ctx->allowed_mechanisms,
		                                           ", ", -1);
		ctx->available_mechanisms_count = g_strv_length(ctx->available_mechanisms);

		/* Since the allowed mechanisms has been changed, we need to reset
		 * our index counter as well.
		 */
		ctx->current_mechanism_index = -1;

		g_object_notify_by_pspec(G_OBJECT(ctx),
		                         properties[PROP_ALLOWED_MECHANISMS]);
	}
}

const char *
hasl_context_get_authzid(HaslContext *ctx) {
	g_return_val_if_fail(HASL_IS_CONTEXT(ctx), NULL);

	return ctx->authzid;
}

void
hasl_context_set_authzid(HaslContext *ctx, const char *authzid) {
	g_return_if_fail(HASL_IS_CONTEXT(ctx));

	if(g_strcmp0(authzid, ctx->authzid) != 0) {
		g_free(ctx->authzid);
		ctx->authzid = g_strdup(authzid);

		g_object_notify_by_pspec(G_OBJECT(ctx), properties[PROP_AUTHZID]);
	}
}

const char *
hasl_context_get_username(HaslContext *ctx) {
	g_return_val_if_fail(HASL_IS_CONTEXT(ctx), NULL);

	return ctx->username;
}

void
hasl_context_set_username(HaslContext *ctx, const char *username) {
	g_return_if_fail(HASL_IS_CONTEXT(ctx));

	if(g_strcmp0(username, ctx->username) != 0) {
		g_free(ctx->username);
		ctx->username = g_strdup(username);

		g_object_notify_by_pspec(G_OBJECT(ctx), properties[PROP_USERNAME]);
	}
}

const char *
hasl_context_get_password(HaslContext *ctx) {
	g_return_val_if_fail(HASL_IS_CONTEXT(ctx), NULL);

	return ctx->password;
}

void
hasl_context_set_password(HaslContext *ctx, const char *password) {
	g_return_if_fail(HASL_IS_CONTEXT(ctx));

	if(g_strcmp0(password, ctx->password) != 0) {
		g_free(ctx->password);
		ctx->password = g_strdup(password);

		g_object_notify_by_pspec(G_OBJECT(ctx), properties[PROP_PASSWORD]);
	}
}

gboolean
hasl_context_get_tls(HaslContext *ctx) {
	g_return_val_if_fail(HASL_IS_CONTEXT(ctx), FALSE);

	return ctx->tls;
}

void
hasl_context_set_tls(HaslContext *ctx, gboolean tls) {
	g_return_if_fail(HASL_IS_CONTEXT(ctx));

	if(tls != ctx->tls) {
		ctx->tls = tls;

		g_object_notify_by_pspec(G_OBJECT(ctx), properties[PROP_TLS]);
	}
}

gboolean
hasl_context_get_allow_clear_text(HaslContext *ctx) {
	g_return_val_if_fail(HASL_IS_CONTEXT(ctx), FALSE);

	return ctx->allow_clear_text;
}

void
hasl_context_set_allow_clear_text(HaslContext *ctx,
                                  gboolean allow_clear_text)
{
	g_return_if_fail(HASL_IS_CONTEXT(ctx));

	if(allow_clear_text != ctx->allow_clear_text) {
		ctx->allow_clear_text = allow_clear_text;

		g_object_notify_by_pspec(G_OBJECT(ctx),
		                         properties[PROP_ALLOW_CLEAR_TEXT]);
	}
}

gboolean
hasl_context_add_mechanism(HaslContext *ctx, const char *name, GType type) {
	g_return_val_if_fail(HASL_IS_CONTEXT(ctx), FALSE);
	g_return_val_if_fail(name != NULL && name[0] != '\0', FALSE);
	g_return_val_if_fail(g_type_is_a(type, HASL_TYPE_MECHANISM), FALSE);

	return g_hash_table_insert(ctx->mechanisms, g_strdup(name),
	                           GSIZE_TO_POINTER(type));
}

char *
hasl_context_get_supported_mechanisms(HaslContext *ctx) {
	GString *str = NULL;
	GList *names = NULL;
	gboolean first = TRUE;

	g_return_val_if_fail(HASL_IS_CONTEXT(ctx), NULL);

	names = g_hash_table_get_keys(ctx->mechanisms);
	names = g_list_sort(names, (GCompareFunc)g_strcmp0);

	str = g_string_new("");

	while(names != NULL) {
		if(first) {
			first = FALSE;
		} else {
			g_string_append(str, " ");
		}

		g_string_append(str, (char *)names->data);

		names = g_list_delete_link(names, names);
	}

	return g_string_free(str, FALSE);
}

const char *
hasl_context_get_current_mechanism(HaslContext *ctx) {
	g_return_val_if_fail(HASL_IS_CONTEXT(ctx), NULL);

	/* hasl_context_next hasn't been called yet. */
	if(ctx->current_mechanism_index < 0) {
		return NULL;
	}

	/* hasl_context_set_allowed_mechanisms hasn't been called. */
	if(ctx->available_mechanisms == NULL) {
		return NULL;
	}

	if((guint)ctx->current_mechanism_index < ctx->available_mechanisms_count) {
		return ctx->available_mechanisms[ctx->current_mechanism_index];
	}

	return NULL;
}

const char *
hasl_context_next(HaslContext *ctx) {
	g_return_val_if_fail(HASL_IS_CONTEXT(ctx), NULL);

	if(ctx->available_mechanisms == NULL) {
		return NULL;
	}

	/* Now loop through the allowed mechanism until we find one that is
	 * possible or we've exhausted the list.
	 */
	while(TRUE) {
		GError *error = NULL;
		GType mechanism_type = G_TYPE_INVALID;
		const char *mechanism_name = NULL;
		gint index = 0;
		gpointer value;

		/* Free the previous mechanism as we're done with it. */
		g_clear_object(&ctx->current_mechanism);

		/* Make sure we have another item before we increment our internal
		 * counter to make sure we can't overflow.
		 */
		index = ctx->current_mechanism_index + 1;
		ctx->current_mechanism_index = index;

		if((guint)index >= ctx->available_mechanisms_count) {
			return NULL;
		}

		mechanism_name = ctx->available_mechanisms[index];

		/* Find the type of the mechanism. */
		value = g_hash_table_lookup(ctx->mechanisms, mechanism_name);
		if(value == NULL) {
			continue;
		}

		/* Validate that the type is a HaslMechanism. */
		mechanism_type = GPOINTER_TO_SIZE(value);
		if(!g_type_is_a(mechanism_type, HASL_TYPE_MECHANISM)) {
			continue;
		}

		/* Create an instance of the mechanism and check if it's possible to
		 * use it.
		 */
		ctx->current_mechanism = g_object_new(mechanism_type, NULL);
		if(!hasl_mechanism_possible(ctx->current_mechanism, ctx, &error)) {
			g_message("skipping mechanism '%s': %s", mechanism_name,
			          (error != NULL) ? error->message : "unknown error");

			continue;
		}

		/* The mechanism should be usable, so return its name. */
		return mechanism_name;
	}

	return NULL;
}

HaslMechanismResult
hasl_context_step(HaslContext *ctx, const guint8 *server_in,
                  gsize server_in_length, guint8 **client_out,
                  gsize *client_out_length, GError **error)
{

	g_return_val_if_fail(HASL_IS_CONTEXT(ctx), HASL_MECHANISM_RESULT_ERROR);

	if(!HASL_IS_MECHANISM(ctx->current_mechanism)) {
		g_set_error(error, HASL_CONTEXT_DOMAIN, 0,
 		            "current mechanism is NULL");

		return HASL_MECHANISM_RESULT_ERROR;
	}

	return hasl_mechanism_step(ctx->current_mechanism, ctx,
	                           server_in, server_in_length,
	                           client_out, client_out_length,
	                           error);
}
