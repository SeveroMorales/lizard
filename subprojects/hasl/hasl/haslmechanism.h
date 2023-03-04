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

#ifndef HASL_MECHANISM_H
#define HASL_MECHANISM_H

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define HASL_TYPE_MECHANISM (hasl_mechanism_get_type())
G_DECLARE_DERIVABLE_TYPE(HaslMechanism, hasl_mechanism, HASL, MECHANISM,
                         GObject)

/**
 * HaslMechanismResult:
 * @success: The mechanism completed successfully.
 * @continue: The mechanism has additional steps to perform.
 * @error: The mechanism encountered an error.
 *
 * The result states for [method@Hasl.Mechanism.step].
 *
 * Since: 0.1.0
 */
typedef enum {
	HASL_MECHANISM_RESULT_SUCCESS = 0,
	HASL_MECHANISM_RESULT_CONTINUE,
	HASL_MECHANISM_RESULT_ERROR,
} HaslMechanismResult;

#include <hasl/haslcontext.h>

struct _HaslMechanismClass {
	/*< private >*/
	GObjectClass parent;

	/*< public >*/
	gboolean (*possible)(HaslMechanism *mechanism, HaslContext *ctx, GError **error);

	HaslMechanismResult (*step)(HaslMechanism *mechanism, HaslContext *ctx, const guint8 *server_in, gsize server_in_length, guint8 **client_out, gsize *client_out_length, GError **error);

	/*< private >*/
	gpointer reserved[8];
};

/**
 * hasl_mechanism_step:
 * @mechanism: The instance.
 * @ctx: The [class@Hasl.Context] instance.
 * @server_in: (nullable): Input from the server.
 * @server_in_length: The length of @server_in in bytes.
 * @client_out: (transfer full): The data that the client should use.
 * @client_out_length: (out) (optional): The length of @client_out in bytes.
 * @error: (out): A return address for an error.
 *
 * Tells @mechanism to perform it's next step. If @mechanism is complete it
 * should return %TRUE. If the mechanism was not successfully, it should return
 * %TRUE but set @error.
 *
 * If %FALSE is returned, @ctx will continue passing messages to @mechanism
 * until @mechanism says it's done by returning %TRUE or another error is
 * encountered.
 *
 * Returns: A [enum@Hasl.MechanismResult] with @error possibly set.
 *
 * Since: 0.1.0
 */
HaslMechanismResult hasl_mechanism_step(HaslMechanism *mechanism, HaslContext *ctx, const guint8 *server_in, gsize server_in_length, guint8 **client_out, gsize *client_out_length, GError **error);

/**
 * hasl_mechanism_possible:
 * @mechanism: The instance.
 * @ctx: The [class@Hasl.Context] instance.
 * @error: The return address for a #GError.
 *
 * This is used by @ctx to determine if it should even attempt @mechanism.
 *
 * Determines whether or not the information in @ctx is there for @mechanism
 * to work.
 *
 * For example, the PLAIN mechanism should only be attempted if
 * [property@Hasl.Context:tls] is %TRUE or if
 * [property@Hasl.Context:allow-clear-text] is %TRUE.
 *
 * Mechanisms should also implement this checking authzid, username, and
 * password as necessary to avoid multiple round trips with the server that we
 * know aren't going to work.
 *
 * Returns: %TRUE if @mechanism should be attempted otherwise %FALSE with
 *          @error optionally set.
 */
gboolean hasl_mechanism_possible(HaslMechanism *mechanism, HaslContext *ctx, GError **error);

G_END_DECLS

#endif /* HASL_MECHANISM_H */
