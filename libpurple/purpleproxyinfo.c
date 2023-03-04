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

#include "purpleproxyinfo.h"

#include "purpleenums.h"

struct _PurpleProxyInfo {
    GObject parent;

    PurpleProxyType proxy_type;
    gchar *hostname;
    gint port;
    gchar *username;
    gchar *password;
};

enum {
    PROP_0,
    PROP_PROXY_TYPE,
    PROP_HOSTNAME,
    PROP_PORT,
    PROP_USERNAME,
    PROP_PASSWORD,
    N_PROPERTIES
};
static GParamSpec *properties[N_PROPERTIES] = {NULL, };

G_DEFINE_TYPE(PurpleProxyInfo, purple_proxy_info, G_TYPE_OBJECT)

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
purple_proxy_info_get_property(GObject *obj, guint param_id, GValue *value,
                               GParamSpec *pspec)
{
    PurpleProxyInfo *info = PURPLE_PROXY_INFO(obj);

    switch(param_id) {
        case PROP_PROXY_TYPE:
            g_value_set_enum(value, purple_proxy_info_get_proxy_type(info));
            break;
        case PROP_HOSTNAME:
            g_value_set_string(value, purple_proxy_info_get_hostname(info));
            break;
        case PROP_PORT:
            g_value_set_int(value, purple_proxy_info_get_port(info));
            break;
        case PROP_USERNAME:
            g_value_set_string(value, purple_proxy_info_get_username(info));
            break;
        case PROP_PASSWORD:
            g_value_set_string(value, purple_proxy_info_get_password(info));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
            break;
    }
}

static void
purple_proxy_info_set_property(GObject *obj, guint param_id,
                               const GValue *value, GParamSpec *pspec)
{
    PurpleProxyInfo *info = PURPLE_PROXY_INFO(obj);

    switch(param_id) {
        case PROP_PROXY_TYPE:
            purple_proxy_info_set_proxy_type(info, g_value_get_enum(value));
            break;
        case PROP_HOSTNAME:
            purple_proxy_info_set_hostname(info, g_value_get_string(value));
            break;
        case PROP_PORT:
            purple_proxy_info_set_port(info, g_value_get_int(value));
            break;
        case PROP_USERNAME:
            purple_proxy_info_set_username(info, g_value_get_string(value));
            break;
        case PROP_PASSWORD:
            purple_proxy_info_set_password(info, g_value_get_string(value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
            break;
    }
}

static void
purple_proxy_info_finalize(GObject *obj) {
    PurpleProxyInfo *info = PURPLE_PROXY_INFO(obj);

    g_free(info->hostname);
    g_free(info->username);
    g_free(info->password);

    G_OBJECT_CLASS(purple_proxy_info_parent_class)->finalize(obj);
}

static void
purple_proxy_info_init(G_GNUC_UNUSED PurpleProxyInfo *info) {
}

static void
purple_proxy_info_class_init(PurpleProxyInfoClass *klass) {
    GObjectClass *obj_class = G_OBJECT_CLASS(klass);

    obj_class->get_property = purple_proxy_info_get_property;
    obj_class->set_property = purple_proxy_info_set_property;
    obj_class->finalize = purple_proxy_info_finalize;

    /**
     * PurpleProxyInfo::proxy-type:
     *
     * The [enum@ProxyType] to use for this proxy.
     *
     * Since: 3.0.0
     */
    properties[PROP_PROXY_TYPE] = g_param_spec_enum(
        "proxy-type", "proxy-type",
        "The proxy type for this proxy.",
        PURPLE_TYPE_PROXY_TYPE,
        PURPLE_PROXY_TYPE_USE_GLOBAL,
        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    /**
     * PurpleProxyInfo::hostname:
     *
     * The hostname to use for this proxy.
     *
     * Since: 3.0.0
     */
    properties[PROP_HOSTNAME] = g_param_spec_string(
        "hostname", "hostname",
        "The hostname for this proxy.",
        NULL,
        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    /**
     * PurpleProxyInfo::port:
     *
     * The port to use for this proxy.
     *
     * Since: 3.0.0
     */
    properties[PROP_PORT] = g_param_spec_int(
        "port", "port",
        "The port for this proxy.",
        0, 65535, 0,
        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    /**
     * PurpleProxyInfo::username:
     *
     * The username to use for this proxy.
     *
     * Since: 3.0.0
     */
    properties[PROP_USERNAME] = g_param_spec_string(
        "username", "username",
        "The username for this proxy.",
        NULL,
        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    /**
     * PurpleProxyInfo::password:
     *
     * The password to use for this proxy.
     *
     * Since: 3.0.0
     */
    properties[PROP_PASSWORD] = g_param_spec_string(
        "password", "password",
        "The password for this proxy.",
        NULL,
        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);


    g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
PurpleProxyInfo *
purple_proxy_info_new(void) {
    return g_object_new(PURPLE_TYPE_PROXY_INFO, NULL);
}

void
purple_proxy_info_set_proxy_type(PurpleProxyInfo *info,
                                 PurpleProxyType proxy_type)
{
    g_return_if_fail(PURPLE_IS_PROXY_INFO(info));

    info->proxy_type = proxy_type;

    g_object_notify_by_pspec(G_OBJECT(info), properties[PROP_PROXY_TYPE]);
}

PurpleProxyType
purple_proxy_info_get_proxy_type(PurpleProxyInfo *info) {
    g_return_val_if_fail(PURPLE_IS_PROXY_INFO(info), PURPLE_PROXY_TYPE_NONE);

    return info->proxy_type;
}

void
purple_proxy_info_set_hostname(PurpleProxyInfo *info, const gchar *hostname) {
    g_return_if_fail(PURPLE_IS_PROXY_INFO(info));

    g_free(info->hostname);
    info->hostname = g_strdup(hostname);

    g_object_notify_by_pspec(G_OBJECT(info), properties[PROP_HOSTNAME]);
}

const gchar *
purple_proxy_info_get_hostname(PurpleProxyInfo *info) {
    g_return_val_if_fail(PURPLE_IS_PROXY_INFO(info), NULL);

    return info->hostname;
}

void
purple_proxy_info_set_port(PurpleProxyInfo *info, gint port) {
    g_return_if_fail(PURPLE_IS_PROXY_INFO(info));

    info->port = port;

    g_object_notify_by_pspec(G_OBJECT(info), properties[PROP_PORT]);
}

gint
purple_proxy_info_get_port(PurpleProxyInfo *info) {
    g_return_val_if_fail(PURPLE_IS_PROXY_INFO(info), 0);

    return info->port;
}

void
purple_proxy_info_set_username(PurpleProxyInfo *info, const gchar *username) {
    g_return_if_fail(PURPLE_IS_PROXY_INFO(info));

    g_free(info->username);
    info->username = g_strdup(username);

    g_object_notify_by_pspec(G_OBJECT(info), properties[PROP_USERNAME]);
}

const gchar *
purple_proxy_info_get_username(PurpleProxyInfo *info) {
    g_return_val_if_fail(PURPLE_IS_PROXY_INFO(info), NULL);

    return info->username;
}

void
purple_proxy_info_set_password(PurpleProxyInfo *info, const gchar *password) {
    g_return_if_fail(PURPLE_IS_PROXY_INFO(info));

    g_free(info->password);
    info->password = g_strdup(password);

    g_object_notify_by_pspec(G_OBJECT(info), properties[PROP_PASSWORD]);
}

const gchar *
purple_proxy_info_get_password(PurpleProxyInfo *info) {
    g_return_val_if_fail(PURPLE_IS_PROXY_INFO(info), NULL);

    return info->password;
}
