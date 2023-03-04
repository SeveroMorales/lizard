/* pidgin
 *
 * Pidgin is the legal property of its developers, whose names are too numerous
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib/gi18n-lib.h>

#include <gtk/gtk.h>

#include <purple.h>

#include "gtkmedia.h"
#include "gtkutils.h"
#include "pidgincore.h"
#include "pidginkeypad.h"

#define PIDGIN_TYPE_MEDIA            (pidgin_media_get_type())
#define PIDGIN_MEDIA(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), PIDGIN_TYPE_MEDIA, PidginMedia))
#define PIDGIN_MEDIA_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), PIDGIN_TYPE_MEDIA, PidginMediaClass))
#define PIDGIN_IS_MEDIA(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), PIDGIN_TYPE_MEDIA))
#define PIDGIN_IS_MEDIA_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), PIDGIN_TYPE_MEDIA))
#define PIDGIN_MEDIA_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), PIDGIN_TYPE_MEDIA, PidginMediaClass))

typedef struct _PidginMedia PidginMedia;
typedef struct _PidginMediaClass PidginMediaClass;
typedef struct _PidginMediaPrivate PidginMediaPrivate;

typedef enum
{
	/* Waiting for response */
	PIDGIN_MEDIA_WAITING = 1,
	/* Got request */
	PIDGIN_MEDIA_REQUESTED,
	/* Accepted call */
	PIDGIN_MEDIA_ACCEPTED,
	/* Rejected call */
	PIDGIN_MEDIA_REJECTED,
} PidginMediaState;

struct _PidginMediaClass
{
	GtkApplicationWindowClass parent_class;
};

struct _PidginMedia
{
	GtkApplicationWindow parent;
	PidginMediaPrivate *priv;
};

struct _PidginMediaPrivate
{
	PurpleMedia *media;
	gchar *screenname;
	gulong level_handler_id;

	GtkBuilder *ui;
	GtkWidget *menubar;
	GtkWidget *statusbar;

	GtkWidget *hold;
	GtkWidget *mute;
	GtkWidget *pause;

	GtkWidget *send_progress;
	GHashTable *recv_progressbars;

	PidginMediaState state;

	GtkWidget *display;
	GtkWidget *send_widget;
	GtkWidget *recv_widget;
	GtkWidget *button_widget;
	GtkWidget *local_video;
	GHashTable *remote_videos;

	guint timeout_id;
	PurpleMediaSessionType request_type;
};

static GType pidgin_media_get_type(void);

G_DEFINE_TYPE_WITH_PRIVATE(PidginMedia, pidgin_media,
		GTK_TYPE_APPLICATION_WINDOW);

static void pidgin_media_dispose (GObject *object);
static void pidgin_media_finalize (GObject *object);
static void pidgin_media_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
static void pidgin_media_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void pidgin_media_set_state(PidginMedia *gtkmedia, PidginMediaState state);

enum {
	PROP_0,
	PROP_MEDIA,
	PROP_SCREENNAME
};

static gboolean
pidgin_media_close_request_cb(G_GNUC_UNUSED GtkWindow *window, gpointer data) {
	PidginMedia *media = data;

	if(media->priv->media) {
		g_action_group_activate_action(G_ACTION_GROUP(media),
				"Hangup", NULL);
	}

	return FALSE;
}

