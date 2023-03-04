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

#ifndef HASL_CONTEXT_H
#define HASL_CONTEXT_H

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define HASL_CONTEXT_DOMAIN (g_quark_from_static_string("hasl-context"))

#define HASL_TYPE_CONTEXT (hasl_context_get_type())
G_DECLARE_FINAL_TYPE(HaslContext, hasl_context, HASL, CONTEXT, GObject)

#include <hasl/haslmechanism.h>

/**
 * hasl_context_new:
 *
 * Creates a new context.
 *
 * Returns: (transfer full): The new context.
 *
 * Since: 0.1.0
 */
HaslContext *hasl_context_new(void);

/**
 * hasl_context_get_allowed_mechanisms:
 * @ctx: The instance.
 *
 * Gets the list of allowed mechanisms for @ctx.
 *
 * Returns: The list of allowed mechanisms.
 *
 * Since: 0.1.0
 */
const char *hasl_context_get_allowed_mechanisms(HaslContext *ctx);

/**
 * hasl_context_set_allowed_mechanisms:
 * @ctx: The instance.
 * @allowed_mechanisms: The list of mechanisms.
 *
 * Sets the list of mechanism that are allowed for @ctx.
 *
 * @allowed_mechanisms can be white space or comma separated.
 *
 * Since: 0.1.0
 */
void hasl_context_set_allowed_mechanisms(HaslContext *ctx, const char *allowed_mechanisms);

/**
 * hasl_context_get_authzid:
 * @ctx: The instance.
 *
 * Gets the authzid of @ctx. See [property@Hasl.Context:authzid] for more
 * information.
 *
 * Returns: (nullable): The authzid if one is set, otherwise %NULL.
 *
 * Since: 0.1.0
 */
const char *hasl_context_get_authzid(HaslContext *ctx);

/**
 * hasl_context_set_authzid:
 * @ctx: The instance.
 * @authzid: (nullable): The new authzid.
 *
 * Sets the authzid of @ctx to @authzid. See [property@Hasl.Context:authzid]
 * for more information.
 *
 * Since: 0.1.0
 */
void hasl_context_set_authzid(HaslContext *ctx, const char *authzid);

/**
 * hasl_context_get_username:
 * @ctx: The instance.
 *
 * Gets the username that has been set on @ctx.
 *
 * Returns: (nullable): The username if set, otherwise %NULL.
 *
 * Since: 0.1.0
 */
const char *hasl_context_get_username(HaslContext *ctx);

/**
 * hasl_context_set_username:
 * @ctx: The instance.
 * @username: (nullable): The new username.
 *
 * Sets the username of @ctx to @username.
 *
 * Since: 0.1.0
 */
void hasl_context_set_username(HaslContext *ctx, const char *username);

/**
 * hasl_context_get_password:
 * @ctx: The instance.
 *
 * Gets the password from @ctx.
 *
 * Returns: (nullable): The password if set, otherwise %NULL.
 *
 * Since: 0.1.0
 */
const char *hasl_context_get_password(HaslContext *ctx);

/**
 * hasl_context_set_password:
 * @ctx: The instance.
 * @password: (nullable): The new password.
 *
 * Sets the password for @ctx to @password.
 *
 * Since: 0.1.0
 */
void hasl_context_set_password(HaslContext *ctx, const char *password);

/**
 * hasl_context_get_tls:
 * @ctx: The instance.
 *
 * Gets whether or not @ctx is working with a TLS connection.
 *
 * See [property@Hasl.Context:tls] for more information.
 *
 * Returns: %TRUE if @ctx is on a TLS connection, otherwise %FALSE.
 *
 * Since: 0.1.0
 */
gboolean hasl_context_get_tls(HaslContext *ctx);

/**
 * hasl_context_set_tls:
 * @ctx: The instance.
 * @tls: Whether or not @ctx is working with a TLS connection.
 *
 * Sets whether or not @ctx is working with a TLS connection to @tls.
 *
 * Since: 0.1.0
 */
void hasl_context_set_tls(HaslContext *ctx, gboolean tls);

/**
 * hasl_context_get_allow_clear_text:
 * @ctx: The instance.
 *
 * Gets whether or not it's okay to use mechanisms that use clear text with a
 * non-TLS connection. Clear text methods are ones that use credentials
 * directly like the `PLAIN` mechanism which depend on TLS to secure the
 * credentials.
 *
 * See [property@Hasl.Context:allow-clear-text] for more information.
 *
 * Returns: %TRUE if clear text methods are allowed.
 *
 * Since: 0.1.0
 */
