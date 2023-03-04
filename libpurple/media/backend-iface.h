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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 */

#if !defined(PURPLE_GLOBAL_HEADER_INSIDE) && !defined(PURPLE_COMPILATION)
# error "only <purple.h> may be included directly"
#endif

#ifndef PURPLE_MEDIA_BACKEND_IFACE_H
#define PURPLE_MEDIA_BACKEND_IFACE_H

#include "codec.h"
#include "enum-types.h"
#include "media.h"

#include <glib-object.h>

G_BEGIN_DECLS

#define PURPLE_MEDIA_TYPE_BACKEND		(purple_media_backend_get_type())
#define PURPLE_MEDIA_IS_BACKEND(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), PURPLE_MEDIA_TYPE_BACKEND))
#define PURPLE_MEDIA_BACKEND(obj)		(G_TYPE_CHECK_INSTANCE_CAST((obj), PURPLE_MEDIA_TYPE_BACKEND, PurpleMediaBackend))
#define PURPLE_MEDIA_BACKEND_GET_INTERFACE(inst)(G_TYPE_INSTANCE_GET_INTERFACE((inst), PURPLE_MEDIA_TYPE_BACKEND, PurpleMediaBackendInterface))

/**
 * PurpleMediaBackend:
 *
 * A placeholder to represent any media backend
 */
typedef struct _PurpleMediaBackend PurpleMediaBackend;

/**
 * PurpleMediaBackendInterface:
 *
 * A structure to derive media backends from.
 */
typedef struct _PurpleMediaBackendInterface PurpleMediaBackendInterface;

struct _PurpleMediaBackendInterface
{
	/*< private >*/
	GTypeInterface parent_iface; /* The parent iface class */

	/*< public >*/
	/* Implementable functions called with purple_media_backend_* */
	gboolean (*add_stream) (PurpleMediaBackend *self,
		const gchar *sess_id, const gchar *who,
		PurpleMediaSessionType type, gboolean initiator,
		const gchar *transmitter, GHashTable *params);

	void (*add_remote_candidates) (PurpleMediaBackend *self,
		const gchar *sess_id, const gchar *participant,
		GList *remote_candidates);
	gboolean (*codecs_ready) (PurpleMediaBackend *self,
		const gchar *sess_id);
	GList *(*get_codecs) (PurpleMediaBackend *self,
		const gchar *sess_id);
	GList *(*get_local_candidates) (PurpleMediaBackend *self,
		const gchar *sess_id, const gchar *participant);
	gboolean (*set_remote_codecs) (PurpleMediaBackend *self,
		const gchar *sess_id, const gchar *participant,
		GList *codecs);
	gboolean (*set_send_codec) (PurpleMediaBackend *self,
		const gchar *sess_id, PurpleMediaCodec *codec);
	gboolean (*set_encryption_parameters) (PurpleMediaBackend *self,
		const gchar *sess_id, const gchar *cipher,
		const gchar *auth, const gchar *key, gsize key_len);
	gboolean (*set_decryption_parameters) (PurpleMediaBackend *self,
		const gchar *sess_id, const gchar *participant,
		const gchar *cipher, const gchar *auth,
		const gchar *key, gsize key_len);
	gboolean (*set_require_encryption) (PurpleMediaBackend *self,
		const gchar *sess_id, const gchar *participant,
		gboolean require_encryption);
	void (*set_params) (PurpleMediaBackend *self, GHashTable *params);
	const gchar **(*get_available_params) (void);
	gboolean (*send_dtmf) (PurpleMediaBackend *self,
		const gchar *sess_id, gchar dtmf, guint8 volume,
		guint16 duration);
	gboolean (*set_send_rtcp_mux) (PurpleMediaBackend *self,
		const gchar *sess_id, const gchar *participant, gboolean send_rtcp_mux);
};

/**
 * purple_media_backend_get_type:
 *
 * Gets the media backend's GType.
 *
 * Returns: The media backend's GType.
 */
GType purple_media_backend_get_type(void);

/**
 * purple_media_backend_add_stream:
 * @self: The backend to add the stream to.
 * @sess_id: The session id of the stream to add.
 * @who: The remote participant of the stream to add.
 * @type: The media type and direction of the stream to add.
 * @initiator: True if the local user initiated the stream.
 * @transmitter: The string id of the tranmsitter to use.
 * @params: (element-type utf8 GValue) (transfer none): The additional
 *          parameters to pass when creating the stream.
 *
 * Creates and adds a stream to the media backend.
 *
 * Returns: True if the stream was successfully created, otherwise False.
 */
gboolean purple_media_backend_add_stream(PurpleMediaBackend *self,
		const gchar *sess_id, const gchar *who, PurpleMediaSessionType type,
		gboolean initiator, const gchar *transmitter, GHashTable *params);

/**
 * purple_media_backend_add_remote_candidates:
 * @self: The backend the stream is in.
 * @sess_id: The session id associated with the stream.
 * @participant: The participant associated with the stream.
 * @remote_candidates: (element-type PurpleMediaCandidate): The list of remote candidates to add.
 *
 * Add remote candidates to a stream.
 */
void purple_media_backend_add_remote_candidates(PurpleMediaBackend *self,
		const gchar *sess_id, const gchar *participant,
		GList *remote_candidates);

/**
 * purple_media_backend_codecs_ready:
 * @self: The media backend the session is in.
 * @sess_id: The session id of the session to check.
 *
 * Get whether or not a session's codecs are ready.
 *
 * A codec is ready if all of the attributes and additional
 * parameters have been collected.
 *
 * Returns: True if the codecs are ready, otherwise False.
 */
