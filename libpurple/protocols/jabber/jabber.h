/**
 * @file jabber.h
 *
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
 */

#ifndef PURPLE_JABBER_JABBER_H
#define PURPLE_JABBER_JABBER_H

typedef enum {
	JABBER_CAP_NONE           = 0,
/*	JABBER_CAP_XHTML          = 1 << 0, */
/*	JABBER_CAP_COMPOSING      = 1 << 1, */
	JABBER_CAP_SI             = 1 << 2,
	JABBER_CAP_SI_FILE_XFER   = 1 << 3,
	JABBER_CAP_BYTESTREAMS    = 1 << 4,
	JABBER_CAP_IBB            = 1 << 5,
	JABBER_CAP_CHAT_STATES    = 1 << 6,
	JABBER_CAP_IQ_SEARCH      = 1 << 7,
	JABBER_CAP_IQ_REGISTER    = 1 << 8,

	JABBER_CAP_PING           = 1 << 11,
	JABBER_CAP_ADHOC          = 1 << 12,
	JABBER_CAP_BLOCKING       = 1 << 13,

	JABBER_CAP_ITEMS          = 1 << 14,
	JABBER_CAP_ROSTER_VERSIONING = 1 << 15,

	JABBER_CAP_MESSAGE_CARBONS = 1 << 19,

	JABBER_CAP_RETRIEVED      = 1 << 31
} JabberCapabilities;

typedef struct _JabberStream JabberStream;

#include <libxml/parser.h>
#include <glib.h>
#include <gmodule.h>
#include <gio/gio.h>
#include <libsoup/soup.h>

#include <config.h>

#include <purple.h>

#include "namespaces.h"

#include "auth.h"
#include "iq.h"
#include "jutil.h"
#include "buddy.h"
#include "bosh.h"

#define CAPS0115_NODE "https://pidgin.im/"

#define JABBER_TYPE_PROTOCOL (jabber_protocol_get_type())
G_DECLARE_DERIVABLE_TYPE(JabberProtocol, jabber_protocol, JABBER, PROTOCOL,
                         PurpleProtocol)

#define JABBER_DEFAULT_REQUIRE_TLS    "require_starttls"

/* Index into attention_types list */
#define JABBER_BUZZ 0

typedef enum {
	JABBER_STREAM_OFFLINE,
	JABBER_STREAM_CONNECTING,
	JABBER_STREAM_INITIALIZING,
	JABBER_STREAM_INITIALIZING_ENCRYPTION,
	JABBER_STREAM_AUTHENTICATING,
	JABBER_STREAM_POST_AUTH,
	JABBER_STREAM_CONNECTED
} JabberStreamState;

struct _JabberProtocolClass {
	PurpleProtocolClass parent_class;
};

struct _JabberStream
{
	guint inpa;

	GCancellable *cancellable;

	xmlParserCtxt *context;
	PurpleXmlNode *current;

	struct {
		guint8 major;
		guint8 minor;
	} protocol_version;

	JabberSaslMech *auth_mech;
	gpointer auth_mech_data;

	/**
	 * The header from the opening <stream/> tag.  This being NULL is treated
	 * as a special condition in the parsing code (signifying the next
	 * stanza started is an opening stream tag), and its being missing on
	 * the stream header is treated as a fatal error.
	 */
	char *stream_id;
	JabberStreamState state;

	GHashTable *buddies;

	/*
	 * This boolean was added to eliminate a heinous bug where we would
	 * get into a loop with the server and move a buddy back and forth
	 * from one group to another.
	 *
	 * The sequence goes something like this:
	 * 1. Our resource and another resource both approve an authorization
	 *    request at the exact same time.  We put the buddy in group A and
	 *    the other resource put the buddy in group B.
	 * 2. The server receives the roster add for group B and sends us a
	 *    roster push.
	 * 3. We receive this roster push and modify our local blist.  This
	 *    triggers us to send a roster add for group B.
	 * 4. The server receives our earlier roster add for group A and sends us a
	 *    roster push.
	 * 5. We receive this roster push and modify our local blist.  This
	 *    triggers us to send a roster add for group A.
	 * 6. The server receives our earlier roster add for group B and sends
	 *    us a roster push.
	 * (repeat steps 3 through 6 ad infinitum)
	 *
	 * This boolean is used to short-circuit the sending of a roster add
	 * when we receive a roster push.
	 *
	 * See these bug reports:
	 * https://trac.adium.im/ticket/8834
	 * https://developer.pidgin.im/ticket/5484
	 * https://developer.pidgin.im/ticket/6188
	 */
	gboolean currently_parsing_roster_push;

	GHashTable *chats;
	GList *chat_servers;
	PurpleRoomlist *roomlist;

	GHashTable *iq_callbacks;
	int next_id;

