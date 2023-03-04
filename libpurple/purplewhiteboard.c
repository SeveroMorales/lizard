/*
 * purple
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 *
 */

#include "purplewhiteboard.h"

#include "purpleprotocol.h"
#include "purpleprotocolwhiteboard.h"
#include "purplewhiteboarduiops.h"
#include "util.h"

typedef struct {
	int state;

	PurpleAccount *account;
	gchar *id;

	/* TODO Remove this and use protocol-specific subclasses. */
	void *proto_data;

	PurpleWhiteboardOps *protocol_ops;

	GList *draw_list;
} PurpleWhiteboardPrivate;

/* GObject Property enums */
enum {
	PROP_0,
	PROP_STATE,
	PROP_ACCOUNT,
	PROP_ID,
	PROP_DRAW_LIST,
	N_PROPERTIES,
};
static GParamSpec *properties[N_PROPERTIES] = { NULL, };

G_DEFINE_TYPE_WITH_PRIVATE(PurpleWhiteboard, purple_whiteboard, G_TYPE_OBJECT)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
purple_whiteboard_set_account(PurpleWhiteboard *whiteboard,
                              PurpleAccount *account)
{
	PurpleWhiteboardPrivate *priv = NULL;

	priv = purple_whiteboard_get_instance_private(whiteboard);

	if(g_set_object(&priv->account, account)) {
		g_object_notify_by_pspec(G_OBJECT(whiteboard),
		                         properties[PROP_ACCOUNT]);
	}
}

