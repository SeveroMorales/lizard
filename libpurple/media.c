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
#include "account.h"
#include "media.h"
#include "media/backend-iface.h"
#include "mediamanager.h"

#include "debug.h"

#include "media-gst.h"

typedef struct _PurpleMediaSession PurpleMediaSession;
typedef struct _PurpleMediaStream PurpleMediaStream;

struct _PurpleMediaSession
{
	gchar *id;
	PurpleMedia *media;
	PurpleMediaSessionType type;
	gboolean initiator;
};

struct _PurpleMediaStream
{
	PurpleMediaSession *session;
	gchar *participant;

	GList *local_candidates;
	GList *remote_candidates;

	gboolean initiator;
	gboolean accepted;
	gboolean candidates_prepared;

	GList *active_local_candidates;
	GList *active_remote_candidates;
};

struct _PurpleMediaPrivate
{
	PurpleMediaManager *manager;
	PurpleAccount *account;
	PurpleMediaBackend *backend;
	gchar *conference_type;
	gboolean initiator;
	gpointer protocol_data;

	GHashTable *sessions;	/* PurpleMediaSession table */
	GList *participants;
	GList *streams;		/* PurpleMediaStream table */
};

static void purple_media_class_init (PurpleMediaClass *klass);
static void purple_media_init (PurpleMedia *media);
static void purple_media_dispose (GObject *object);
static void purple_media_finalize (GObject *object);
static void purple_media_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
static void purple_media_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);

static void purple_media_new_local_candidate_cb(PurpleMediaBackend *backend,
		const gchar *sess_id, const gchar *participant,
		PurpleMediaCandidate *candidate, PurpleMedia *media);
static void purple_media_candidates_prepared_cb(PurpleMediaBackend *backend,
		const gchar *sess_id, const gchar *name, PurpleMedia *media);
static void purple_media_candidate_pair_established_cb(
		PurpleMediaBackend *backend,
		const gchar *sess_id, const gchar *name,
		PurpleMediaCandidate *local_candidate,
		PurpleMediaCandidate *remote_candidate,
		PurpleMedia *media);
static void purple_media_codecs_changed_cb(PurpleMediaBackend *backend,
		const gchar *sess_id, PurpleMedia *media);


enum {
	S_ERROR,
	CANDIDATES_PREPARED,
	CODECS_CHANGED,
	LEVEL,
	NEW_CANDIDATE,
	STATE_CHANGED,
	STREAM_INFO,
	CANDIDATE_PAIR_ESTABLISHED,
	LAST_SIGNAL
};
static guint purple_media_signals[LAST_SIGNAL] = {0};

enum {
	PROP_0,
	PROP_MANAGER,
	PROP_BACKEND,
	PROP_ACCOUNT,
	PROP_CONFERENCE_TYPE,
	PROP_INITIATOR,
	PROP_PROTOCOL_DATA,
};

G_DEFINE_TYPE_WITH_PRIVATE(PurpleMedia, purple_media, G_TYPE_OBJECT);

