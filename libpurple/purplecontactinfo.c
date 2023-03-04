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

#include "purplecontactinfo.h"

#include "purpleenums.h"
#include "util.h"

typedef struct  {
	char *id;

	char *username;
	char *display_name;
	char *alias;
	char *color;

	char *name_for_display;

	GdkPixbuf *avatar;

	PurplePresence *presence;

	PurpleTags *tags;

	PurplePerson *person;

	PurpleContactInfoPermission permission;
} PurpleContactInfoPrivate;

enum {
	PROP_0,
	PROP_ID,
	PROP_USERNAME,
	PROP_DISPLAY_NAME,
	PROP_ALIAS,
	PROP_COLOR,
	PROP_AVATAR,
	PROP_PRESENCE,
	PROP_TAGS,
	PROP_PERSON,
	PROP_PERMISSION,
	PROP_NAME_FOR_DISPLAY,
	N_PROPERTIES
};
static GParamSpec *properties[N_PROPERTIES] = {NULL, };

enum {
	SIG_PRESENCE_CHANGED,
	N_SIGNALS,
};
static guint signals[N_SIGNALS] = {0, };

G_DEFINE_TYPE_WITH_PRIVATE(PurpleContactInfo, purple_contact_info,
                           G_TYPE_OBJECT)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
purple_contact_info_update_name_for_display(PurpleContactInfo *info) {
	PurpleContactInfoPrivate *priv = NULL;
	const char *name_for_display = NULL;

	priv = purple_contact_info_get_instance_private(info);

	/* If the info has an alias set, use it. */
	if(name_for_display == NULL && !purple_strempty(priv->alias)) {
		name_for_display = priv->alias;
	}

	/* If info is associated with a PurplePerson that has an alias set, use the
	 * alias of that PurplePerson.
	 */
	if(name_for_display == NULL && priv->person != NULL) {
		const char *alias = purple_person_get_alias(priv->person);

		if(!purple_strempty(alias)) {
			name_for_display = alias;
		}
	}

	/* If the info has a display name set, use it. */
	if(name_for_display == NULL && !purple_strempty(priv->display_name)) {
		name_for_display = priv->display_name;
	}

	/* Fallback to the username if that is set. */
	if(name_for_display == NULL && !purple_strempty(priv->username)) {
		name_for_display = priv->username;
	}

	/* Finally, in a last ditch effort, use the id of the info. */
	if(name_for_display == NULL) {
		name_for_display = priv->id;
	}

	if(!purple_strequal(name_for_display, priv->name_for_display)) {
		/* If we have a new name for display, free the old one, dup the new one
		 * into the struct, and then emit the notify signal.
		 */
		g_free(priv->name_for_display);
		priv->name_for_display = g_strdup(name_for_display);

		g_object_notify_by_pspec(G_OBJECT(info),
		                         properties[PROP_NAME_FOR_DISPLAY]);
	}
}

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static void
purple_contact_info_person_alias_changed_cb(G_GNUC_UNUSED GObject *obj,
                                            G_GNUC_UNUSED GParamSpec *pspec,
                                            gpointer data)
{
	purple_contact_info_update_name_for_display(data);
}

/*
 * This is a notify callback on the presence for a contact info, it is used
 * to emit the presence-changed signal.
 */