static void
pidgin_media_class_init (PidginMediaClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	gobject_class->dispose = pidgin_media_dispose;
	gobject_class->finalize = pidgin_media_finalize;
	gobject_class->set_property = pidgin_media_set_property;
	gobject_class->get_property = pidgin_media_get_property;

	g_object_class_install_property(gobject_class, PROP_MEDIA,
			g_param_spec_object("media",
			"PurpleMedia",
			"The PurpleMedia associated with this media.",
			PURPLE_TYPE_MEDIA,
			G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
	g_object_class_install_property(gobject_class, PROP_SCREENNAME,
			g_param_spec_string("screenname",
			"Screenname",
			"The screenname of the user this session is with.",
			NULL,
			G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	gtk_widget_class_set_template_from_resource(
		widget_class,
		"/im/pidgin/Pidgin3/Media/window.ui"
	);

	gtk_widget_class_bind_template_child_private(widget_class, PidginMedia,
	                                             display);
	gtk_widget_class_bind_template_child_private(widget_class, PidginMedia,
	                                             statusbar);

	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_media_close_request_cb);

}

static void
pidgin_media_hangup_activate_cb(G_GNUC_UNUSED GSimpleAction *action,
                                G_GNUC_UNUSED GVariant *parameter,
                                gpointer user_data)
{
	PidginMedia *media = PIDGIN_MEDIA(user_data);

	purple_media_stream_info(media->priv->media,
			PURPLE_MEDIA_INFO_HANGUP, NULL, NULL, TRUE);
}

static void
pidgin_media_hold_change_state_cb(GSimpleAction *action, GVariant *value,
		gpointer user_data)
{
	PidginMedia *media = PIDGIN_MEDIA(user_data);

	purple_media_stream_info(media->priv->media,
			g_variant_get_boolean(value) ?
			PURPLE_MEDIA_INFO_HOLD : PURPLE_MEDIA_INFO_UNHOLD,
			NULL, NULL, TRUE);

	g_simple_action_set_state(action, value);
}

static void
pidgin_media_mute_change_state_cb(GSimpleAction *action, GVariant *value,
		gpointer user_data)
{
	PidginMedia *media = PIDGIN_MEDIA(user_data);

	purple_media_stream_info(media->priv->media,
			g_variant_get_boolean(value) ?
			PURPLE_MEDIA_INFO_MUTE : PURPLE_MEDIA_INFO_UNMUTE,
			NULL, NULL, TRUE);

	g_simple_action_set_state(action, value);
}

static void
pidgin_media_pause_change_state_cb(GSimpleAction *action, GVariant *value,
		gpointer user_data)
{
	PidginMedia *media = PIDGIN_MEDIA(user_data);

	purple_media_stream_info(media->priv->media,
			g_variant_get_boolean(value) ?
			PURPLE_MEDIA_INFO_PAUSE : PURPLE_MEDIA_INFO_UNPAUSE,
			NULL, NULL, TRUE);

	g_simple_action_set_state(action, value);
}

static const GActionEntry media_action_entries[] = {
	{
		.name = "Hangup",
		.activate = pidgin_media_hangup_activate_cb,
	},
	{
		.name = "Hold",
		.state = "false",
		.change_state = pidgin_media_hold_change_state_cb,
	},
	{
		.name = "Mute",
		.state = "false",
		.change_state = pidgin_media_mute_change_state_cb,
	},
	{
		.name = "Pause",
		.state =  "false",
		.change_state = pidgin_media_pause_change_state_cb,
	},
};

static void
pidgin_media_init (PidginMedia *media)
{
	media->priv = pidgin_media_get_instance_private(media);

	gtk_widget_init_template(GTK_WIDGET(media));

	g_action_map_add_action_entries(G_ACTION_MAP(media),
			media_action_entries,
			G_N_ELEMENTS(media_action_entries), media);

	gtk_statusbar_push(GTK_STATUSBAR(media->priv->statusbar),
			0, _("Calling..."));

	media->priv->recv_progressbars =
			g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
	media->priv->remote_videos =
			g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
}

static gchar *
create_key(const gchar *session_id, const gchar *participant)
{
	return g_strdup_printf("%s_%s", session_id, participant);
}

static void
pidgin_media_insert_widget(PidginMedia *gtkmedia, GtkWidget *widget,
		const gchar *session_id, const gchar *participant)
{
	gchar *key = create_key(session_id, participant);
	PurpleMediaSessionType type =
			purple_media_get_session_type(gtkmedia->priv->media, session_id);

	if (type & PURPLE_MEDIA_AUDIO) {
		g_hash_table_insert(gtkmedia->priv->recv_progressbars, key, widget);
	} else if (type & PURPLE_MEDIA_VIDEO) {
		g_hash_table_insert(gtkmedia->priv->remote_videos, key, widget);
	}
}

static GtkWidget *
pidgin_media_get_widget(PidginMedia *gtkmedia,
		const gchar *session_id, const gchar *participant)
{
	GtkWidget *widget = NULL;
	gchar *key = create_key(session_id, participant);
	PurpleMediaSessionType type =
			purple_media_get_session_type(gtkmedia->priv->media, session_id);

	if (type & PURPLE_MEDIA_AUDIO) {
		widget = g_hash_table_lookup(gtkmedia->priv->recv_progressbars, key);
	} else if (type & PURPLE_MEDIA_VIDEO) {
		widget = g_hash_table_lookup(gtkmedia->priv->remote_videos, key);
	}

	g_free(key);
	return widget;
}

static void
pidgin_media_remove_widget(PidginMedia *gtkmedia,
		const gchar *session_id, const gchar *participant)
{
	GtkWidget *widget = NULL;
	PurpleMediaSessionType type = PURPLE_MEDIA_NONE;
	gchar *key = NULL;

	widget = pidgin_media_get_widget(gtkmedia, session_id, participant);
	if (widget == NULL) {
		return;
	}

	type = purple_media_get_session_type(gtkmedia->priv->media, session_id);
	key = create_key(session_id, participant);

	if (type & PURPLE_MEDIA_AUDIO) {
		g_hash_table_remove(gtkmedia->priv->recv_progressbars, key);

		if (g_hash_table_size(gtkmedia->priv->recv_progressbars) == 0 &&
				gtkmedia->priv->send_progress) {

			g_clear_pointer(&gtkmedia->priv->send_progress, gtk_widget_unparent);
			g_clear_pointer(&gtkmedia->priv->mute, gtk_widget_unparent);
		}
	} else if (type & PURPLE_MEDIA_VIDEO) {
		g_hash_table_remove(gtkmedia->priv->remote_videos, key);

		if (g_hash_table_size(gtkmedia->priv->remote_videos) == 0 &&
				gtkmedia->priv->local_video) {

			g_clear_pointer(&gtkmedia->priv->local_video, gtk_widget_unparent);
			g_clear_pointer(&gtkmedia->priv->pause, gtk_widget_unparent);
		}
	}

	g_free(key);

	gtk_widget_unparent(widget);
}

static void
level_message_cb(G_GNUC_UNUSED PurpleMedia *media, char *session_id,
                 char *participant, double level, PidginMedia *gtkmedia)
{
	GtkWidget *progress = NULL;

	if (participant == NULL) {
		progress = gtkmedia->priv->send_progress;
	} else {
		progress = pidgin_media_get_widget(gtkmedia, session_id, participant);
	}

	if (progress) {
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress), level);
	}
}


static void
pidgin_media_disconnect_levels(PurpleMedia *media, PidginMedia *gtkmedia)
{
	PurpleMediaManager *manager = NULL;
	GstElement *element = NULL;
	GstBus *bus = NULL;

	manager = purple_media_get_manager(media);
	element = purple_media_manager_get_pipeline(manager);
	bus = gst_pipeline_get_bus(GST_PIPELINE(element));
	g_signal_handlers_disconnect_by_func(bus, level_message_cb, gtkmedia);
}

static void
pidgin_media_dispose(GObject *media)
{
	PidginMedia *gtkmedia = PIDGIN_MEDIA(media);
	purple_debug_info("gtkmedia", "pidgin_media_dispose\n");

	if (gtkmedia->priv->media) {
		purple_request_close_with_handle(gtkmedia);
		purple_media_remove_output_windows(gtkmedia->priv->media);
		pidgin_media_disconnect_levels(gtkmedia->priv->media, gtkmedia);
		g_clear_object(&gtkmedia->priv->media);
	}

	g_clear_object(&gtkmedia->priv->ui);

	if (gtkmedia->priv->timeout_id != 0) {
		g_source_remove(gtkmedia->priv->timeout_id);
	}

	g_clear_pointer(&gtkmedia->priv->recv_progressbars, g_hash_table_destroy);
	g_clear_pointer(&gtkmedia->priv->remote_videos, g_hash_table_destroy);
	g_clear_pointer(&gtkmedia->priv->screenname, g_free);

	G_OBJECT_CLASS(pidgin_media_parent_class)->dispose(media);
}

static void
pidgin_media_finalize(GObject *media)
{
	/* PidginMedia *gtkmedia = PIDGIN_MEDIA(media); */
	purple_debug_info("gtkmedia", "pidgin_media_finalize\n");

	G_OBJECT_CLASS(pidgin_media_parent_class)->finalize(media);
}

static void
pidgin_media_emit_message(PidginMedia *gtkmedia, const char *msg)
{
	PurpleConversation *conv;
	PurpleConversationManager *manager;
	PurpleAccount *account;

	account = purple_media_get_account(gtkmedia->priv->media);
	manager = purple_conversation_manager_get_default();
	conv = purple_conversation_manager_find(manager, account,
	                                        gtkmedia->priv->screenname);

	if(PURPLE_IS_CONVERSATION(conv)) {
		purple_conversation_write_system_message(conv, msg, 0);
	}

	g_object_unref(account);
}

static void
pidgin_media_error_cb(G_GNUC_UNUSED PidginMedia *media, const char *error,
                      PidginMedia *gtkmedia)
{
	PurpleConversation *conv;
	PurpleConversationManager *manager;
	PurpleAccount *account;

	account = purple_media_get_account(gtkmedia->priv->media);
	manager = purple_conversation_manager_get_default();
	conv = purple_conversation_manager_find(manager, account,
	                                        gtkmedia->priv->screenname);

	if(PURPLE_IS_CONVERSATION(conv)) {
		purple_conversation_write_system_message(conv, error,
		                                         PURPLE_MESSAGE_ERROR);
	} else {
		purple_notify_error(NULL, NULL, _("Media error"), error,
		                    purple_request_cpar_from_conversation(conv));
	}

	gtk_statusbar_push(GTK_STATUSBAR(gtkmedia->priv->statusbar),
	                   0, error);

	g_object_unref(account);
}

static void
pidgin_media_accept_cb(PurpleMedia *media, G_GNUC_UNUSED int index)
{
	purple_media_stream_info(media, PURPLE_MEDIA_INFO_ACCEPT,
			NULL, NULL, TRUE);
}

static void
pidgin_media_reject_cb(PurpleMedia *media, G_GNUC_UNUSED int index)
{
	GList *iter = purple_media_get_session_ids(media);
	for (; iter; iter = g_list_delete_link(iter, iter)) {
		const gchar *sessionid = iter->data;
		if (!purple_media_accepted(media, sessionid, NULL)) {
			purple_media_stream_info(media, PURPLE_MEDIA_INFO_REJECT,
					sessionid, NULL, TRUE);
		}
	}
}

static gboolean
pidgin_request_timeout_cb(PidginMedia *gtkmedia)
{
	PurpleAccount *account;
	PurpleBuddy *buddy;
	const gchar *alias;
	PurpleMediaSessionType type;
	gchar *message = NULL;

	account = purple_media_get_account(gtkmedia->priv->media);
	buddy = purple_blist_find_buddy(account, gtkmedia->priv->screenname);
	if(buddy != NULL) {
		alias = purple_buddy_get_contact_alias(buddy);
	} else {
		alias = gtkmedia->priv->screenname;
	}
	type = gtkmedia->priv->request_type;
	gtkmedia->priv->timeout_id = 0;

	if (type & PURPLE_MEDIA_AUDIO && type & PURPLE_MEDIA_VIDEO) {
		message = g_strdup_printf(_("%s wishes to start an audio/video session with you."),
				alias);
	} else if (type & PURPLE_MEDIA_AUDIO) {
		message = g_strdup_printf(_("%s wishes to start an audio session with you."),
				alias);
	} else if (type & PURPLE_MEDIA_VIDEO) {
		message = g_strdup_printf(_("%s wishes to start a video session with you."),
				alias);
	}

	gtkmedia->priv->request_type = PURPLE_MEDIA_NONE;
	if (!purple_media_accepted(gtkmedia->priv->media, NULL, NULL)) {
		purple_request_accept_cancel(gtkmedia, _("Incoming Call"),
			message, NULL, PURPLE_DEFAULT_ACTION_NONE,
			purple_request_cpar_from_account(account),
			gtkmedia->priv->media, pidgin_media_accept_cb,
			pidgin_media_reject_cb);
	}
	pidgin_media_emit_message(gtkmedia, message);
	g_free(message);
	g_object_unref(account);
	return FALSE;
}

static void
pidgin_media_input_volume_changed(G_GNUC_UNUSED GtkScaleButton *range,
                                  double value, PurpleMedia *media)
{
	double val = (double)value * 100.0;
	purple_media_set_input_volume(media, NULL, val);
}

static void
pidgin_media_output_volume_changed(G_GNUC_UNUSED GtkScaleButton *range,
                                   double value, PurpleMedia *media)
{
	double val = (double)value * 100.0;
	purple_media_set_output_volume(media, NULL, NULL, val);
}

static void
destroy_parent_widget_cb(G_GNUC_UNUSED GtkWidget *widget, GtkWidget *parent)
{
	g_return_if_fail(GTK_IS_WIDGET(parent));

	gtk_widget_unparent(parent);
}

static GtkWidget *
pidgin_media_add_audio_widget(PidginMedia *gtkmedia,
		PurpleMediaSessionType type, const gchar *sid)
{
	GtkWidget *volume_widget, *progress_parent, *volume, *progress;
	double value;

	static const gchar * input_volume_icons[] = {
		"microphone-sensitivity-muted-symbolic",
		"microphone-sensitivity-high-symbolic",
		"microphone-sensitivity-low-symbolic",
		"microphone-sensitivity-medium-symbolic",
		NULL
	};

	if (type & PURPLE_MEDIA_SEND_AUDIO) {
		value = purple_prefs_get_int("/purple/media/audio/volume/input");
	} else if (type & PURPLE_MEDIA_RECV_AUDIO) {
		value = purple_prefs_get_int("/purple/media/audio/volume/output");
	} else {
		g_return_val_if_reached(NULL);
	}

	/* Setup widget structure */
	volume_widget = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
	progress_parent = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_widget_set_hexpand(progress_parent, TRUE);
	gtk_box_append(GTK_BOX(volume_widget), progress_parent);

	/* Volume button */
	volume = gtk_volume_button_new();
	gtk_scale_button_set_value(GTK_SCALE_BUTTON(volume), value/100.0);
	gtk_box_append(GTK_BOX(volume_widget), volume);

	/* Volume level indicator */
	progress = gtk_progress_bar_new();
	gtk_widget_set_size_request(progress, 250, 10);
	gtk_widget_set_vexpand(progress, TRUE);
	gtk_box_append(GTK_BOX(progress_parent), progress);

	if (type & PURPLE_MEDIA_SEND_AUDIO) {
		g_signal_connect (G_OBJECT(volume), "value-changed",
				G_CALLBACK(pidgin_media_input_volume_changed),
				gtkmedia->priv->media);
		gtk_scale_button_set_icons(GTK_SCALE_BUTTON(volume),
				input_volume_icons);

		gtkmedia->priv->send_progress = progress;
	} else if (type & PURPLE_MEDIA_RECV_AUDIO) {
		g_signal_connect (G_OBJECT(volume), "value-changed",
				G_CALLBACK(pidgin_media_output_volume_changed),
				gtkmedia->priv->media);

		pidgin_media_insert_widget(gtkmedia, progress, sid, gtkmedia->priv->screenname);
	}

	g_signal_connect(G_OBJECT(progress), "destroy",
			G_CALLBACK(destroy_parent_widget_cb),
			volume_widget);

	return volume_widget;
}

static void
pidgin_media_keypad_pressed_cb(G_GNUC_UNUSED PidginKeypad *keypad, guint key,
                               gpointer data)
{
	PidginMedia *gtkmedia = data;
	gchar *sid;

	sid = g_object_get_data(G_OBJECT(gtkmedia), "session-id");

	purple_media_send_dtmf(gtkmedia->priv->media, sid, key, 25, 50);
}

static GtkWidget *
pidgin_media_add_dtmf_widget(PidginMedia *gtkmedia,
                             G_GNUC_UNUSED PurpleMediaSessionType type,
                             const char *_sid)
{
	GtkApplicationWindow *win = GTK_APPLICATION_WINDOW(gtkmedia);
	GtkWidget *keypad = NULL;

	keypad = pidgin_keypad_new();
	pidgin_keypad_set_key_capture_widget(PIDGIN_KEYPAD(keypad),
	                                     GTK_WIDGET(win));
	g_signal_connect(keypad, "pressed",
	                 G_CALLBACK(pidgin_media_keypad_pressed_cb), gtkmedia);

	g_object_set_data_full(G_OBJECT(win), "session-id",
		g_strdup(_sid), g_free);

	return keypad;
}

static void
pidgin_media_ready_cb(PurpleMedia *media, PidginMedia *gtkmedia, const gchar *sid)
{
	GtkWidget *send_widget = NULL, *recv_widget = NULL, *button_widget = NULL;
	PurpleMediaSessionType type =
			purple_media_get_session_type(media, sid);

	if (gtkmedia->priv->recv_widget == NULL
			&& type & (PURPLE_MEDIA_RECV_VIDEO |
			PURPLE_MEDIA_RECV_AUDIO)) {
		recv_widget = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
		gtk_widget_set_hexpand(recv_widget, TRUE);
		gtk_widget_set_vexpand(recv_widget, TRUE);
		gtk_box_append(GTK_BOX(gtkmedia->priv->display), recv_widget);
	} else {
		recv_widget = gtkmedia->priv->recv_widget;
	}
	if (gtkmedia->priv->send_widget == NULL
			&& type & (PURPLE_MEDIA_SEND_VIDEO |
			PURPLE_MEDIA_SEND_AUDIO)) {
		send_widget = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
		gtk_box_prepend(GTK_BOX(gtkmedia->priv->display), send_widget);

		button_widget = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
		gtk_box_append(GTK_BOX(send_widget), button_widget);

		/* Hold button */
		gtkmedia->priv->hold =
				gtk_toggle_button_new_with_mnemonic(_("_Hold"));
		gtk_box_prepend(GTK_BOX(button_widget), gtkmedia->priv->hold);
		gtk_actionable_set_action_name(
				GTK_ACTIONABLE(gtkmedia->priv->hold),
				"win.Hold");
	} else {
		send_widget = gtkmedia->priv->send_widget;
		button_widget = gtkmedia->priv->button_widget;
	}

	if (type & PURPLE_MEDIA_RECV_VIDEO) {
		PidginMediaPrivate *priv = gtkmedia->priv;
		PurpleMediaManager *manager = NULL;
		GstElement *pipeline = NULL;
		GstElement *sink = NULL;
		GtkWidget *remote_video = NULL;

		purple_media_set_output_window(priv->media, sid, priv->screenname);
		manager = purple_media_get_manager(priv->media);
		pipeline = purple_media_manager_get_pipeline(manager);
		sink = gst_bin_get_by_name(GST_BIN(pipeline), "gtkglsink");
		if (sink == NULL) {
			sink = gst_bin_get_by_name(GST_BIN(pipeline), "gtksink");
		}
		g_object_get(G_OBJECT(sink), "widget", &remote_video, NULL);
		gtk_widget_show(remote_video);
		gtk_widget_set_vexpand(remote_video, TRUE);
		gtk_box_append(GTK_BOX(recv_widget), remote_video);

		pidgin_media_insert_widget(gtkmedia, remote_video, sid, priv->screenname);
	}

	if (type & PURPLE_MEDIA_SEND_VIDEO && !gtkmedia->priv->local_video) {
		PidginMediaPrivate *priv = gtkmedia->priv;
		PurpleMediaManager *manager = NULL;
		GstElement *pipeline = NULL;
		GstElement *sink = NULL;
		GtkWidget *local_video = NULL;

		purple_media_set_output_window(priv->media, sid, NULL);
		manager = purple_media_get_manager(priv->media);
		pipeline = purple_media_manager_get_pipeline(manager);
		sink = gst_bin_get_by_name(GST_BIN(pipeline), "gtkglsink");
		if (sink == NULL) {
			sink = gst_bin_get_by_name(GST_BIN(pipeline), "gtksink");
		}
		g_object_get(G_OBJECT(sink), "widget", &local_video, NULL);
		gtk_widget_show(local_video);
		gtk_widget_set_vexpand(local_video, TRUE);
		gtk_box_append(GTK_BOX(send_widget), local_video);

		gtkmedia->priv->pause =
				gtk_toggle_button_new_with_mnemonic(_("_Pause"));
		gtk_box_prepend(GTK_BOX(button_widget), gtkmedia->priv->pause);
		gtk_actionable_set_action_name(
				GTK_ACTIONABLE(gtkmedia->priv->pause),
				"win.Pause");

		gtkmedia->priv->local_video = local_video;
	}
	if (type & PURPLE_MEDIA_RECV_AUDIO) {
		GtkWidget *audio = NULL;

		audio = pidgin_media_add_audio_widget(gtkmedia, PURPLE_MEDIA_RECV_AUDIO,
		                                      sid);
		gtk_box_prepend(GTK_BOX(recv_widget), audio);
	}

	if (type & PURPLE_MEDIA_SEND_AUDIO) {
		gtkmedia->priv->mute =
				gtk_toggle_button_new_with_mnemonic(_("_Mute"));
		gtk_box_prepend(GTK_BOX(button_widget), gtkmedia->priv->mute);
		gtk_actionable_set_action_name(
				GTK_ACTIONABLE(gtkmedia->priv->mute),
				"win.Mute");

		gtk_box_prepend(GTK_BOX(recv_widget),
				pidgin_media_add_audio_widget(gtkmedia,
				PURPLE_MEDIA_SEND_AUDIO, sid));

		gtk_box_prepend(GTK_BOX(recv_widget),
				pidgin_media_add_dtmf_widget(gtkmedia,
				PURPLE_MEDIA_SEND_AUDIO, sid));
	}

	if (type & PURPLE_MEDIA_AUDIO &&
			gtkmedia->priv->level_handler_id == 0) {
		gtkmedia->priv->level_handler_id = g_signal_connect(
				media, "level", G_CALLBACK(level_message_cb),
				gtkmedia);
	}

	if (send_widget != NULL) {
		gtkmedia->priv->send_widget = send_widget;
	}
	if (recv_widget != NULL) {
		gtkmedia->priv->recv_widget = recv_widget;
	}
	if (button_widget != NULL) {
		gtkmedia->priv->button_widget = button_widget;
	}

	if (purple_media_is_initiator(media, sid, NULL) == FALSE) {
		if (gtkmedia->priv->timeout_id != 0)
			g_source_remove(gtkmedia->priv->timeout_id);
		gtkmedia->priv->request_type |= type;
		gtkmedia->priv->timeout_id = g_timeout_add(500,
				(GSourceFunc)pidgin_request_timeout_cb,
				gtkmedia);
	}

	/* set the window icon according to the type */
	if (type & PURPLE_MEDIA_VIDEO) {
		gtk_window_set_icon_name(GTK_WINDOW(gtkmedia), "video-call");
	} else if (type & PURPLE_MEDIA_AUDIO) {
		gtk_window_set_icon_name(GTK_WINDOW(gtkmedia), "audio-call");
	}
}

static void
pidgin_media_state_changed_cb(PurpleMedia *media, PurpleMediaState state,
		gchar *sid, gchar *name, PidginMedia *gtkmedia)
{
	purple_debug_info("gtkmedia", "state: %d sid: %s name: %s\n",
			state, sid ? sid : "(null)", name ? name : "(null)");
	if (state == PURPLE_MEDIA_STATE_END) {
		if (sid != NULL && name != NULL) {
			pidgin_media_remove_widget(gtkmedia, sid, name);
		} else if (sid == NULL && name == NULL) {
			pidgin_media_emit_message(gtkmedia,
					_("The call has been terminated."));
			gtk_window_destroy(GTK_WINDOW(gtkmedia));
		}
	} else if (state == PURPLE_MEDIA_STATE_NEW &&
			sid != NULL && name != NULL) {
		pidgin_media_ready_cb(media, gtkmedia, sid);
	}
}

static void
pidgin_media_stream_info_cb(G_GNUC_UNUSED PurpleMedia *media,
                            PurpleMediaInfoType type,
                            G_GNUC_UNUSED gchar *sid,
                            G_GNUC_UNUSED gchar *name, gboolean local,
                            PidginMedia *gtkmedia)
{
	if (type == PURPLE_MEDIA_INFO_REJECT) {
		pidgin_media_emit_message(gtkmedia,
				_("You have rejected the call."));
	} else if (type == PURPLE_MEDIA_INFO_ACCEPT) {
		if (local) {
			purple_request_close_with_handle(gtkmedia);
		}
		pidgin_media_set_state(gtkmedia, PIDGIN_MEDIA_ACCEPTED);
		pidgin_media_emit_message(gtkmedia, _("Call in progress."));
		gtk_statusbar_push(GTK_STATUSBAR(gtkmedia->priv->statusbar),
				0, _("Call in progress"));
		gtk_widget_show(GTK_WIDGET(gtkmedia));
	} else if (type == PURPLE_MEDIA_INFO_MUTE && !local) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtkmedia->priv->mute), TRUE);
	} else if (type == PURPLE_MEDIA_INFO_UNMUTE && !local) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtkmedia->priv->mute), FALSE);
	}
}

