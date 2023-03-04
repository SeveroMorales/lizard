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

#ifndef PURPLE_MEDIA_ENUM_TYPES_H
#define PURPLE_MEDIA_ENUM_TYPES_H

#include <glib-object.h>

G_BEGIN_DECLS

/**
 * PURPLE_MEDIA_TYPE_CANDIDATE_TYPE:
 *
 * The standard _get_type macro for #PurpleMediaCandidateType.
 */
#define PURPLE_MEDIA_TYPE_CANDIDATE_TYPE (purple_media_candidate_type_get_type())

/**
 * PURPLE_MEDIA_TYPE_CAPS:
 *
 * The standard _get_type macro for #PurpleMediaCaps.
 */
#define PURPLE_MEDIA_TYPE_CAPS (purple_media_caps_get_type())

/**
 * PURPLE_MEDIA_TYPE_INFO_TYPE:
 *
 * The standard _get_type macro for #PurpleMediaInfoType.
 */
#define PURPLE_MEDIA_TYPE_INFO_TYPE (purple_media_info_type_get_type())

/**
 * PURPLE_MEDIA_TYPE_NETWORK_PROTOCOL:
 *
 * The standard _get_type macro for #PurpleMediaNetworkProtocol.
 */
#define PURPLE_MEDIA_TYPE_NETWORK_PROTOCOL (purple_media_network_protocol_get_type())

/**
 * PURPLE_MEDIA_TYPE_SESSION_TYPE:
 *
 * The standard _get_type macro for #PurpleMediaSessionType.
 */
#define PURPLE_MEDIA_TYPE_SESSION_TYPE (purple_media_session_type_get_type())

/**
 * PURPLE_MEDIA_TYPE_STATE:
 *
 * The standard _get_type macro for #PurpleMediaState.
 */
#define PURPLE_MEDIA_TYPE_STATE (purple_media_state_get_type())

/**
 * PurpleMediaCandidateType:
 * @PURPLE_MEDIA_CANDIDATE_TYPE_HOST: A host candidate (local).
 * @PURPLE_MEDIA_CANDIDATE_TYPE_SRFLX: A server reflexive candidate.
 * @PURPLE_MEDIA_CANDIDATE_TYPE_PRFLX: A peer reflexive candidate.
 * @PURPLE_MEDIA_CANDIDATE_TYPE_RELAY: A relay candidate.
 * @PURPLE_MEDIA_CANDIDATE_TYPE_MULTICAST: A multicast address.
 *
 * Media candidate types
 */
typedef enum {
	PURPLE_MEDIA_CANDIDATE_TYPE_HOST,
	PURPLE_MEDIA_CANDIDATE_TYPE_SRFLX,
	PURPLE_MEDIA_CANDIDATE_TYPE_PRFLX,
	PURPLE_MEDIA_CANDIDATE_TYPE_RELAY,
	PURPLE_MEDIA_CANDIDATE_TYPE_MULTICAST
} PurpleMediaCandidateType;

/**
 * PurpleMediaCaps:
 * @PURPLE_MEDIA_CAPS_NONE: No capabilities.
 * @PURPLE_MEDIA_CAPS_AUDIO: Bi-directional audio.
 * @PURPLE_MEDIA_CAPS_AUDIO_SINGLE_DIRECTION: Single direction audio.
 * @PURPLE_MEDIA_CAPS_VIDEO: Bi-directional video.
 * @PURPLE_MEDIA_CAPS_VIDEO_SINGLE_DIRECTION: Single direction video.
 * @PURPLE_MEDIA_CAPS_AUDIO_VIDEO: Bi-directional audio and video.
 * @PURPLE_MEDIA_CAPS_MODIFY_SESSION: Modifiable session.
 * @PURPLE_MEDIA_CAPS_CHANGE_DIRECTION: Initiator and destination can be
 *                                      switched.
 *
 * Media caps
 */