gboolean purple_media_backend_codecs_ready(PurpleMediaBackend *self,
		const gchar *sess_id);

/**
 * purple_media_backend_get_codecs:
 * @self: The media backend the session is in.
 * @sess_id: The session id of the session to use.
 *
 * Gets the codec intersection list for a session.
 *
 * The intersection list consists of all codecs that are compatible
 * between the local and remote software.
 *
 * Returns: (transfer full) (element-type PurpleMediaCodec): The codec intersection list.
 */
GList *purple_media_backend_get_codecs(PurpleMediaBackend *self,
		const gchar *sess_id);

/**
 * purple_media_backend_get_local_candidates:
 * @self: The media backend the stream is in.
 * @sess_id: The session id associated with the stream.
 * @participant: The participant associated with the stream.
 *
 * Gets the list of local candidates for a stream.
 *
 * Return Value: (transfer full) (element-type PurpleMediaCandidate): The list of local candidates.
 */
GList *purple_media_backend_get_local_candidates(PurpleMediaBackend *self,
		const gchar *sess_id, const gchar *participant);

/**
 * purple_media_backend_set_remote_codecs:
 * @self: The media backend the stream is in.
 * @sess_id: The session id the stream is associated with.
 * @participant: The participant the stream is associated with.
 * @codecs: (element-type PurpleMediaCodec): The list of remote codecs to set.
 *
 * Sets the remote codecs on a stream.
 *
 * Returns: True if the remote codecs were set successfully, otherwise False.
 */
gboolean purple_media_backend_set_remote_codecs(PurpleMediaBackend *self,
		const gchar *sess_id, const gchar *participant,
		GList *codecs);

/**
 * purple_media_backend_set_send_codec:
 * @self: The media backend the session is in.
 * @sess_id: The session id of the session to set the codec for.
 * @codec: The codec to set.
 *
 * Sets which codec format to send media content in for a session.
 *
 * Returns: True if set successfully, otherwise False.
 */
gboolean purple_media_backend_set_send_codec(PurpleMediaBackend *self,
		const gchar *sess_id, PurpleMediaCodec *codec);

/**
 * purple_media_backend_set_encryption_parameters:
 * @self: The media backend the session is in.
 * @sess_id: The session id of the session to set parameters of.
 * @cipher: The cipher to use to encrypt our media in the session.
 * @auth: The algorithm to use to compute authentication codes for our media
 *        frames.
 * @key: The encryption key.
 * @key_len: Byte length of the encryption key.
 *
 * Sets the encryption parameters of our media in the session.
 */
gboolean purple_media_backend_set_encryption_parameters(PurpleMediaBackend *self,
		const gchar *sess_id, const gchar *cipher,
		const gchar *auth, const gchar *key, gsize key_len);

/**
 * purple_media_backend_set_decryption_parameters:
 * @self: The media backend the session is in.
 * @sess_id: The session id of the session to set parameters of.
 * @participant: The participant of the session to set parameters of.
 * @cipher: The cipher to use to decrypt media coming from this session's
 *          participant.
 * @auth: The algorithm to use for authentication of the media coming from
 *        the session's participant.
 * @key: The decryption key.
 * @key_len: Byte length of the decryption key.
 *
 * Sets the decryption parameters for a session participant's media.
 */
gboolean purple_media_backend_set_decryption_parameters(PurpleMediaBackend *self,
		const gchar *sess_id, const gchar *participant,
		const gchar *cipher, const gchar *auth,
		const gchar *key, gsize key_len);

/**
 * purple_media_backend_set_require_encryption:
 * @self: The media object to find the session in.
 * @sess_id: The id of the session to set parameters of.
 * @participant: The participant of the session to set parameters of.
 * @require_encryption: TRUE if the media requires encryption.
 *
 * Sets whether a session participant's media requires encryption.
 */
gboolean purple_media_backend_set_require_encryption(PurpleMediaBackend *self,
		const gchar *sess_id, const gchar *participant,
		gboolean require_encryption);

/**
 * purple_media_backend_set_params:
 * @self: The media backend to set the parameters on.
 * @params: (element-type utf8 GObject.Value) (transfer none): Hash table of
 *          parameters to pass to the backend.
 *
 * Sets various optional parameters of the media backend.
 */
void purple_media_backend_set_params(PurpleMediaBackend *self,
		GHashTable *params);

/**
 * purple_media_backend_get_available_params:
 * @self: The media backend
 *
 * Gets the list of optional parameters supported by the media backend.
 * The list should NOT be freed.
 *
 * Return Value: (transfer none): NULL-terminated array of names of supported parameters.
 */
const gchar **purple_media_backend_get_available_params(PurpleMediaBackend *self);

/**
 * purple_media_backend_set_send_rtcp_mux:
 * @self: The media backend the session is in.
 * @sess_id: The session id of the session to set the rtcp-mux option to
 * @participant: The participant the stream is associated with.
 * @send_rtcp_mux: Whether or not to enable rtcp-mux
 *
 * Controls whether or not the RTCP should be muxed with the RTP
 *
 * Returns: True if set successfully, otherwise False.
 *
 * Since: 2.11.0
 */
gboolean purple_media_backend_set_send_rtcp_mux(PurpleMediaBackend *self,
		const gchar *sess_id, const gchar *participant, gboolean send_rtcp_mux);


G_END_DECLS

#endif /* PURPLE_MEDIA_BACKEND_IFACE_H */