	GList *bs_proxies;
	GList *oob_file_transfers;
	GList *file_transfers;

	time_t idle;
	time_t old_idle;

	JabberID *user;
	JabberBuddy *user_jb;

	PurpleConnection *gc;
	GSocketClient *client;
	GIOStream *stream;
	GInputStream *input;
	PurpleQueuedOutputStream *output;

	char *initial_avatar_hash;
	char *avatar_hash;
	GSList *pending_avatar_requests;

	GSList *pending_buddy_info_requests;

	gboolean reinit;

	JabberCapabilities server_caps;
	char *server_name;

	char *serverFQDN;

	gboolean vcard_fetched;

	/* Entity Capabilities hash */
	char *caps_hash;

	/* does the local server support PEP? */
	gboolean pep;

	/* Is Buzz enabled? */
	gboolean allowBuzz;

	/* A list of JabberAdHocCommands supported by the server */
	GList *commands;

	/* last presence update to check for differences */
	JabberBuddyState old_state;
	char *old_msg;
	int old_priority;
	char *old_avatarhash;

	/* same for user tune */
	char *old_artist;
	char *old_title;
	char *old_source;
	char *old_uri;
	int old_length;
	char *old_track;

	char *certificate_CN;

	/* A purple timeout tag for the keepalive */
	guint keepalive_timeout;
	guint max_inactivity;
	guint inactivity_timer;
	guint conn_close_timeout;

	PurpleJabberBOSHConnection *bosh;

	SoupSession *http_conns;

	/* keep a hash table of JingleSessions */
	GHashTable *sessions;
};

typedef gboolean (JabberFeatureEnabled)(JabberStream *js, const gchar *namespace);

typedef struct
{
	gchar *namespace;
	JabberFeatureEnabled *is_enabled;
} JabberFeature;

typedef struct
{
	gchar *category;
	gchar *type;
	gchar *name;
	gchar *lang;
} JabberIdentity;

typedef struct {
	char *jid;
	char *host;
	guint16 port;
	char *zeroconf;
} JabberBytestreamsStreamhost;

/* what kind of additional features as returned from disco#info are supported? */
extern GList *jabber_features;
/* A sorted list of identities advertised.  Use jabber_add_identity to add
 * so it remains sorted.
 */
extern GList *jabber_identities;

/**
 * Returns the GType for the JabberProtocol object.
 */

void jabber_stream_features_parse(JabberStream *js, PurpleXmlNode *packet);
void jabber_process_packet(JabberStream *js, PurpleXmlNode **packet);
void jabber_send(JabberStream *js, PurpleXmlNode *data);

void jabber_stream_set_state(JabberStream *js, JabberStreamState state);

char *jabber_get_next_id(JabberStream *js);

/** Parse an error into a human-readable string and optionally a disconnect
 *  reason.
 *  @param js     the stream on which the error occurred.
 *  @param packet the error packet
 *  @param reason where to store the disconnection reason, or @c NULL if you
 *                don't care or you don't intend to close the connection.
 */
char *jabber_parse_error(JabberStream *js, PurpleXmlNode *packet, PurpleConnectionError *reason);

/**
 * Add a feature to the list of features advertised via disco#info.  If you
 * call this while accounts are connected, Bad Things(TM) will happen because
 * the Entity Caps hash will be out-of-date (which should be fixed :/)
 *
 * @param namespace The namespace of the feature
 * @param cb        A callback determining whether or not this feature
 *                  will advertised; may be NULL.
 */
void jabber_add_feature(const gchar *namespace, JabberFeatureEnabled cb);

JabberIdentity *jabber_identity_new(const gchar *category, const gchar *type, const gchar *lang, const gchar *name);
void jabber_identity_free(JabberIdentity *id);

/**
 * GCompareFunc for JabberIdentity structs.
 */
gint jabber_identity_compare(gconstpointer a, gconstpointer b);

void jabber_bytestreams_streamhost_free(JabberBytestreamsStreamhost *sh);

/**
 * Returns true if this connection is over a secure (SSL) stream. Use this
 * instead of checking js->gsc because BOSH stores its PurpleSslConnection
 * members in its own data structure.
 */
gboolean jabber_stream_is_ssl(JabberStream *js);

/**
 * Restart the "we haven't sent anything in a while and should send
 * something or the server will kick us off" timer (obviously
 * called when sending something.  It's exposed for BOSH.)
 */
void jabber_stream_restart_inactivity_timer(JabberStream *js);

/** Protocol functions */
void jabber_blocklist_parse_push(JabberStream *js, const char *from,
                                 JabberIqType type, const char *id,
                                 PurpleXmlNode *child);
void jabber_request_block_list(JabberStream *js);

#endif /* PURPLE_JABBER_JABBER_H */