static void
purple_contact_info_presence_notify_cb(GObject *source, GParamSpec *pspec,
                                       gpointer data)
{
	g_signal_emit(data, signals[SIG_PRESENCE_CHANGED],
	              g_param_spec_get_name_quark(pspec),
	              source, pspec);
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
purple_contact_info_get_property(GObject *obj, guint param_id, GValue *value,
                                 GParamSpec *pspec)
{
	PurpleContactInfo *info = PURPLE_CONTACT_INFO(obj);

	switch(param_id) {
		case PROP_ID:
			g_value_set_string(value, purple_contact_info_get_id(info));
			break;
		case PROP_USERNAME:
			g_value_set_string(value, purple_contact_info_get_username(info));
			break;
		case PROP_DISPLAY_NAME:
			g_value_set_string(value,
			                   purple_contact_info_get_display_name(info));
			break;
		case PROP_ALIAS:
			g_value_set_string(value, purple_contact_info_get_alias(info));
			break;
		case PROP_COLOR:
			g_value_set_string(value, purple_contact_info_get_color(info));
			break;
		case PROP_AVATAR:
			g_value_set_object(value, purple_contact_info_get_avatar(info));
			break;
		case PROP_PRESENCE:
			g_value_set_object(value, purple_contact_info_get_presence(info));
			break;
		case PROP_TAGS:
			g_value_set_object(value, purple_contact_info_get_tags(info));
			break;
		case PROP_PERSON:
			g_value_set_object(value, purple_contact_info_get_person(info));
			break;
		case PROP_PERMISSION:
			g_value_set_enum(value, purple_contact_info_get_permission(info));
			break;
		case PROP_NAME_FOR_DISPLAY:
			g_value_set_string(value,
			                   purple_contact_info_get_name_for_display(info));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_contact_info_set_property(GObject *obj, guint param_id,
                                 const GValue *value, GParamSpec *pspec)
{
	PurpleContactInfo *info = PURPLE_CONTACT_INFO(obj);

	switch(param_id) {
		case PROP_ID:
			purple_contact_info_set_id(info, g_value_get_string(value));
			break;
		case PROP_USERNAME:
			purple_contact_info_set_username(info, g_value_get_string(value));
			break;
		case PROP_DISPLAY_NAME:
			purple_contact_info_set_display_name(info,
			                                     g_value_get_string(value));
			break;
		case PROP_ALIAS:
			purple_contact_info_set_alias(info, g_value_get_string(value));
			break;
		case PROP_COLOR:
			purple_contact_info_set_color(info, g_value_get_string(value));
			break;
		case PROP_AVATAR:
			purple_contact_info_set_avatar(info, g_value_get_object(value));
			break;
		case PROP_PERSON:
			purple_contact_info_set_person(info, g_value_get_object(value));
			break;
		case PROP_PERMISSION:
			purple_contact_info_set_permission(info, g_value_get_enum(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_contact_info_dispose(GObject *obj) {
	PurpleContactInfo *info = PURPLE_CONTACT_INFO(obj);
	PurpleContactInfoPrivate *priv = NULL;

	priv = purple_contact_info_get_instance_private(info);

	g_clear_object(&priv->avatar);
	g_clear_object(&priv->presence);
	g_clear_object(&priv->tags);
	g_clear_object(&priv->person);

	G_OBJECT_CLASS(purple_contact_info_parent_class)->dispose(obj);
}

static void
purple_contact_info_finalize(GObject *obj) {
	PurpleContactInfo *info = PURPLE_CONTACT_INFO(obj);
	PurpleContactInfoPrivate *priv = NULL;

	priv = purple_contact_info_get_instance_private(info);

	g_clear_pointer(&priv->id, g_free);
	g_clear_pointer(&priv->username, g_free);
	g_clear_pointer(&priv->display_name, g_free);
	g_clear_pointer(&priv->alias, g_free);
	g_clear_pointer(&priv->color, g_free);
	g_clear_pointer(&priv->name_for_display, g_free);

	G_OBJECT_CLASS(purple_contact_info_parent_class)->finalize(obj);
}

static void
purple_contact_info_constructed(GObject *obj) {
	PurpleContactInfo *info = NULL;
	PurpleContactInfoPrivate *priv = NULL;

	G_OBJECT_CLASS(purple_contact_info_parent_class)->constructed(obj);

	info = PURPLE_CONTACT_INFO(obj);
	priv = purple_contact_info_get_instance_private(info);

	if(priv->id == NULL) {
		purple_contact_info_set_id(info, NULL);
	}

	purple_contact_info_update_name_for_display(info);
}

static void
purple_contact_info_init(PurpleContactInfo *info) {
	PurpleContactInfoPrivate *priv = NULL;

	priv = purple_contact_info_get_instance_private(info);

	priv->tags = purple_tags_new();

	priv->presence = g_object_new(PURPLE_TYPE_PRESENCE, NULL);
	g_signal_connect_object(priv->presence, "notify",
	                        G_CALLBACK(purple_contact_info_presence_notify_cb),
	                        info, 0);
}

static void
purple_contact_info_class_init(PurpleContactInfoClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->constructed = purple_contact_info_constructed;
	obj_class->dispose = purple_contact_info_dispose;
	obj_class->finalize = purple_contact_info_finalize;
	obj_class->get_property = purple_contact_info_get_property;
	obj_class->set_property = purple_contact_info_set_property;

	/**
	 * PurpleContactInfo:id:
	 *
	 * The protocol specific id for the contact.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_ID] = g_param_spec_string(
		"id", "id",
		"The id of the contact",
		NULL,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleContactInfo:username:
	 *
	 * The username for this contact. In rare cases this can change, like when
	 * a user changes their "nick" on IRC which is their user name.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_USERNAME] = g_param_spec_string(
		"username", "username",
		"The username of the contact",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleContactInfo:display-name:
	 *
	 * The display name for this contact. This is generally set by the person
	 * the contact is representing and controlled via the protocol plugin.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_DISPLAY_NAME] = g_param_spec_string(
		"display-name", "display-name",
		"The display name of the contact",
		NULL,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleContactInfo:alias:
	 *
	 * The alias for this contact. This is controlled by the libpurple user and
	 * may be used by the protocol if it allows for aliasing.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_ALIAS] = g_param_spec_string(
		"alias", "alias",
		"The alias of the contact.",
		NULL,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleContactInfo:color:
	 *
	 * The color for this contact. This is an RGB hex code that user interfaces
	 * can use when rendering the contact. This may also be controlled via a
	 * protocol plugin in the event that the protocol allows people to set a
	 * highlight/branding color.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_COLOR] = g_param_spec_string(
		"color", "color",
		"The color to use when rendering the contact.",
		NULL,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleContactInfo:avatar:
	 *
	 * The avatar for this contact. This is typically controlled by the protocol
	 * and should only be read by others.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_AVATAR] = g_param_spec_object(
		"avatar", "avatar",
		"The avatar of the contact",
		GDK_TYPE_PIXBUF,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleContactInfo:presence:
	 *
	 * The [class@Purple.Presence] for this contact. This is typically
	 * controlled by the protocol and should only be read by others.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_PRESENCE] = g_param_spec_object(
		"presence", "presence",
		"The presence of the contact",
		PURPLE_TYPE_PRESENCE,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleContactInfo:tags:
	 *
	 * The [class@Purple.Tags] for this contact.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_TAGS] = g_param_spec_object(
		"tags", "tags",
		"The tags for the contact",
		PURPLE_TYPE_TAGS,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleContactInfo:person:
	 *
	 * The [class@Purple.Person] that this contact belongs to.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_PERSON] = g_param_spec_object(
		"person", "person",
		"The person this contact belongs to.",
		PURPLE_TYPE_PERSON,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleContactInfo:permission:
	 *
	 * The permission level for the contact.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_PERMISSION] = g_param_spec_enum(
		"permission", "permission",
		"The permission level of the contact",
		PURPLE_TYPE_CONTACT_INFO_PERMISSION,
		PURPLE_CONTACT_INFO_PERMISSION_UNSET,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleContactInfo:name-for-display:
	 *
	 * The name that the user interface should display for this contact info.
	 *
	 * This will first check [property@Purple.ContactInfo:alias] and return
	 * that if it is set.
	 *
	 * Next, if the [property@Purple.ContactInfo:person] points to a valid
	 * [class@Purple.Person], the alias of [class@Purple.Person] will be
	 * returned if it is set.
	 *
	 * Otherwise, this will be set to the first set property from the following
	 * list:
	 *
	 *  * [property@Purple.ContactInfo:display-name]
	 *  * [property@Purple.ContactInfo:username]
	 *  * [property@Purple.ContactInfo:id]
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_NAME_FOR_DISPLAY] = g_param_spec_string(
		"name-for-display", "name-for-display",
		"The name that should be displayed for the contact info",
		NULL,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);

	/**
	 * PurpleContactInfo::presence-changed:
	 * @info: The instance.
	 * @presence: The presence that was changed.
	 * @pspec: The [class@GObject.ParamSpec] for the property that changed.
	 *
	 * This is a propagation of the notify signal from @presence. This means
	 * that your callback will be called when anything in the presence changes.
	 *
	 * This also supports details, so you can specify the signal name as
	 * something like `presence-changed::message` and your callback will only
	 * be called when the message property of @presence has been changed.
	 *
	 * Since: 3.0.0
	 */
	signals[SIG_PRESENCE_CHANGED] = g_signal_new_class_handler(
		"presence-changed",
		G_OBJECT_CLASS_TYPE(klass),
		G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
		NULL,
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		2,
		PURPLE_TYPE_PRESENCE,
		G_TYPE_PARAM);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
PurpleContactInfo *
purple_contact_info_new(const gchar *id) {
	return g_object_new(
		PURPLE_TYPE_CONTACT_INFO,
		"id", id,
		NULL);
}

const gchar *
purple_contact_info_get_id(PurpleContactInfo *info) {
	PurpleContactInfoPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_CONTACT_INFO(info), NULL);

	priv = purple_contact_info_get_instance_private(info);

	return priv->id;
}

void
purple_contact_info_set_id(PurpleContactInfo *info, const gchar *id) {
	PurpleContactInfoPrivate *priv = NULL;
	gboolean changed = FALSE;

	g_return_if_fail(PURPLE_IS_CONTACT_INFO(info));

	priv = purple_contact_info_get_instance_private(info);

	changed = !purple_strequal(priv->id, id);

	g_free(priv->id);
	priv->id = g_strdup(id);

	if(changed) {
		g_object_freeze_notify(G_OBJECT(info));

		g_object_notify_by_pspec(G_OBJECT(info), properties[PROP_ID]);

		purple_contact_info_update_name_for_display(info);

		g_object_thaw_notify(G_OBJECT(info));
	}
}

const gchar *
purple_contact_info_get_username(PurpleContactInfo *info) {
	PurpleContactInfoPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_CONTACT_INFO(info), NULL);

	priv = purple_contact_info_get_instance_private(info);

	return priv->username;
}

void
purple_contact_info_set_username(PurpleContactInfo *info,
                                 const gchar *username)
{
	PurpleContactInfoPrivate *priv = NULL;
	gboolean changed = FALSE;

	g_return_if_fail(PURPLE_IS_CONTACT_INFO(info));

	priv = purple_contact_info_get_instance_private(info);

	changed = !purple_strequal(priv->username, username);

	g_free(priv->username);
	priv->username = g_strdup(username);

	if(changed) {
		g_object_freeze_notify(G_OBJECT(info));

		g_object_notify_by_pspec(G_OBJECT(info), properties[PROP_USERNAME]);

		purple_contact_info_update_name_for_display(info);

		g_object_thaw_notify(G_OBJECT(info));
	}
}

const gchar *
purple_contact_info_get_display_name(PurpleContactInfo *info) {
	PurpleContactInfoPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_CONTACT_INFO(info), NULL);

	priv = purple_contact_info_get_instance_private(info);

	return priv->display_name;
}

void
purple_contact_info_set_display_name(PurpleContactInfo *info,
                                     const gchar *display_name)
{
	PurpleContactInfoPrivate *priv = NULL;
	gboolean changed = FALSE;

	g_return_if_fail(PURPLE_IS_CONTACT_INFO(info));

	priv = purple_contact_info_get_instance_private(info);

	changed = !purple_strequal(priv->display_name, display_name);

	g_free(priv->display_name);
	priv->display_name = g_strdup(display_name);

	if(changed) {
		g_object_freeze_notify(G_OBJECT(info));

		g_object_notify_by_pspec(G_OBJECT(info), properties[PROP_DISPLAY_NAME]);

		purple_contact_info_update_name_for_display(info);

		g_object_thaw_notify(G_OBJECT(info));
	}
}

const gchar *
purple_contact_info_get_alias(PurpleContactInfo *info) {
	PurpleContactInfoPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_CONTACT_INFO(info), NULL);

	priv = purple_contact_info_get_instance_private(info);

	return priv->alias;
}

void
purple_contact_info_set_alias(PurpleContactInfo *info, const gchar *alias) {
	PurpleContactInfoPrivate *priv = NULL;
	gboolean changed = FALSE;

	g_return_if_fail(PURPLE_IS_CONTACT_INFO(info));

	priv = purple_contact_info_get_instance_private(info);

	changed = !purple_strequal(priv->alias, alias);

	g_free(priv->alias);
	priv->alias = g_strdup(alias);

	if(changed) {
		g_object_freeze_notify(G_OBJECT(info));

		g_object_notify_by_pspec(G_OBJECT(info), properties[PROP_ALIAS]);

		purple_contact_info_update_name_for_display(info);

		g_object_thaw_notify(G_OBJECT(info));
	}
}

const char *
purple_contact_info_get_color(PurpleContactInfo *info) {
	PurpleContactInfoPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_CONTACT_INFO(info), NULL);

	priv = purple_contact_info_get_instance_private(info);

	return priv->color;
}

void
purple_contact_info_set_color(PurpleContactInfo *info, const char *color) {
	PurpleContactInfoPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_CONTACT_INFO(info));

	priv = purple_contact_info_get_instance_private(info);

	if(!purple_strequal(priv->color, color)) {
		g_free(priv->color);
		priv->color = g_strdup(color);

		g_object_notify_by_pspec(G_OBJECT(info), properties[PROP_COLOR]);
	}
}

GdkPixbuf *
purple_contact_info_get_avatar(PurpleContactInfo *info) {
	PurpleContactInfoPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_CONTACT_INFO(info), NULL);

	priv = purple_contact_info_get_instance_private(info);

	return priv->avatar;
}

void
purple_contact_info_set_avatar(PurpleContactInfo *info, GdkPixbuf *avatar) {
	PurpleContactInfoPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_CONTACT_INFO(info));

	priv = purple_contact_info_get_instance_private(info);

	if(g_set_object(&priv->avatar, avatar)) {
		g_object_notify_by_pspec(G_OBJECT(info), properties[PROP_AVATAR]);
	}
}

