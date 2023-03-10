/* purple
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02111-1301 USA
 */

#if !defined(PURPLE_GLOBAL_HEADER_INSIDE) && !defined(PURPLE_COMPILATION)
# error "only <purple.h> may be included directly"
#endif

#ifndef PURPLE_IMAGE_H
#define PURPLE_IMAGE_H

#include <glib-object.h>

#define PURPLE_TYPE_IMAGE  purple_image_get_type()

struct _PurpleImageClass {
	/*< private >*/
	GObjectClass parent_class;

	void (*purple_reserved1)(void);
	void (*purple_reserved2)(void);
	void (*purple_reserved3)(void);
	void (*purple_reserved4)(void);
};

G_BEGIN_DECLS

/**
 * PurpleImage:
 *
 * #PurpleImage object is a container for raw image data. It doesn't manipulate
 * image data, just stores it in its binary format - png, jpeg etc. Thus, it's
 * totally independent from the UI.
 *
 * This class also provides certain file-related features, like: friendly
 * filenames (not necessarily real filename for displaying); remote images
 * (which data is not yet loaded) or guessing file format from its header.
 */

G_DECLARE_DERIVABLE_TYPE(PurpleImage, purple_image, PURPLE, IMAGE, GObject)

/**
 * purple_image_new_from_bytes:
 * @bytes: (transfer none): A #GBytes containing the raw image data.
 *
 * Loads a raw image data as a new #PurpleImage object.
 *
 * Returns: the new #PurpleImage.
 */
PurpleImage *purple_image_new_from_bytes(GBytes *bytes);

/**
 * purple_image_new_from_file:
 * @path: the path to the image file.
 * @error: Return address for a #GError, or %NULL.
 *
 * Loads an image file as a new #PurpleImage object. The @path must exists, be
 * readable and should point to a valid image file. If you don't set @be_eager
 * parameter, there will be a risk that file will be removed from disk before
 * you access its data.
 *
 * Returns: the new #PurpleImage.
 */
PurpleImage *purple_image_new_from_file(const gchar *path, GError **error);

/**
 * purple_image_new_from_data:
 * @data: the pointer to the image data buffer.
 * @length: the length of @data.
 *
 * Creates a new #PurpleImage object with contents of @data buffer.
 *
 * The @data buffer is owned by #PurpleImage object, so you might want
 * to g_memdup2() it first.
 *
 * Returns: the new #PurpleImage.
 */
PurpleImage *purple_image_new_from_data(const guint8 *data, gsize length);

/**
 * purple_image_new_take_data:
 * @data: (transfer full): the pointer to the image data buffer.
 * @length: the length of @data.
 *
 * Creates a new #PurpleImage object with contents of @data buffer.
 *
 * The @data buffer is owned by #PurpleImage object, so you might want
 * to g_memdup2() it first.
 *
 * Returns: the new #PurpleImage.
 */
PurpleImage *purple_image_new_take_data(guint8 *data, gsize length);

/**
 * purple_image_save:
 * @image: the image.
 * @path: destination of a saved image file.
 *
 * Saves an @image to the disk.
 *
 * Returns: %TRUE if succeeded, %FALSE otherwise.
 */
gboolean purple_image_save(PurpleImage *image, const gchar *path);

/**
 * purple_image_get_contents:
 * @image: The #PurpleImage.
 *
 * Returns a new reference to the #GBytes that contains the image data.
 *
 * Returns: (transfer full): A #GBytes containing the image data.
 */
GBytes *purple_image_get_contents(PurpleImage *image);


/**
 * purple_image_get_path:
 * @image: the image.
 *
 * Returns the physical path of the @image file. It is set only, if the @image is
 * really backed by an existing file. In the other case it returns %NULL.
 *
 * Returns: the physical path of the @image, or %NULL.
 */
const gchar *purple_image_get_path(PurpleImage *image);

/**
 * purple_image_get_data_size:
 * @image: the image.
 *
 * Returns the size of @image's data.
 *
 * Returns: the size of data, or 0 in case of failure.
 */
gsize purple_image_get_data_size(PurpleImage *image);

/**
 * purple_image_get_data:
 * @image: the image.
 *
 * Returns the pointer to the buffer containing image data.
 *
 * Returns: (transfer none): the @image data.
 */
gconstpointer purple_image_get_data(PurpleImage *image);

/**
 * purple_image_get_extension:
 * @image: the image.
 *
 * Guesses the @image format based on its contents.
 *
 * Returns: (transfer none): the file extension suitable for @image format.
 */
const gchar *purple_image_get_extension(PurpleImage *image);

/**
 * purple_image_get_mimetype:
 * @image: the image.
 *
 * Guesses the @image mime-type based on its contents.
 *
 * Returns: (transfer none): the mime-type suitable for @image format.
 */
const gchar *purple_image_get_mimetype(PurpleImage *image);

/**
 * purple_image_generate_filename:
 * @image: the image.
 *
 * Calculates almost-unique filename by computing checksum from file contents
 * and appending a suitable extension. You should not assume the checksum
 * is SHA-1, because it may change in the future.
 *
 * Returns: (transfer none): the generated file name.
 */
const gchar *purple_image_generate_filename(PurpleImage *image);

/**
 * purple_image_set_friendly_filename:
 * @image: the image.
 * @filename: the friendly filename.
 *
 * Sets the "friendly filename" for the @image. This don't have to be a real
 * name, because it's used for displaying or as a default file name when the
 * user wants to save the @image to the disk.
 *
 * The provided @filename may either be a full path, or contain
 * filesystem-unfriendly characters, because it will be reformatted.
 */
void purple_image_set_friendly_filename(PurpleImage *image, const gchar *filename);

/**
 * purple_image_get_friendly_filename:
 * @image: the image.
 *
 * Returns the "friendly filename" for the @image, to be displayed or used as
 * a default name when saving a file to the disk.
 * See #purple_image_set_friendly_filename.
 *
 * If the friendly filename was not set, it will be generated with
 * #purple_image_generate_filename.
 *
 * Returns: (transfer none): the friendly filename.
 */
const gchar *purple_image_get_friendly_filename(PurpleImage *image);

G_END_DECLS

#endif /* PURPLE_IMAGE_H */