static void
pidgin_media_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	PidginMedia *media = PIDGIN_MEDIA(object);

	switch (prop_id) {
		case PROP_MEDIA:
			g_set_object(&media->priv->media, g_value_get_object(value));

			if (purple_media_is_initiator(media->priv->media, NULL, NULL)) {
				pidgin_media_set_state(media, PIDGIN_MEDIA_WAITING);
			} else {
				pidgin_media_set_state(media, PIDGIN_MEDIA_REQUESTED);
			}

			g_signal_connect(G_OBJECT(media->priv->media), "error",
				G_CALLBACK(pidgin_media_error_cb), media);
			g_signal_connect(G_OBJECT(media->priv->media), "state-changed",
				G_CALLBACK(pidgin_media_state_changed_cb), media);
			g_signal_connect(G_OBJECT(media->priv->media), "stream-info",
				G_CALLBACK(pidgin_media_stream_info_cb), media);
			break;
		case PROP_SCREENNAME:
			g_free(media->priv->screenname);
			media->priv->screenname = g_value_dup_string(value);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
pidgin_media_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	PidginMedia *media = PIDGIN_MEDIA(object);

	switch (prop_id) {
		case PROP_MEDIA:
			g_value_set_object(value, media->priv->media);
			break;
		case PROP_SCREENNAME:
			g_value_set_string(value, media->priv->screenname);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static GtkWidget *
pidgin_media_new(PurpleMedia *media, const gchar *screenname)
{
	PidginMedia *gtkmedia = g_object_new(pidgin_media_get_type(),
					     "media", media,
					     "screenname", screenname, NULL);
	return GTK_WIDGET(gtkmedia);
}

static void
pidgin_media_set_state(PidginMedia *gtkmedia, PidginMediaState state)
{
	gtkmedia->priv->state = state;
}

static gboolean
pidgin_media_new_cb(G_GNUC_UNUSED PurpleMediaManager *manager,
                    PurpleMedia *media, PurpleAccount *account,
                    char *screenname, G_GNUC_UNUSED gpointer data)
{
	PidginMedia *gtkmedia = NULL;
	PurpleBuddy *buddy = NULL;
	const gchar *alias = NULL;

	gtkmedia = PIDGIN_MEDIA(pidgin_media_new(media, screenname));
	buddy = purple_blist_find_buddy(account, screenname);
	if(buddy != NULL) {
		alias = purple_buddy_get_contact_alias(buddy);
	} else {
		alias = screenname;
	}
	gtk_window_set_title(GTK_WINDOW(gtkmedia), alias);

	gtk_widget_set_visible(GTK_WIDGET(gtkmedia),
	                       purple_media_is_initiator(media, NULL, NULL));

	return TRUE;
}

void
pidgin_medias_init(void)
{
	PurpleMediaManager *manager = purple_media_manager_get();
	PurpleMediaElementInfo *video_src = NULL;
	PurpleMediaElementInfo *video_sink = NULL;
	PurpleMediaElementInfo *audio_src = NULL;
	PurpleMediaElementInfo *audio_sink = NULL;
	const char *pref;

	pref = purple_prefs_get_string(
			PIDGIN_PREFS_ROOT "/vvconfig/video/src/device");
	if (pref) {
		video_src = purple_media_manager_get_element_info(manager, pref);
	}
	if (!video_src) {
		pref = "autovideosrc";
		purple_prefs_set_string(
			PIDGIN_PREFS_ROOT "/vvconfig/video/src/device", pref);
		video_src = purple_media_manager_get_element_info(manager,
				pref);
	}

	pref = purple_prefs_get_string(
			PIDGIN_PREFS_ROOT "/vvconfig/video/sink/device");
	if (pref) {
		video_sink = purple_media_manager_get_element_info(manager, pref);
	}
	if (!video_sink) {
		pref = "autovideosink";
		purple_prefs_set_string(
			PIDGIN_PREFS_ROOT "/vvconfig/video/sink/device", pref);
		video_sink = purple_media_manager_get_element_info(manager,
				pref);
	}

	pref = purple_prefs_get_string(
			PIDGIN_PREFS_ROOT "/vvconfig/audio/src/device");
	if (pref) {
		audio_src = purple_media_manager_get_element_info(manager, pref);
	}
	if (!audio_src) {
		pref = "autoaudiosrc";
		purple_prefs_set_string(
			PIDGIN_PREFS_ROOT "/vvconfig/audio/src/device", pref);
		audio_src = purple_media_manager_get_element_info(manager,
				pref);
	}

	pref = purple_prefs_get_string(
			PIDGIN_PREFS_ROOT "/vvconfig/audio/sink/device");
	if (pref) {
		audio_sink = purple_media_manager_get_element_info(manager, pref);
	}
	if (!audio_sink) {
		pref = "autoaudiosink";
		purple_prefs_set_string(
			PIDGIN_PREFS_ROOT "/vvconfig/audio/sink/device", pref);
		audio_sink = purple_media_manager_get_element_info(manager,
				pref);
	}

	g_signal_connect(G_OBJECT(manager), "init-media",
			 G_CALLBACK(pidgin_media_new_cb), NULL);

	purple_media_manager_set_ui_caps(manager,
			PURPLE_MEDIA_CAPS_AUDIO |
			PURPLE_MEDIA_CAPS_AUDIO_SINGLE_DIRECTION |
			PURPLE_MEDIA_CAPS_VIDEO |
			PURPLE_MEDIA_CAPS_VIDEO_SINGLE_DIRECTION |
			PURPLE_MEDIA_CAPS_AUDIO_VIDEO);

	purple_debug_info("gtkmedia", "Registering media element types\n");
	purple_media_manager_set_active_element(manager, video_src);
	purple_media_manager_set_active_element(manager, video_sink);
	purple_media_manager_set_active_element(manager, audio_src);
	purple_media_manager_set_active_element(manager, audio_sink);
}