static void
purple_whiteboard_set_id(PurpleWhiteboard *whiteboard, const gchar *id) {
	PurpleWhiteboardPrivate *priv = NULL;

	priv = purple_whiteboard_get_instance_private(whiteboard);

	g_free(priv->id);
	priv->id = g_strdup(id);

	g_object_notify_by_pspec(G_OBJECT(whiteboard), properties[PROP_ID]);
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
purple_whiteboard_set_property(GObject *obj, guint param_id,
                               const GValue *value, GParamSpec *pspec)
{
	PurpleWhiteboard *whiteboard = PURPLE_WHITEBOARD(obj);

	switch(param_id) {
		case PROP_STATE:
			purple_whiteboard_set_state(whiteboard, g_value_get_int(value));
			break;
		case PROP_ACCOUNT:
			purple_whiteboard_set_account(whiteboard,
			                              g_value_get_object(value));
			break;
		case PROP_ID:
			purple_whiteboard_set_id(whiteboard, g_value_get_string(value));
			break;
		case PROP_DRAW_LIST:
			purple_whiteboard_set_draw_list(whiteboard,
			                                g_value_get_pointer(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_whiteboard_get_property(GObject *obj, guint param_id, GValue *value,
                               GParamSpec *pspec)
{
	PurpleWhiteboard *whiteboard = PURPLE_WHITEBOARD(obj);

	switch (param_id) {
		case PROP_STATE:
			g_value_set_int(value, purple_whiteboard_get_state(whiteboard));
			break;
		case PROP_ACCOUNT:
			g_value_set_object(value,
			                   purple_whiteboard_get_account(whiteboard));
			break;
		case PROP_ID:
			g_value_set_string(value,
			                   purple_whiteboard_get_id(whiteboard));
			break;
		case PROP_DRAW_LIST:
			g_value_set_pointer(value,
			                    purple_whiteboard_get_draw_list(whiteboard));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_whiteboard_init(G_GNUC_UNUSED PurpleWhiteboard *whiteboard) {
}

static void
purple_whiteboard_constructed(GObject *object) {
	PurpleWhiteboard *whiteboard = PURPLE_WHITEBOARD(object);
	PurpleWhiteboardPrivate *priv = NULL;
	PurpleProtocol *protocol = NULL;

	G_OBJECT_CLASS(purple_whiteboard_parent_class)->constructed(object);

	priv = purple_whiteboard_get_instance_private(whiteboard);

	protocol = purple_connection_get_protocol(
				purple_account_get_connection(priv->account));
	purple_whiteboard_set_protocol_ops(whiteboard,
				purple_protocol_get_whiteboard_ops(protocol));

	/* Start up protocol specifics */
	if(priv->protocol_ops != NULL && priv->protocol_ops->start != NULL) {
		priv->protocol_ops->start(whiteboard);
	}
}

static void
purple_whiteboard_finalize(GObject *object) {
	PurpleWhiteboard *whiteboard = PURPLE_WHITEBOARD(object);
	PurpleWhiteboardPrivate *priv = NULL;

	priv = purple_whiteboard_get_instance_private(whiteboard);

	/* Do protocol specific session ending procedures */
	if(priv->protocol_ops != NULL && priv->protocol_ops->end != NULL) {
		priv->protocol_ops->end(whiteboard);
	}

	g_clear_object(&priv->account);
	g_clear_pointer(&priv->id, g_free);

	/* TODO: figure out how we need to clean up the drawlist */

	G_OBJECT_CLASS(purple_whiteboard_parent_class)->finalize(object);
}

static void
purple_whiteboard_class_init(PurpleWhiteboardClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->get_property = purple_whiteboard_get_property;
	obj_class->set_property = purple_whiteboard_set_property;
	obj_class->finalize = purple_whiteboard_finalize;
	obj_class->constructed = purple_whiteboard_constructed;

	properties[PROP_STATE] = g_param_spec_int(
		"state", "State",
		"State of the whiteboard.",
		G_MININT, G_MAXINT, 0,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

	properties[PROP_ACCOUNT] = g_param_spec_object(
		"account", "Account",
		"The whiteboard's account.", PURPLE_TYPE_ACCOUNT,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	properties[PROP_ID] = g_param_spec_string(
		"id", "id",
		"The ID of the whiteboard.", NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	properties[PROP_DRAW_LIST] = g_param_spec_pointer(
		"draw-list", "Draw list",
		"A list of points to draw to the buddy.",
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

/******************************************************************************
 * API
 *****************************************************************************/
void
purple_whiteboard_set_protocol_ops(PurpleWhiteboard *whiteboard,
                                   PurpleWhiteboardOps *ops)
{
	PurpleWhiteboardPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_WHITEBOARD(whiteboard));

	priv = purple_whiteboard_get_instance_private(whiteboard);

	priv->protocol_ops = ops;
}

PurpleAccount *
purple_whiteboard_get_account(PurpleWhiteboard *whiteboard) {
	PurpleWhiteboardPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_WHITEBOARD(whiteboard), NULL);

	priv = purple_whiteboard_get_instance_private(whiteboard);

	return priv->account;
}

const gchar *
purple_whiteboard_get_id(PurpleWhiteboard *whiteboard) {
	PurpleWhiteboardPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_WHITEBOARD(whiteboard), NULL);

	priv = purple_whiteboard_get_instance_private(whiteboard);

	return priv->id;
}

void
purple_whiteboard_set_state(PurpleWhiteboard *whiteboard, int state) {
	PurpleWhiteboardPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_WHITEBOARD(whiteboard));

	priv = purple_whiteboard_get_instance_private(whiteboard);

	priv->state = state;

	g_object_notify_by_pspec(G_OBJECT(whiteboard), properties[PROP_STATE]);
}

gint
purple_whiteboard_get_state(PurpleWhiteboard *whiteboard) {
	PurpleWhiteboardPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_WHITEBOARD(whiteboard), -1);

	priv = purple_whiteboard_get_instance_private(whiteboard);

	return priv->state;
}

void
purple_whiteboard_start(PurpleWhiteboard *whiteboard) {
	purple_whiteboard_ui_ops_create(whiteboard);
}

void
purple_whiteboard_draw_list_destroy(GList *draw_list) {
	g_list_free(draw_list);
}

gboolean
purple_whiteboard_get_dimensions(PurpleWhiteboard *whiteboard, gint *width,
                                 gint *height)
{
	PurpleWhiteboardPrivate *priv = NULL;
	PurpleWhiteboardOps *protocol_ops = NULL;

	g_return_val_if_fail(PURPLE_IS_WHITEBOARD(whiteboard), FALSE);

	priv = purple_whiteboard_get_instance_private(whiteboard);
	protocol_ops = priv->protocol_ops;

	if(protocol_ops != NULL && protocol_ops->get_dimensions != NULL) {
		protocol_ops->get_dimensions(whiteboard, width, height);

		return TRUE;
	}

	return FALSE;
}

void
purple_whiteboard_set_dimensions(PurpleWhiteboard *whiteboard, gint width,
                                 gint height)
{
	purple_whiteboard_ui_ops_set_dimensions(whiteboard, width, height);
}

void
purple_whiteboard_send_draw_list(PurpleWhiteboard *whiteboard, GList *list) {
	PurpleWhiteboardPrivate *priv = NULL;
	PurpleWhiteboardOps *protocol_ops = NULL;

	g_return_if_fail(PURPLE_IS_WHITEBOARD(whiteboard));

	priv = purple_whiteboard_get_instance_private(whiteboard);
	protocol_ops = priv->protocol_ops;

	if(protocol_ops != NULL && protocol_ops->send_draw_list != NULL) {
		protocol_ops->send_draw_list(whiteboard, list);
	}
}

void
purple_whiteboard_draw_point(PurpleWhiteboard *whiteboard, gint x, gint y,
                             gint color, gint size)
{
	purple_whiteboard_ui_ops_draw_point(whiteboard, x, y, color, size);
}

void
purple_whiteboard_draw_line(PurpleWhiteboard *whiteboard, gint x1, gint y1,
                            gint x2, gint y2, gint color, gint size)
{
	purple_whiteboard_ui_ops_draw_line(whiteboard, x1, y1, x2, y2, color,
	                                   size);
}

void
purple_whiteboard_clear(PurpleWhiteboard *whiteboard) {
	purple_whiteboard_ui_ops_clear(whiteboard);
}

void
purple_whiteboard_send_clear(PurpleWhiteboard *whiteboard) {
	PurpleWhiteboardPrivate *priv = NULL;
	PurpleWhiteboardOps *protocol_ops = NULL;

	g_return_if_fail(PURPLE_IS_WHITEBOARD(whiteboard));

	priv = purple_whiteboard_get_instance_private(whiteboard);
	protocol_ops = priv->protocol_ops;

	if(protocol_ops != NULL && protocol_ops->clear != NULL) {
		protocol_ops->clear(whiteboard);
	}
}

void
purple_whiteboard_send_brush(PurpleWhiteboard *whiteboard, gint size,
                             gint color)
{
	PurpleWhiteboardPrivate *priv = NULL;
	PurpleWhiteboardOps *protocol_ops = NULL;

	g_return_if_fail(PURPLE_IS_WHITEBOARD(whiteboard));

	priv = purple_whiteboard_get_instance_private(whiteboard);
	protocol_ops = priv->protocol_ops;

	if(protocol_ops != NULL && protocol_ops->set_brush != NULL) {
		protocol_ops->set_brush(whiteboard, size, color);
	}
}

gboolean
purple_whiteboard_get_brush(PurpleWhiteboard *whiteboard, gint *size,
                            gint *color)
{
	PurpleWhiteboardPrivate *priv = NULL;
	PurpleWhiteboardOps *protocol_ops = NULL;

	g_return_val_if_fail(PURPLE_IS_WHITEBOARD(whiteboard), FALSE);

	priv = purple_whiteboard_get_instance_private(whiteboard);
	protocol_ops = priv->protocol_ops;

	if(protocol_ops != NULL && protocol_ops->get_brush != NULL) {
		protocol_ops->get_brush(whiteboard, size, color);

		return TRUE;
	}

	return FALSE;
}

void
purple_whiteboard_set_brush(PurpleWhiteboard *whiteboard, gint size,
                            gint color)
{
	purple_whiteboard_ui_ops_set_brush(whiteboard, size, color);
}

GList *
purple_whiteboard_get_draw_list(PurpleWhiteboard *whiteboard) {
	PurpleWhiteboardPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_WHITEBOARD(whiteboard), NULL);

	priv = purple_whiteboard_get_instance_private(whiteboard);

	return priv->draw_list;
}

void
purple_whiteboard_set_draw_list(PurpleWhiteboard *whiteboard,
                                GList* draw_list)
{
	PurpleWhiteboardPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_WHITEBOARD(whiteboard));

	priv = purple_whiteboard_get_instance_private(whiteboard);
	priv->draw_list = draw_list;

	g_object_notify_by_pspec(G_OBJECT(whiteboard), properties[PROP_DRAW_LIST]);
}

void
purple_whiteboard_set_protocol_data(PurpleWhiteboard *whiteboard,
                                    gpointer proto_data)
{
	PurpleWhiteboardPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_WHITEBOARD(whiteboard));

	priv = purple_whiteboard_get_instance_private(whiteboard);
	priv->proto_data = proto_data;
}

gpointer
purple_whiteboard_get_protocol_data(PurpleWhiteboard *whiteboard) {
	PurpleWhiteboardPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_WHITEBOARD(whiteboard), NULL);

	priv = purple_whiteboard_get_instance_private(whiteboard);

	return priv->proto_data;
}

PurpleWhiteboard *
purple_whiteboard_new(PurpleAccount *account, const gchar *id, gint state) {
	PurpleWhiteboard *whiteboard = NULL;
	PurpleProtocol *protocol = NULL;

	g_return_val_if_fail(PURPLE_IS_ACCOUNT(account), NULL);
	g_return_val_if_fail(id != NULL, NULL);

	protocol = purple_account_get_protocol(account);

	g_return_val_if_fail(PURPLE_IS_PROTOCOL(protocol), NULL);

	if(PURPLE_IS_PROTOCOL_WHITEBOARD(protocol)) {
		whiteboard = purple_protocol_whiteboard_create(
			PURPLE_PROTOCOL_WHITEBOARD(protocol), account, id, state);
	} else {
		whiteboard = g_object_new(PURPLE_TYPE_WHITEBOARD,
			"account", account,
			"id", id,
			"state", state,
			NULL
		);
	}

	return whiteboard;
}

gboolean
purple_whiteboard_equal(PurpleWhiteboard *whiteboard1,
                        PurpleWhiteboard *whiteboard2)
{
	PurpleWhiteboardPrivate *priv1 = NULL, *priv2 = NULL;

	if(whiteboard1 == NULL) {
		return (whiteboard2 == NULL);
	} else if(whiteboard2 == NULL) {
		return FALSE;
	}

	priv1 = purple_whiteboard_get_instance_private(whiteboard1);
	priv2 = purple_whiteboard_get_instance_private(whiteboard2);

	return purple_strequal(priv1->id, priv2->id);
}