PurplePresence *
purple_contact_info_get_presence(PurpleContactInfo *info) {
	PurpleContactInfoPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_CONTACT_INFO(info), NULL);

	priv = purple_contact_info_get_instance_private(info);

	return priv->presence;
}

PurpleTags *
purple_contact_info_get_tags(PurpleContactInfo *info) {
	PurpleContactInfoPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_CONTACT_INFO(info), NULL);

	priv = purple_contact_info_get_instance_private(info);

	return priv->tags;
}

void
purple_contact_info_set_person(PurpleContactInfo *info, PurplePerson *person) {
	PurpleContactInfoPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_CONTACT_INFO(info));

	priv = purple_contact_info_get_instance_private(info);

	if(g_set_object(&priv->person, person)) {
		/* If we got a new person, we need to connect to the notify::alias
		 * signal.
		 */
		if(PURPLE_IS_PERSON(priv->person)) {
			g_signal_connect_object(priv->person, "notify::alias",
			                        G_CALLBACK(purple_contact_info_person_alias_changed_cb),
			                        info, 0);
		}

		/* Freeze notifications as the person update could change the
		 * name-for-display property.
		 */
		g_object_freeze_notify(G_OBJECT(info));

		g_object_notify_by_pspec(G_OBJECT(info), properties[PROP_PERSON]);

		/* Update the name-for-display property */
		purple_contact_info_update_name_for_display(info);

		g_object_thaw_notify(G_OBJECT(info));
	}
}