typedef enum {
	PURPLE_MEDIA_CAPS_NONE = 0,
	PURPLE_MEDIA_CAPS_AUDIO = 1,
	PURPLE_MEDIA_CAPS_AUDIO_SINGLE_DIRECTION = 1 << 1,
	PURPLE_MEDIA_CAPS_VIDEO = 1 << 2,
	PURPLE_MEDIA_CAPS_VIDEO_SINGLE_DIRECTION = 1 << 3,
	PURPLE_MEDIA_CAPS_AUDIO_VIDEO = 1 << 4,
	PURPLE_MEDIA_CAPS_MODIFY_SESSION = 1 << 5,
	PURPLE_MEDIA_CAPS_CHANGE_DIRECTION = 1 << 6
} PurpleMediaCaps;

/**
 * PurpleMediaComponentType:
 * @PURPLE_MEDIA_COMPONENT_NONE: Use this when specifying a component is
 *                               innapropriate.
 * @PURPLE_MEDIA_COMPONENT_RTP: This component is for RTP data.
 * @PURPLE_MEDIA_COMPONENT_RTCP: This component is for RTCP control.
 *
 * Media component types
 */
typedef enum {
	PURPLE_MEDIA_COMPONENT_NONE = 0,
	PURPLE_MEDIA_COMPONENT_RTP = 1,
	PURPLE_MEDIA_COMPONENT_RTCP = 2
} PurpleMediaComponentType;

/**
 * PurpleMediaInfoType:
 * @PURPLE_MEDIA_INFO_HANGUP: Terminate the media.
 * @PURPLE_MEDIA_INFO_ACCEPT: Create/join the media.
 * @PURPLE_MEDIA_INFO_REJECT: Terminate the media, possibly noting that it was
 *                            rejected.
 * @PURPLE_MEDIA_INFO_MUTE: Mute the media.
 * @PURPLE_MEDIA_INFO_UNMUTE: Unmute the media.
 * @PURPLE_MEDIA_INFO_PAUSE: Pause the media.
 * @PURPLE_MEDIA_INFO_UNPAUSE: Unpause the media.
 * @PURPLE_MEDIA_INFO_HOLD: Put the media on hold.
 * @PURPLE_MEDIA_INFO_UNHOLD: Remove the media from hold.
 *
 * Media info types
 */
typedef enum {
	PURPLE_MEDIA_INFO_HANGUP = 0,
	PURPLE_MEDIA_INFO_ACCEPT,
	PURPLE_MEDIA_INFO_REJECT,
	PURPLE_MEDIA_INFO_MUTE,
	PURPLE_MEDIA_INFO_UNMUTE,
	PURPLE_MEDIA_INFO_PAUSE,
	PURPLE_MEDIA_INFO_UNPAUSE,
	PURPLE_MEDIA_INFO_HOLD,
	PURPLE_MEDIA_INFO_UNHOLD
} PurpleMediaInfoType;

/**
 * PurpleMediaNetworkProtocol:
 * @PURPLE_MEDIA_NETWORK_PROTOCOL_UDP: A UDP based protocol.
 * @PURPLE_MEDIA_NETWORK_PROTOCOL_TCP_PASSIVE: A TCP based protocol, will
 *                                             listen for incoming connections.
 * @PURPLE_MEDIA_NETWORK_PROTOCOL_TCP_ACTIVE: A TCP based protocol, will
 *                                            attempt to open an outbound
 *                                            connection.
 * @PURPLE_MEDIA_NETWORK_PROTOCOL_TCP_SO: A TCP based protocol, will listen for
 *                                        incoming connections and attempt an
 *                                        outbound connection at the same time
 *                                        as the peer (Simultanuous-Open).
 *
 * Media network protocols
 */
typedef enum {
	PURPLE_MEDIA_NETWORK_PROTOCOL_UDP,
	PURPLE_MEDIA_NETWORK_PROTOCOL_TCP_PASSIVE,
	PURPLE_MEDIA_NETWORK_PROTOCOL_TCP_ACTIVE,
	PURPLE_MEDIA_NETWORK_PROTOCOL_TCP_SO,
} PurpleMediaNetworkProtocol;

