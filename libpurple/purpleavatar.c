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

#include "purpleavatar.h"

struct _PurpleAvatar {
	GObject parent;

	char *filename;

	GdkPixbuf *pixbuf;

	gboolean animated;
	GdkPixbufAnimation *animation;

	PurpleTags *tags;
};

enum {
	PROP_0,
	PROP_FILENAME,
	PROP_PIXBUF,
	PROP_ANIMATED,
	PROP_ANIMATION,
	PROP_TAGS,
	N_PROPERTIES,
};
static GParamSpec *properties[N_PROPERTIES] = {NULL, };

G_DEFINE_TYPE(PurpleAvatar, purple_avatar, G_TYPE_OBJECT)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
purple_avatar_set_filename(PurpleAvatar *avatar, const char *filename) {
	g_return_if_fail(PURPLE_IS_AVATAR(avatar));

	g_free(avatar->filename);
	avatar->filename = g_strdup(filename);

	g_object_notify_by_pspec(G_OBJECT(avatar), properties[PROP_FILENAME]);
}

static PurpleAvatar *
purple_avatar_new_common(const char *filename, GdkPixbufAnimation *animation) {
	PurpleAvatar *avatar = NULL;

	avatar = g_object_new(PURPLE_TYPE_AVATAR, "filename", filename, NULL);

	if(gdk_pixbuf_animation_is_static_image(animation)) {
		/* If we loaded a static image, grab the static image and set it to our
		 * pixbuf member, clear the animation, and return the new avatar.
		 */

		avatar->pixbuf = gdk_pixbuf_animation_get_static_image(animation);
		g_object_ref(avatar->pixbuf);

		g_clear_object(&animation);
	} else {
		/* If we did load an animation, set the appropriate properties and
		 * return the avatar.
		 */
		avatar->animated = TRUE;
		avatar->animation = animation;
	}

	return avatar;
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
purple_avatar_get_property(GObject *obj, guint param_id, GValue *value,
                           GParamSpec *pspec)
{
	PurpleAvatar *avatar = PURPLE_AVATAR(obj);

	switch(param_id) {
		case PROP_FILENAME:
			g_value_set_string(value, purple_avatar_get_filename(avatar));
			break;
		case PROP_PIXBUF:
			g_value_set_object(value, purple_avatar_get_pixbuf(avatar));
			break;
		case PROP_ANIMATED:
			g_value_set_boolean(value, purple_avatar_get_animated(avatar));
			break;
		case PROP_ANIMATION:
			g_value_set_object(value, purple_avatar_get_animation(avatar));
			break;
		case PROP_TAGS:
			g_value_set_object(value, purple_avatar_get_tags(avatar));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_avatar_set_property(GObject *obj, guint param_id, const GValue *value,
                           GParamSpec *pspec)
{
	PurpleAvatar *avatar = PURPLE_AVATAR(obj);

	switch(param_id) {
		case PROP_FILENAME:
			purple_avatar_set_filename(avatar, g_value_get_string(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_avatar_finalize(GObject *obj) {
	PurpleAvatar *avatar = PURPLE_AVATAR(obj);

	g_clear_pointer(&avatar->filename, g_free);
	g_clear_object(&avatar->pixbuf);
	g_clear_object(&avatar->animation);
	g_clear_object(&avatar->tags);

	G_OBJECT_CLASS(purple_avatar_parent_class)->finalize(obj);
}

static void
purple_avatar_init(PurpleAvatar *avatar) {
	avatar->tags = purple_tags_new();
}

static void
purple_avatar_class_init(PurpleAvatarClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->finalize = purple_avatar_finalize;
	obj_class->get_property = purple_avatar_get_property;
	obj_class->set_property = purple_avatar_set_property;

	/**
	 * PurpleAvatar:filename:
	 *
	 * The filename that this avatar was created from.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_FILENAME] = g_param_spec_string(
		"filename", "filename",
		"The filename to save/load the avatar from.",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleAvatar:pixbuf:
	 *
	 * The [class@GdkPixbuf.Pixbuf] of the avatar. If
	 * [property@Purple.Avatar:animated] is %TRUE, this will be a static frame
	 * from the animation.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_PIXBUF] = g_param_spec_object(
		"pixbuf", "pixbuf",
		"The pixbuf of the avatar.",
		GDK_TYPE_PIXBUF,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleAvatar:animated:
	 *
	 * Whether or not this avatar is animated.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_ANIMATED] = g_param_spec_boolean(
		"animated", "animated",
		"Whether or not the avatar is animated.",
		FALSE,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleAvatar:animation:
	 *
	 * The [class@GdkPixbuf.PixbufAnimation] if
	 * [property@Purple.Avatar:animated] is %TRUE.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_ANIMATION] = g_param_spec_object(
		"animation", "animation",
		"The animation of the avatar.",
		GDK_TYPE_PIXBUF_ANIMATION,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleAvatar:tags:
	 *
	 * The [class@Purple.Tags] for the avatar.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_TAGS] = g_param_spec_object(
		"tags", "tags",
		"The tags for the avatar.",
		PURPLE_TYPE_TAGS,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
PurpleAvatar *
purple_avatar_new_from_filename(const char *filename, GError **error) {
	GdkPixbufAnimation *animation = NULL;
	GError *local_error = NULL;

	g_return_val_if_fail(filename != NULL, NULL);

	animation = gdk_pixbuf_animation_new_from_file(filename, &local_error);
	if(!GDK_IS_PIXBUF_ANIMATION(animation) || local_error != NULL) {
		g_clear_object(&animation);

		g_propagate_error(error, local_error);

		return NULL;
	}

	return purple_avatar_new_common(filename, animation);
}

PurpleAvatar *
purple_avatar_new_from_resource(const char *resource_path, GError **error) {
	GdkPixbufAnimation *animation = NULL;
	GError *local_error = NULL;

	g_return_val_if_fail(resource_path != NULL, NULL);

	animation = gdk_pixbuf_animation_new_from_resource(resource_path,
	                                                   &local_error);
	if(!GDK_IS_PIXBUF_ANIMATION(animation) || local_error != NULL) {
		g_clear_object(&animation);

		g_propagate_error(error, local_error);

		return NULL;
	}

	return purple_avatar_new_common(NULL, animation);
}

const char *
purple_avatar_get_filename(PurpleAvatar *avatar) {
	g_return_val_if_fail(PURPLE_IS_AVATAR(avatar), NULL);

	return avatar->filename;
}

GdkPixbuf *
purple_avatar_get_pixbuf(PurpleAvatar *avatar) {
	g_return_val_if_fail(PURPLE_IS_AVATAR(avatar), NULL);

	if(avatar->animated) {
		return gdk_pixbuf_animation_get_static_image(avatar->animation);
	}

	return avatar->pixbuf;
}

gboolean
purple_avatar_get_animated(PurpleAvatar *avatar) {
	g_return_val_if_fail(PURPLE_IS_AVATAR(avatar), FALSE);

	return avatar->animated;
}

GdkPixbufAnimation *
purple_avatar_get_animation(PurpleAvatar *avatar) {
	g_return_val_if_fail(PURPLE_IS_AVATAR(avatar), NULL);

	return avatar->animation;
}

PurpleTags *
purple_avatar_get_tags(PurpleAvatar *avatar) {
	g_return_val_if_fail(PURPLE_IS_AVATAR(avatar), NULL);

	return avatar->tags;
}