gboolean hasl_context_get_allow_clear_text(HaslContext *ctx);

/**
 * hasl_context_set_allow_clear_text:
 * @ctx: The instance.
 * @allow_clear_text: Whether or not to allow the mechanisms that depend on
 *                    clear text.
 *
 * Sets whether or not mechanisms that depend on clear text credentials are
 * allowed. The `PLAIN` mechanism is an example of a clear text based mechanism
 * as it depends on TLS to secure the credentials.
 *
 * See [property@Hasl.Context:allow-clear-text] for more information.
 *
 * Since: 0.1.0
 */
void hasl_context_set_allow_clear_text(HaslContext *ctx, gboolean allow_clear_text);

/**
 * hasl_context_add_mechanism:
 * @ctx: The instance.
 * @name: The name of the mechanism.
 * @type: The type of the mechanism.
 *
 * Registers @type in @ctx with the given @name. @name should follow the
 * convention from
 * [RFC4422§3.1](https://www.rfc-editor.org/rfc/rfc4422#section-3.1).
 *
 * @type is the [type@GObject.Type] of the [class@Hasl.Mechanism] subclass.
 *
 * See the following example:
 *
 * ```c
 * hasl_context_add_mechanism(ctx, "PLAIN", HASL_TYPE_MECHANISM_PLAIN);
 * ```
 *
 * This only needs to be called for custom mechanisms, all of the built in
 * mechanism are already added to all contexts by default. However, you may
 * replace them by name if you need to adapt to a server that isn't RFC
 * compliant.
 *
 * Returns: %TRUE if the mechanism was added, otherwise %FALSE.
 *
 * Since: 0.1.0
 */
gboolean hasl_context_add_mechanism(HaslContext *ctx, const char *name, GType type);

/**
 * hasl_context_get_supported_mechanisms:
 * @ctx: The instance.
 *
 * Gets a space separated alphabetically sorted list of all the mechanisms that
 * @ctx knows about.
 *
 * Returns: (transfer full): The list of mechanisms.
 *
 * Since: 0.1.0
 */
char *hasl_context_get_supported_mechanisms(HaslContext *ctx);

/**
 * hasl_context_get_current_mechanism:
 * @ctx: The instance.
 *
 * Gets the name of the current mechanism that @ctx is attempting.
 *
 * If [method@hasl.Context.next] has not yet been called or if all mechanisms
 * in [property@hasl.Context:allowed-mechanisms] have been exhausted then %NULL
 * will be returned.
 *
 * Returns: The name of the current mechanism or %NULL.
 *
 * Since: 0.1.0
 */
const char *hasl_context_get_current_mechanism(HaslContext *ctx);

/**
 * hasl_context_next:
 * @ctx: The instance.
 *
 * Ask the context to move to the next possible mechanism. The context will
 * create an instance of the mechanism to use internally and return the name
 * that should be given to the server.
 *
 * Returns: (nullable): The name of the next mechanism to try or %NULL if all
 *          mechanisms have been exhausted.
 *
 * Since: 0.1.0
 */
const char *hasl_context_next(HaslContext *ctx);

/**
 * hasl_context_step:
 * @ctx: The instance.
 * @server_in: (nullable): Input from the server.
 * @server_in_length: The length of @server_in in bytes.
 * @client_out: (transfer full): The data that the client should use.
 * @client_out_length: (out) (optional): The length of @client_out in bytes.
 * @error: (out): A return address for an error.
 *
 * Calls [method@Hasl.Mechanism.step] for the currently active mechansim in
 * @ctx.
 *
 * If there is no active mechanism because [method@Hasl.Context.next] has not
 * been called or all mechanisms have been exhausted this will return
 * [enum@Hasl.MechanismResult.FAILURE] with @error set.
 *
 * Otherwise the return value depends on the current state of the mechanism
 * that is being attempted.
 *
 * Returns: a [enum@Hasl.MechanismResult] on how to proceed.
 *
 * Since: 0.1.0
 */
HaslMechanismResult hasl_context_step(HaslContext *ctx, const guint8 *server_in, gsize server_in_length, guint8 **client_out, gsize *client_out_length, GError **error);

G_END_DECLS

#endif /* HASL_CONTEXT_H */