static void
purple_media_class_init (PurpleMediaClass *klass)
{
	GObjectClass *gobject_class = (GObjectClass*)klass;

	gobject_class->dispose = purple_media_dispose;
	gobject_class->finalize = purple_media_finalize;
	gobject_class->set_property = purple_media_set_property;
	gobject_class->get_property = purple_media_get_property;

	g_object_class_install_property(gobject_class, PROP_MANAGER,
			g_param_spec_object("manager",
			"Purple Media Manager",
			"The media manager that contains this media session.",
			PURPLE_TYPE_MEDIA_MANAGER,
			G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE |
			G_PARAM_STATIC_STRINGS));

	/*
	 * This one should be PURPLE_TYPE_MEDIA_BACKEND, but it doesn't
	 * like interfaces because they "aren't GObjects"
	 */
	g_object_class_install_property(gobject_class, PROP_BACKEND,
			g_param_spec_object("backend",
			"Purple Media Backend",
			"The backend object this media object uses.",
			G_TYPE_OBJECT,
			G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(gobject_class, PROP_ACCOUNT,
			g_param_spec_object("account", "PurpleAccount",
			"The account this media session is on.",
			PURPLE_TYPE_ACCOUNT,
			G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE |
			G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(gobject_class, PROP_CONFERENCE_TYPE,
			g_param_spec_string("conference-type",
			"Conference Type",
			"The type of conference that this media object "
			"has been created to provide.",
			NULL,
			G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE |
			G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(gobject_class, PROP_INITIATOR,
			g_param_spec_boolean("initiator",
			"initiator",
			"If the local user initiated the conference.",
			FALSE,
			G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE |
			G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(gobject_class, PROP_PROTOCOL_DATA,
			g_param_spec_pointer("protocol-data",
			"gpointer",
			"Data the protocol set on the media session.",
			G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	purple_media_signals[S_ERROR] = g_signal_new("error", G_TYPE_FROM_CLASS(klass),
					 G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL,
					 G_TYPE_NONE, 1, G_TYPE_STRING);
	purple_media_signals[CANDIDATES_PREPARED] = g_signal_new("candidates-prepared", G_TYPE_FROM_CLASS(klass),
					 G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL,
					 G_TYPE_NONE, 2, G_TYPE_STRING,
					 G_TYPE_STRING);
	purple_media_signals[CODECS_CHANGED] = g_signal_new("codecs-changed", G_TYPE_FROM_CLASS(klass),
					 G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL,
					 G_TYPE_NONE, 1, G_TYPE_STRING);
	purple_media_signals[LEVEL] = g_signal_new("level", G_TYPE_FROM_CLASS(klass),
					 G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL,
					 G_TYPE_NONE, 3, G_TYPE_STRING,
					 G_TYPE_STRING, G_TYPE_DOUBLE);
	purple_media_signals[NEW_CANDIDATE] = g_signal_new("new-candidate", G_TYPE_FROM_CLASS(klass),
					 G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL,
					 G_TYPE_NONE, 3, G_TYPE_POINTER,
					 G_TYPE_POINTER, PURPLE_MEDIA_TYPE_CANDIDATE);
	purple_media_signals[STATE_CHANGED] = g_signal_new("state-changed", G_TYPE_FROM_CLASS(klass),
					 G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL,
					 G_TYPE_NONE, 3, PURPLE_MEDIA_TYPE_STATE,
					 G_TYPE_STRING, G_TYPE_STRING);
	purple_media_signals[STREAM_INFO] = g_signal_new("stream-info", G_TYPE_FROM_CLASS(klass),
					 G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL,
					 G_TYPE_NONE, 4, PURPLE_MEDIA_TYPE_INFO_TYPE,
					 G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN);
	purple_media_signals[CANDIDATE_PAIR_ESTABLISHED] = g_signal_new("candidate-pair-established", G_TYPE_FROM_CLASS(klass),
					 G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL,
					 G_TYPE_NONE, 4, G_TYPE_POINTER, G_TYPE_POINTER,
					 PURPLE_MEDIA_TYPE_CANDIDATE, PURPLE_MEDIA_TYPE_CANDIDATE);
}


static void
purple_media_init (PurpleMedia *media)
{
	media->priv = purple_media_get_instance_private(media);
	memset(media->priv, 0, sizeof(*media->priv));
}

static void
purple_media_stream_free(PurpleMediaStream *stream)
{
	if (stream == NULL) {
		return;
	}

	g_free(stream->participant);

	g_clear_pointer(&stream->local_candidates,
	                purple_media_candidate_list_free);
	g_clear_pointer(&stream->remote_candidates,
	                purple_media_candidate_list_free);
	g_clear_pointer(&stream->active_local_candidates,
	                purple_media_candidate_list_free);
	g_clear_pointer(&stream->active_remote_candidates,
	                purple_media_candidate_list_free);

	g_free(stream);
}

static void
purple_media_session_free(PurpleMediaSession *session)
{
	if (session == NULL) {
		return;
	}

	g_free(session->id);
	g_free(session);
}

static void
purple_media_dispose(GObject *media)
{
	PurpleMediaPrivate *priv =
			purple_media_get_instance_private(PURPLE_MEDIA(media));

	purple_debug_info("media","purple_media_dispose\n");

	purple_media_manager_remove_media(priv->manager, PURPLE_MEDIA(media));

	g_clear_object(&priv->backend);
	g_clear_object(&priv->manager);
	g_clear_pointer(&priv->conference_type, g_free);

	G_OBJECT_CLASS(purple_media_parent_class)->dispose(media);
}

static void
purple_media_finalize(GObject *media)
{
	PurpleMediaPrivate *priv =
			purple_media_get_instance_private(PURPLE_MEDIA(media));
	purple_debug_info("media","purple_media_finalize\n");

	g_list_free_full(priv->streams, (GDestroyNotify)purple_media_stream_free);
	g_list_free_full(priv->participants, g_free);

	if (priv->sessions) {
		GList *sessions = g_hash_table_get_values(priv->sessions);
		g_list_free_full(sessions, (GDestroyNotify)purple_media_session_free);
		g_hash_table_destroy(priv->sessions);
	}

	G_OBJECT_CLASS(purple_media_parent_class)->finalize(media);
}

static void
purple_media_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	PurpleMedia *media;
	g_return_if_fail(PURPLE_IS_MEDIA(object));

	media = PURPLE_MEDIA(object);

	switch (prop_id) {
		case PROP_MANAGER:
			media->priv->manager = g_value_dup_object(value);
			break;
		case PROP_ACCOUNT:
			media->priv->account = g_value_get_object(value);
			break;
		case PROP_CONFERENCE_TYPE:
			media->priv->conference_type = g_value_dup_string(value);
			media->priv->backend = g_object_new(
					purple_media_manager_get_backend_type(purple_media_manager_get()),
					"conference-type", media->priv->conference_type,
					"media", media,
					NULL);
			g_signal_connect(media->priv->backend,
					"active-candidate-pair",
					G_CALLBACK(purple_media_candidate_pair_established_cb),
					media);
			g_signal_connect(media->priv->backend,
					"candidates-prepared",
					G_CALLBACK(purple_media_candidates_prepared_cb),
					media);
			g_signal_connect(media->priv->backend,
					"codecs-changed",
					G_CALLBACK(purple_media_codecs_changed_cb),
					media);
			g_signal_connect(media->priv->backend,
					"new-candidate",
					G_CALLBACK(purple_media_new_local_candidate_cb),
					media);
			break;
		case PROP_INITIATOR:
			media->priv->initiator = g_value_get_boolean(value);
			break;
		case PROP_PROTOCOL_DATA:
			media->priv->protocol_data = g_value_get_pointer(value);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
purple_media_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	PurpleMedia *media;
	g_return_if_fail(PURPLE_IS_MEDIA(object));

	media = PURPLE_MEDIA(object);

	switch (prop_id) {
		case PROP_MANAGER:
			g_value_set_object(value, media->priv->manager);
			break;
		case PROP_BACKEND:
			g_value_set_object(value, media->priv->backend);
			break;
		case PROP_ACCOUNT:
			g_value_set_object(value, media->priv->account);
			break;
		case PROP_CONFERENCE_TYPE:
			g_value_set_string(value, media->priv->conference_type);
			break;
		case PROP_INITIATOR:
			g_value_set_boolean(value, media->priv->initiator);
			break;
		case PROP_PROTOCOL_DATA:
			g_value_set_pointer(value, media->priv->protocol_data);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}

}

static PurpleMediaSession*
purple_media_get_session(PurpleMedia *media, const gchar *sess_id)
{
	g_return_val_if_fail(PURPLE_IS_MEDIA(media), NULL);
	return (PurpleMediaSession*) (media->priv->sessions) ?
			g_hash_table_lookup(media->priv->sessions, sess_id) : NULL;
}

static PurpleMediaStream*
purple_media_get_stream(PurpleMedia *media, const gchar *session, const gchar *participant)
{
	GList *streams;

	g_return_val_if_fail(PURPLE_IS_MEDIA(media), NULL);

	streams = media->priv->streams;

	for (; streams; streams = g_list_next(streams)) {
		PurpleMediaStream *stream = streams->data;
		if (purple_strequal(stream->session->id, session) &&
				purple_strequal(stream->participant, participant))
		{
			return stream;
		}
	}

	return NULL;
}

static GList *
purple_media_get_streams(PurpleMedia *media, const gchar *session,
		const gchar *participant)
{
	GList *streams;
	GList *ret = NULL;

	g_return_val_if_fail(PURPLE_IS_MEDIA(media), NULL);

	streams = media->priv->streams;

	for (; streams; streams = g_list_next(streams)) {
		PurpleMediaStream *stream = streams->data;
		if ((session == NULL ||
				purple_strequal(stream->session->id, session)) &&
				(participant == NULL ||
				purple_strequal(stream->participant, participant)))
		{
			ret = g_list_append(ret, stream);
		}
	}

	return ret;
}

static void
purple_media_add_session(PurpleMedia *media, PurpleMediaSession *session)
{
	g_return_if_fail(PURPLE_IS_MEDIA(media));
	g_return_if_fail(session != NULL);

	if (!media->priv->sessions) {
		purple_debug_info("media", "Creating hash table for sessions\n");
		media->priv->sessions = g_hash_table_new_full(g_str_hash, g_str_equal,
		                                              g_free, NULL);
	}
	g_hash_table_insert(media->priv->sessions, g_strdup(session->id), session);
}

static PurpleMediaStream *
purple_media_insert_stream(PurpleMediaSession *session,
		const gchar *name, gboolean initiator)
{
	PurpleMediaStream *media_stream;

	g_return_val_if_fail(session != NULL, NULL);

	media_stream = g_new0(PurpleMediaStream, 1);
	media_stream->participant = g_strdup(name);
	media_stream->session = session;
	media_stream->initiator = initiator;

	session->media->priv->streams =
			g_list_append(session->media->priv->streams, media_stream);

	return media_stream;
}

static void
purple_media_insert_local_candidate(PurpleMediaSession *session, const gchar *name,
				     PurpleMediaCandidate *candidate)
{
	PurpleMediaStream *stream;

	g_return_if_fail(session != NULL);

	stream = purple_media_get_stream(session->media, session->id, name);
	stream->local_candidates = g_list_append(stream->local_candidates, candidate);
}

GList *
purple_media_get_session_ids(PurpleMedia *media)
{
	g_return_val_if_fail(PURPLE_IS_MEDIA(media), NULL);
	return media->priv->sessions != NULL ?
			g_hash_table_get_keys(media->priv->sessions) : NULL;
}

GstElement *
purple_media_get_src(PurpleMedia *media, G_GNUC_UNUSED const char *sess_id)
{
	g_return_val_if_fail(PURPLE_IS_MEDIA(media), NULL);

	return NULL;
}

PurpleAccount *
purple_media_get_account(PurpleMedia *media)
{
	PurpleAccount *account;
	g_return_val_if_fail(PURPLE_IS_MEDIA(media), NULL);
	g_object_get(G_OBJECT(media), "account", &account, NULL);
	return account;
}

gpointer
purple_media_get_protocol_data(PurpleMedia *media)
{
	gpointer protocol_data;
	g_return_val_if_fail(PURPLE_IS_MEDIA(media), NULL);
	g_object_get(G_OBJECT(media), "protocol-data", &protocol_data, NULL);
	return protocol_data;
}

void
purple_media_set_protocol_data(PurpleMedia *media, gpointer protocol_data)
{
	g_return_if_fail(PURPLE_IS_MEDIA(media));
	g_object_set(G_OBJECT(media), "protocol-data", protocol_data, NULL);
}

void
purple_media_error(PurpleMedia *media, const gchar *error, ...)
{
	va_list args;
	gchar *message;

	g_return_if_fail(PURPLE_IS_MEDIA(media));

	va_start(args, error);
	message = g_strdup_vprintf(error, args);
	va_end(args);

	purple_debug_error("media", "%s\n", message);
	g_signal_emit(media, purple_media_signals[S_ERROR], 0, message);

	g_free(message);
}

void
purple_media_end(PurpleMedia *media,
		const gchar *session_id, const gchar *participant)
{
	GList *iter, *sessions = NULL, *participants = NULL;

	g_return_if_fail(PURPLE_IS_MEDIA(media));

	iter = purple_media_get_streams(media, session_id, participant);

	/* Free matching streams */
	for (; iter; iter = g_list_delete_link(iter, iter)) {
		PurpleMediaStream *stream = iter->data;

		g_signal_emit(media, purple_media_signals[STATE_CHANGED],
				0, PURPLE_MEDIA_STATE_END,
				stream->session->id, stream->participant);

		media->priv->streams =
				g_list_remove(media->priv->streams, stream);

		if (g_list_find(sessions, stream->session) == NULL) {
			sessions = g_list_prepend(sessions, stream->session);
		}

		if (g_list_find_custom(participants, stream->participant,
				(GCompareFunc)strcmp) == NULL)
		{
			participants = g_list_prepend(participants,
					g_strdup(stream->participant));
		}

		purple_media_stream_free(stream);
	}

	iter = media->priv->streams;

	/* Reduce to list of sessions to remove */
	for (; iter; iter = g_list_next(iter)) {
		PurpleMediaStream *stream = iter->data;

		sessions = g_list_remove(sessions, stream->session);
	}

	/* Free sessions with no streams left */
	for (; sessions; sessions = g_list_delete_link(sessions, sessions)) {
		PurpleMediaSession *session = sessions->data;

		g_signal_emit(media, purple_media_signals[STATE_CHANGED],
				0, PURPLE_MEDIA_STATE_END,
				session->id, NULL);

		g_hash_table_remove(media->priv->sessions, session->id);
		purple_media_session_free(session);
	}

	iter = media->priv->streams;

	/* Reduce to list of participants to remove */
	for (; iter; iter = g_list_next(iter)) {
		PurpleMediaStream *stream = iter->data;
		GList *tmp;

		tmp = g_list_find_custom(participants,
				stream->participant, (GCompareFunc)strcmp);

		if (tmp != NULL) {
			g_free(tmp->data);
			participants = g_list_delete_link(participants,	tmp);
		}
	}

	/* Remove participants with no streams left (just emit the signal) */
	for (; participants; participants =
			g_list_delete_link(participants, participants)) {
		gchar *participant = participants->data;
		GList *link = g_list_find_custom(media->priv->participants,
				participant, (GCompareFunc)strcmp);

		g_signal_emit(media, purple_media_signals[STATE_CHANGED],
				0, PURPLE_MEDIA_STATE_END,
				NULL, participant);

		if (link != NULL) {
			g_free(link->data);
			media->priv->participants = g_list_delete_link(
					media->priv->participants, link);
		}

		g_free(participant);
	}

	/* Free the conference if no sessions left */
	if (media->priv->sessions != NULL &&
			g_hash_table_size(media->priv->sessions) == 0) {
		g_signal_emit(media, purple_media_signals[STATE_CHANGED],
				0, PURPLE_MEDIA_STATE_END,
				NULL, NULL);
		g_object_unref(media);
		return;
	}
}

void
purple_media_stream_info(PurpleMedia *media, PurpleMediaInfoType type,
		const gchar *session_id, const gchar *participant,
		gboolean local)
{
	g_return_if_fail(PURPLE_IS_MEDIA(media));

	if (type == PURPLE_MEDIA_INFO_ACCEPT) {
		GList *streams, *sessions = NULL, *participants = NULL;

		g_return_if_fail(PURPLE_IS_MEDIA(media));

		streams = purple_media_get_streams(media, session_id, participant);

		/* Emit stream acceptance */
		for (; streams; streams = g_list_delete_link(streams, streams)) {
			PurpleMediaStream *stream = streams->data;

			stream->accepted = TRUE;

			g_signal_emit(media,
					purple_media_signals[STREAM_INFO],
					0, type, stream->session->id,
					stream->participant, local);

			if (g_list_find(sessions, stream->session) == NULL) {
				sessions = g_list_prepend(sessions, stream->session);
			}

			if (g_list_find_custom(participants,
					stream->participant,
					(GCompareFunc)strcmp) == NULL) {
				participants = g_list_prepend(participants,
						g_strdup(stream->participant));
			}
		}

		/* Emit session acceptance */
		for (; sessions; sessions =
				g_list_delete_link(sessions, sessions)) {
			PurpleMediaSession *session = sessions->data;

			if (purple_media_accepted(media, session->id, NULL)) {
				g_signal_emit(media, purple_media_signals[
						STREAM_INFO], 0,
						PURPLE_MEDIA_INFO_ACCEPT,
						session->id, NULL, local);
			}
		}

		/* Emit participant acceptance */
		for (; participants; participants = g_list_delete_link(
				participants, participants)) {
			gchar *participant = participants->data;

			if (purple_media_accepted(media, NULL, participant)) {
				g_signal_emit(media, purple_media_signals[
						STREAM_INFO], 0,
						PURPLE_MEDIA_INFO_ACCEPT,
						NULL, participant, local);
			}

			g_free(participant);
		}

		/* Emit conference acceptance */
		if (purple_media_accepted(media, NULL, NULL)) {
			g_signal_emit(media,
					purple_media_signals[STREAM_INFO],
					0, PURPLE_MEDIA_INFO_ACCEPT,
					NULL, NULL, local);
		}

		return;
	} else if (type == PURPLE_MEDIA_INFO_HANGUP ||
			type == PURPLE_MEDIA_INFO_REJECT) {
		GList *streams;

		g_return_if_fail(PURPLE_IS_MEDIA(media));

		streams = purple_media_get_streams(media, session_id, participant);

		/* Emit for stream */
		for (; streams; streams = g_list_delete_link(streams, streams)) {
			PurpleMediaStream *stream = streams->data;

			g_signal_emit(media,
					purple_media_signals[STREAM_INFO],
					0, type, stream->session->id,
					stream->participant, local);
		}

		if (session_id != NULL && participant != NULL) {
			/* Everything that needs to be emitted has been */
		} else if (session_id == NULL && participant == NULL) {
			/* Emit for everything in the conference */
			GList *sessions = NULL;
			GList *participants = media->priv->participants;

			if (media->priv->sessions != NULL) {
				sessions = g_hash_table_get_values(
					media->priv->sessions);
			}

			/* Emit for sessions */
			for (; sessions; sessions = g_list_delete_link(
					sessions, sessions)) {
				PurpleMediaSession *session = sessions->data;

				g_signal_emit(media, purple_media_signals[
						STREAM_INFO], 0, type,
						session->id, NULL, local);
			}

			/* Emit for participants */
			for (; participants; participants =
					g_list_next(participants)) {
				gchar *participant = participants->data;

				g_signal_emit(media, purple_media_signals[
						STREAM_INFO], 0, type,
						NULL, participant, local);
			}

			/* Emit for conference */
			g_signal_emit(media,
					purple_media_signals[STREAM_INFO],
					0, type, NULL, NULL, local);
		} else if (session_id != NULL) {
			/* Emit just the specific session */
			PurpleMediaSession *session =
					purple_media_get_session(
					media, session_id);

			if (session == NULL) {
				purple_debug_warning("media",
						"Couldn't find session"
						" to hangup/reject.\n");
			} else {
				g_signal_emit(media, purple_media_signals[
						STREAM_INFO], 0, type,
						session->id, NULL, local);
			}
		} else if (participant != NULL) {
			/* Emit just the specific participant */
			if (!g_list_find_custom(media->priv->participants,
					participant, (GCompareFunc)strcmp)) {
				purple_debug_warning("media",
						"Couldn't find participant"
						" to hangup/reject.\n");
			} else {
				g_signal_emit(media, purple_media_signals[
						STREAM_INFO], 0, type, NULL,
						participant, local);
			}
		}

		purple_media_end(media, session_id, participant);
		return;
	}

	g_signal_emit(media, purple_media_signals[STREAM_INFO],
			0, type, session_id, participant, local);
}

void
purple_media_set_params(PurpleMedia *media, GHashTable *params)
{
	g_return_if_fail(PURPLE_IS_MEDIA(media));

	purple_media_backend_set_params(media->priv->backend, params);
}

const gchar **
purple_media_get_available_params(PurpleMedia *media)
{
	static const gchar *NULL_ARRAY[] = { NULL };
	g_return_val_if_fail(PURPLE_IS_MEDIA(media), NULL_ARRAY);

	return purple_media_backend_get_available_params(media->priv->backend);
}

gboolean
purple_media_param_is_supported(PurpleMedia *media, const gchar *param)
{
	const gchar **params;

	g_return_val_if_fail(PURPLE_IS_MEDIA(media), FALSE);
	g_return_val_if_fail(param != NULL, FALSE);

	params = purple_media_backend_get_available_params(media->priv->backend);
	for (; *params != NULL; ++params) {
		if (purple_strequal(*params, param)) {
			return TRUE;
		}
	}

	return FALSE;
}

static void
purple_media_new_local_candidate_cb(G_GNUC_UNUSED PurpleMediaBackend *backend,
		const gchar *sess_id, const gchar *participant,
		PurpleMediaCandidate *candidate, PurpleMedia *media)
{
	PurpleMediaSession *session =
			purple_media_get_session(media, sess_id);

	purple_media_insert_local_candidate(session, participant,
			purple_media_candidate_copy(candidate));

	g_signal_emit(session->media, purple_media_signals[NEW_CANDIDATE],
		      0, session->id, participant, candidate);
}

static void
purple_media_candidates_prepared_cb(G_GNUC_UNUSED PurpleMediaBackend *backend,
		const gchar *sess_id, const gchar *name, PurpleMedia *media)
{
	PurpleMediaStream *stream_data;

	g_return_if_fail(PURPLE_IS_MEDIA(media));

	stream_data = purple_media_get_stream(media, sess_id, name);
	stream_data->candidates_prepared = TRUE;

	g_signal_emit(media, purple_media_signals[CANDIDATES_PREPARED],
			0, sess_id, name);
}

/* callback called when a pair of transport candidates (local and remote)
 * has been established */
static void
purple_media_candidate_pair_established_cb(G_GNUC_UNUSED PurpleMediaBackend *backend,
		const gchar *sess_id, const gchar *name,
		PurpleMediaCandidate *local_candidate,
		PurpleMediaCandidate *remote_candidate,
		PurpleMedia *media)
{
	PurpleMediaStream *stream;
	GList *iter;
	guint id;

	g_return_if_fail(PURPLE_IS_MEDIA(media));

	stream = purple_media_get_stream(media, sess_id, name);
	id = purple_media_candidate_get_component_id(local_candidate);

	iter = stream->active_local_candidates;
	for(; iter; iter = g_list_next(iter)) {
		PurpleMediaCandidate *c = iter->data;
		if (id == purple_media_candidate_get_component_id(c)) {
			g_object_unref(c);
			stream->active_local_candidates = g_list_delete_link(
					stream->active_local_candidates,
					iter);
			break;
		}
	}
	stream->active_local_candidates = g_list_prepend(
			stream->active_local_candidates,
			purple_media_candidate_copy(
			local_candidate));

	id = purple_media_candidate_get_component_id(local_candidate);

	iter = stream->active_remote_candidates;
	for(; iter; iter = g_list_next(iter)) {
		PurpleMediaCandidate *c = iter->data;
		if (id == purple_media_candidate_get_component_id(c)) {
			g_object_unref(c);
			stream->active_remote_candidates = g_list_delete_link(
					stream->active_remote_candidates,
					iter);
			break;
		}
	}
	stream->active_remote_candidates = g_list_prepend(
			stream->active_remote_candidates,
			purple_media_candidate_copy(
			remote_candidate));

	g_signal_emit(media, purple_media_signals[CANDIDATE_PAIR_ESTABLISHED],
		0, sess_id, name, local_candidate, remote_candidate);
	purple_debug_info("media", "candidate pair established\n");
}

static void
purple_media_codecs_changed_cb(G_GNUC_UNUSED PurpleMediaBackend *backend,
		const gchar *sess_id, PurpleMedia *media)
{
	g_signal_emit(media, purple_media_signals[CODECS_CHANGED], 0, sess_id);
}

gboolean
purple_media_add_stream(PurpleMedia *media, const gchar *sess_id,
		const gchar *who, PurpleMediaSessionType type, gboolean initiator,
		const gchar *transmitter, GHashTable *params)
{
	PurpleMediaSession *session;

	g_return_val_if_fail(PURPLE_IS_MEDIA(media), FALSE);

	if (!purple_media_backend_add_stream(media->priv->backend,
			sess_id, who, type, initiator, transmitter, params))
	{
		purple_debug_error("media", "Error adding stream.\n");
		return FALSE;
	}

	session = purple_media_get_session(media, sess_id);

	if (!session) {
		session = g_new0(PurpleMediaSession, 1);
		session->id = g_strdup(sess_id);
		session->media = media;
		session->type = type;
		session->initiator = initiator;

		purple_media_add_session(media, session);
		g_signal_emit(media, purple_media_signals[STATE_CHANGED],
				0, PURPLE_MEDIA_STATE_NEW,
				session->id, NULL);
	}

	if (!g_list_find_custom(media->priv->participants,
			who, (GCompareFunc)strcmp)) {
		media->priv->participants = g_list_prepend(
				media->priv->participants, g_strdup(who));

		g_signal_emit(media, purple_media_signals[STATE_CHANGED], 0,
				PURPLE_MEDIA_STATE_NEW, NULL, who);
	}

	if (purple_media_get_stream(media, sess_id, who) == NULL) {
		purple_media_insert_stream(session, who, initiator);

		g_signal_emit(media, purple_media_signals[STATE_CHANGED],
				0, PURPLE_MEDIA_STATE_NEW,
				session->id, who);
	}

	return TRUE;
}

PurpleMediaManager *
purple_media_get_manager(PurpleMedia *media)
{
	PurpleMediaManager *ret;
	g_return_val_if_fail(PURPLE_IS_MEDIA(media), NULL);
	g_object_get(media, "manager", &ret, NULL);
	return ret;
}

PurpleMediaSessionType
purple_media_get_session_type(PurpleMedia *media, const gchar *sess_id)
{
	PurpleMediaSession *session;
	g_return_val_if_fail(PURPLE_IS_MEDIA(media), PURPLE_MEDIA_NONE);
	session = purple_media_get_session(media, sess_id);
	return session->type;
}

/* XXX: Should wait until codecs-ready is TRUE before using this function */
GList *
purple_media_get_codecs(PurpleMedia *media, const gchar *sess_id)
{
	g_return_val_if_fail(PURPLE_IS_MEDIA(media), NULL);

	return purple_media_backend_get_codecs(media->priv->backend, sess_id);
}

GList *
purple_media_get_local_candidates(PurpleMedia *media, const gchar *sess_id,
                                  const gchar *participant)
{
	g_return_val_if_fail(PURPLE_IS_MEDIA(media), NULL);

	return purple_media_backend_get_local_candidates(media->priv->backend,
			sess_id, participant);
}

void
purple_media_add_remote_candidates(PurpleMedia *media, const gchar *sess_id,
                                   const gchar *participant,
                                   GList *remote_candidates)
{
	PurpleMediaStream *stream;

	g_return_if_fail(PURPLE_IS_MEDIA(media));
	stream = purple_media_get_stream(media, sess_id, participant);

	if (stream == NULL) {
		purple_debug_error("media",
				"purple_media_add_remote_candidates: "
				"couldn't find stream %s %s.\n",
				sess_id ? sess_id : "(null)",
				participant ? participant : "(null)");
		return;
	}

	stream->remote_candidates = g_list_concat(stream->remote_candidates,
			purple_media_candidate_list_copy(remote_candidates));

	purple_media_backend_add_remote_candidates(media->priv->backend,
			sess_id, participant, remote_candidates);
}

GList *
purple_media_get_active_local_candidates(PurpleMedia *media,
		const gchar *sess_id, const gchar *participant)
{
	PurpleMediaStream *stream;
	g_return_val_if_fail(PURPLE_IS_MEDIA(media), NULL);
	stream = purple_media_get_stream(media, sess_id, participant);
	return purple_media_candidate_list_copy(
			stream->active_local_candidates);
}

GList *
purple_media_get_active_remote_candidates(PurpleMedia *media,
		const gchar *sess_id, const gchar *participant)
{
	PurpleMediaStream *stream;
	g_return_val_if_fail(PURPLE_IS_MEDIA(media), NULL);
	stream = purple_media_get_stream(media, sess_id, participant);
	return purple_media_candidate_list_copy(
			stream->active_remote_candidates);
}

gboolean
purple_media_set_remote_codecs(PurpleMedia *media, const gchar *sess_id,
                               const gchar *participant, GList *codecs)
{
	g_return_val_if_fail(PURPLE_IS_MEDIA(media), FALSE);

	return purple_media_backend_set_remote_codecs(media->priv->backend,
			sess_id, participant, codecs);
}

gboolean
purple_media_candidates_prepared(PurpleMedia *media,
		const gchar *session_id, const gchar *participant)
{
	GList *streams;
	gboolean prepared = TRUE;

	g_return_val_if_fail(PURPLE_IS_MEDIA(media), FALSE);

	streams = purple_media_get_streams(media, session_id, participant);

	for (; streams; streams = g_list_delete_link(streams, streams)) {
		PurpleMediaStream *stream = streams->data;
		if (stream->candidates_prepared == FALSE) {
			g_list_free(streams);
			prepared = FALSE;
			break;
		}
	}

	return prepared;
}

gboolean
purple_media_set_send_codec(PurpleMedia *media, const gchar *sess_id, PurpleMediaCodec *codec)
{
	g_return_val_if_fail(PURPLE_IS_MEDIA(media), FALSE);

	return purple_media_backend_set_send_codec(
			media->priv->backend, sess_id, codec);
}

gboolean
purple_media_set_encryption_parameters(PurpleMedia *media, const gchar *sess_id,
		const gchar *cipher, const gchar *auth,
		const gchar *key, gsize key_len)
{
	g_return_val_if_fail(PURPLE_IS_MEDIA(media), FALSE);
	return purple_media_backend_set_encryption_parameters(media->priv->backend,
			sess_id, cipher, auth, key, key_len);
}

gboolean
purple_media_set_decryption_parameters(PurpleMedia *media, const gchar *sess_id,
		const gchar *participant, const gchar *cipher,
		const gchar *auth, const gchar *key, gsize key_len)
{
	g_return_val_if_fail(PURPLE_IS_MEDIA(media), FALSE);
	return purple_media_backend_set_decryption_parameters(media->priv->backend,
			sess_id, participant, cipher, auth, key, key_len);
}

gboolean
purple_media_set_require_encryption(PurpleMedia *media, const gchar *sess_id,
                const gchar *participant, gboolean require_encryption)
{
	g_return_val_if_fail(PURPLE_IS_MEDIA(media), FALSE);
	return purple_media_backend_set_require_encryption(media->priv->backend,
			sess_id, participant, require_encryption);
}

gboolean
purple_media_codecs_ready(PurpleMedia *media, const gchar *sess_id)
{
	g_return_val_if_fail(PURPLE_IS_MEDIA(media), FALSE);

	return purple_media_backend_codecs_ready(
			media->priv->backend, sess_id);
}

gboolean
purple_media_set_send_rtcp_mux(PurpleMedia *media, const gchar *sess_id,
                               const gchar *participant, gboolean send_rtcp_mux)
{
	g_return_val_if_fail(PURPLE_IS_MEDIA(media), FALSE);

	return purple_media_backend_set_send_rtcp_mux(media->priv->backend,
			sess_id, participant, send_rtcp_mux);
}

gboolean
purple_media_is_initiator(PurpleMedia *media,
		const gchar *sess_id, const gchar *participant)
{
	g_return_val_if_fail(PURPLE_IS_MEDIA(media), FALSE);

	if (sess_id == NULL && participant == NULL)
		return media->priv->initiator;
	else if (sess_id != NULL && participant == NULL) {
		PurpleMediaSession *session =
				purple_media_get_session(media, sess_id);
		return session != NULL ? session->initiator : FALSE;
	} else if (sess_id != NULL && participant != NULL) {
		PurpleMediaStream *stream = purple_media_get_stream(
				media, sess_id, participant);
		return stream != NULL ? stream->initiator : FALSE;
	}

	return FALSE;
}

gboolean
purple_media_accepted(PurpleMedia *media, const gchar *sess_id,
		const gchar *participant)
{
	gboolean accepted = TRUE;

	g_return_val_if_fail(PURPLE_IS_MEDIA(media), FALSE);

	if (sess_id == NULL && participant == NULL) {
		GList *streams = media->priv->streams;

		for (; streams; streams = g_list_next(streams)) {
			PurpleMediaStream *stream = streams->data;
			if (stream->accepted == FALSE) {
				accepted = FALSE;
				break;
			}
		}
	} else if (sess_id != NULL && participant == NULL) {
		GList *streams = purple_media_get_streams(media, sess_id, NULL);
		for (; streams; streams = g_list_delete_link(streams, streams)) {
			PurpleMediaStream *stream = streams->data;
			if (stream->accepted == FALSE) {
				g_list_free(streams);
				accepted = FALSE;
				break;
			}
		}
	} else if (sess_id != NULL && participant != NULL) {
		PurpleMediaStream *stream = purple_media_get_stream(
				media, sess_id, participant);
		if (stream == NULL || stream->accepted == FALSE) {
			accepted = FALSE;
		}
	}

	return accepted;
}

void
purple_media_set_input_volume(PurpleMedia *media,
                              G_GNUC_UNUSED const char *session_id,
                              G_GNUC_UNUSED double level)
{
	g_return_if_fail(PURPLE_IS_MEDIA(media));
}

void
purple_media_set_output_volume(PurpleMedia *media,
                               G_GNUC_UNUSED const char *session_id,
                               G_GNUC_UNUSED const char *participant,
                               G_GNUC_UNUSED double level)
{
	g_return_if_fail(PURPLE_IS_MEDIA(media));
}

gulong
purple_media_set_output_window(PurpleMedia *media, const gchar *session_id,
		const gchar *participant)
{
	g_return_val_if_fail(PURPLE_IS_MEDIA(media), FALSE);

	return purple_media_manager_set_output_window(media->priv->manager,
			media, session_id, participant);
}

void
purple_media_remove_output_windows(PurpleMedia *media)
{
	GList *iter = media->priv->streams;
	for (; iter; iter = g_list_next(iter)) {
		PurpleMediaStream *stream = iter->data;
		purple_media_manager_remove_output_windows(
				media->priv->manager, media,
				stream->session->id, stream->participant);
	}

	iter = purple_media_get_session_ids(media);
	for (; iter; iter = g_list_delete_link(iter, iter)) {
		gchar *session_name = iter->data;
		purple_media_manager_remove_output_windows(
				media->priv->manager, media,
				session_name, NULL);
	}
}

GstElement *
purple_media_get_tee(PurpleMedia *media, G_GNUC_UNUSED const char *session_id,
                     G_GNUC_UNUSED const char *participant)
{
	g_return_val_if_fail(PURPLE_IS_MEDIA(media), NULL);

	return NULL;
}

gboolean
purple_media_send_dtmf(PurpleMedia *media, const gchar *session_id,
		gchar dtmf, guint8 volume, guint16 duration)
{
	PurpleMediaBackendInterface *backend_iface = NULL;

	if (media) {
		backend_iface = PURPLE_MEDIA_BACKEND_GET_INTERFACE(media->priv->backend);
	}

	if (dtmf == 'a') {
		dtmf = 'A';
	} else if (dtmf == 'b') {
		dtmf = 'B';
	} else if (dtmf == 'c') {
		dtmf = 'C';
	} else if (dtmf == 'd') {
		dtmf = 'D';
	}

	g_return_val_if_fail(strchr("0123456789ABCD#*", dtmf), FALSE);

	if (backend_iface && backend_iface->send_dtmf
		&& backend_iface->send_dtmf(media->priv->backend,
				session_id, dtmf, volume, duration))
	{
		return TRUE;
	}

	return FALSE;
}