PurplePerson *
purple_contact_info_get_person(PurpleContactInfo *info) {
	PurpleContactInfoPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_CONTACT_INFO(info), NULL);

	priv = purple_contact_info_get_instance_private(info);

	return priv->person;
}

PurpleContactInfoPermission
purple_contact_info_get_permission(PurpleContactInfo *info) {
	PurpleContactInfoPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_CONTACT_INFO(info),
	                     PURPLE_CONTACT_INFO_PERMISSION_UNSET);

	priv = purple_contact_info_get_instance_private(info);

	return priv->permission;
}

void
purple_contact_info_set_permission(PurpleContactInfo *info,
                                   PurpleContactInfoPermission permission)
{
	PurpleContactInfoPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_CONTACT_INFO(info));

	priv = purple_contact_info_get_instance_private(info);

	priv->permission = permission;

	g_object_notify_by_pspec(G_OBJECT(info), properties[PROP_PERMISSION]);
}

const char *
purple_contact_info_get_name_for_display(PurpleContactInfo *info) {
	PurpleContactInfoPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_CONTACT_INFO(info), NULL);

	priv = purple_contact_info_get_instance_private(info);

	return priv->name_for_display;
}

int
purple_contact_info_compare(PurpleContactInfo *a, PurpleContactInfo *b) {
	PurplePerson *person_a = NULL;
	PurplePerson *person_b = NULL;
	const char *name_a = NULL;
	const char *name_b = NULL;

	/* Check for NULL values. */
	if(a != NULL && b == NULL) {
		return -1;
	} else if(a == NULL && b != NULL) {
		return 1;
	} else if(a == NULL && b == NULL) {
		return 0;
	}

	/* Check if the contacts have persons associated with them. */
	person_a = purple_contact_info_get_person(a);
	person_b = purple_contact_info_get_person(b);

	if(person_a != NULL && person_b == NULL) {
		return -1;
	} else if(person_a == NULL && person_b != NULL) {
		return 1;
	}

	/* Finally get the names for the displaying and compare those. */
	name_a = purple_contact_info_get_name_for_display(a);
	name_b = purple_contact_info_get_name_for_display(b);

	return purple_utf8_strcasecmp(name_a, name_b);
}

gboolean
purple_contact_info_matches(PurpleContactInfo *info, const char *needle) {
	PurpleContactInfoPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_CONTACT_INFO(info), FALSE);

	if(purple_strempty(needle)) {
		return TRUE;
	}

	priv = purple_contact_info_get_instance_private(info);

	if(!purple_strempty(priv->username)) {
		if(purple_strmatches(needle, priv->username)) {
			return TRUE;
		}
	}

	if(!purple_strempty(priv->alias)) {
		if(purple_strmatches(needle, priv->alias)) {
			return TRUE;
		}
	}

	if(!purple_strempty(priv->display_name)) {
		if(purple_strmatches(needle, priv->display_name)) {
			return TRUE;
		}
	}

	/* Nothing matched, so return FALSE. */
	return FALSE;
}