/**
 * PurpleMediaSessionType:
 * @PURPLE_MEDIA_NONE: Used for errors.
 * @PURPLE_MEDIA_RECV_AUDIO: The session has incoming audio.
 * @PURPLE_MEDIA_SEND_AUDIO: The session has outgoing audio.
 * @PURPLE_MEDIA_RECV_VIDEO: The session has incoming video.
 * @PURPLE_MEDIA_SEND_VIDEO: The session has outgoing video.
 * @PURPLE_MEDIA_RECV_APPLICATION: The session has incoming application
 *                                 specific data.
 * @PURPLE_MEDIA_SEND_APPLICATION: The session has outgoing application
 *                                 specific data.
 * @PURPLE_MEDIA_AUDIO: An alias for #PURPLE_MEDIA_RECV_AUDIO and
 *                      #PURPLE_MEDIA_SEND_AUDIO.
 * @PURPLE_MEDIA_VIDEO: An alias for #PURPLE_MEDIA_RECV_VIDEO and
 *                      #PURPLE_MEDIA_SEND_VIDEO.
 * @PURPLE_MEDIA_APPLICATION: An alias for #PURPLE_MEDIA_RECV_APPLICATION
 *                            and #PURPLE_MEDIA_SEND_APPLICATION.
 *
 * Media session types
 */
typedef enum {
	PURPLE_MEDIA_NONE	= 0,
	PURPLE_MEDIA_RECV_AUDIO = 1 << 0,
	PURPLE_MEDIA_SEND_AUDIO = 1 << 1,
	PURPLE_MEDIA_RECV_VIDEO = 1 << 2,
	PURPLE_MEDIA_SEND_VIDEO = 1 << 3,
	PURPLE_MEDIA_RECV_APPLICATION = 1 << 4,
	PURPLE_MEDIA_SEND_APPLICATION = 1 << 5,
	PURPLE_MEDIA_AUDIO = PURPLE_MEDIA_RECV_AUDIO | PURPLE_MEDIA_SEND_AUDIO,
	PURPLE_MEDIA_VIDEO = PURPLE_MEDIA_RECV_VIDEO | PURPLE_MEDIA_SEND_VIDEO,
	PURPLE_MEDIA_APPLICATION = PURPLE_MEDIA_RECV_APPLICATION |
                                   PURPLE_MEDIA_SEND_APPLICATION
} PurpleMediaSessionType;

/**
 * PurpleMediaState:
 * @PURPLE_MEDIA_STATE_NEW: The media has not yet connected.
 * @PURPLE_MEDIA_STATE_CONNECTED: The media is connected.
 * @PURPLE_MEDIA_STATE_END: The media has previously connected but has since
 *                          disconnected.
 *
 * Media state-changed types
 */
typedef enum {
	PURPLE_MEDIA_STATE_NEW = 0,
	PURPLE_MEDIA_STATE_CONNECTED,
	PURPLE_MEDIA_STATE_END
} PurpleMediaState;

/**
 * purple_media_candidate_type_get_type:
 *
 * Gets the media candidate type's GType
 *
 * Returns: The media candidate type's GType.
 */
GType purple_media_candidate_type_get_type(void);

/**
 * purple_media_caps_get_type:
 *
 * Gets the type of the media caps flags
 *
 * Returns: The media caps flags' GType
 */
GType purple_media_caps_get_type(void);

/**
 * purple_media_info_type_get_type:
 *
 * Gets the type of the info type enum
 *
 * Returns: The info type enum's GType
 */
GType purple_media_info_type_get_type(void);

/**
 * purple_media_network_protocol_get_type:
 *
 * Gets the media network protocol's GType
 *
 * Returns: The media network protocol's GType.
 */
GType purple_media_network_protocol_get_type(void);

/**
 * purple_media_session_type_get_type:
 *
 * Gets the media session type's GType
 *
 * Returns: The media session type's GType.
 */
GType purple_media_session_type_get_type(void);

/**
 * purple_media_state_get_type:
 *
 * Gets the type of the state enum
 *
 * Returns: The state enum's GType
 */
GType purple_media_state_get_type(void);

G_END_DECLS

#endif /* PURPLE_MEDIA_ENUM_TYPES */
