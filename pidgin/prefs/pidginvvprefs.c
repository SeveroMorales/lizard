/*
 * Pidgin - Internet Messenger
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <math.h>

#include <glib/gi18n-lib.h>

#include <purple.h>

#include <adwaita.h>

#include "pidginvvprefs.h"
#include "pidgincore.h"
#include "pidginprefsinternal.h"

struct _PidginVVPrefs {
	AdwPreferencesPage parent;

	struct {
		PidginPrefCombo input;
		PidginPrefCombo output;
		GtkWidget *threshold_row;
		GtkWidget *threshold;
		GtkWidget *volume;
		GtkWidget *test;
		GtkWidget *level;
		GtkWidget *drop;
		GstElement *pipeline;
	} voice;

	struct {
		PidginPrefCombo input;
		PidginPrefCombo output;
		GtkWidget *frame;
		GtkWidget *test;
		GstElement *pipeline;
	} video;
};

G_DEFINE_TYPE(PidginVVPrefs, pidgin_vv_prefs, ADW_TYPE_PREFERENCES_PAGE)

/* Keep in sync with voice.level's GtkLevelBar::max-value in the
 * pidgin/resources/Prefs.vv.ui builder file. */
#define MAX_AUDIO_LEVEL (19.0)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
populate_vv_device_menuitems(PurpleMediaElementType type, GtkListStore *store)
{
	PurpleMediaManager *manager = NULL;
	GList *devices;

	gtk_list_store_clear(store);

	manager = purple_media_manager_get();
	devices = purple_media_manager_enumerate_elements(manager, type);
	for (; devices; devices = g_list_delete_link(devices, devices)) {
		PurpleMediaElementInfo *info = devices->data;
		GtkTreeIter iter;
		char *name, *id;

		name = purple_media_element_info_get_name(info);
		id = purple_media_element_info_get_id(info);

		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter, PIDGIN_PREF_COMBO_TEXT, name,
		                   PIDGIN_PREF_COMBO_VALUE, id, -1);

		g_free(name);
		g_free(id);
		g_object_unref(info);
	}
}

static GstElement *
create_test_element(PurpleMediaElementType type)
{
	PurpleMediaElementInfo *element_info;

	element_info = purple_media_manager_get_active_element(purple_media_manager_get(), type);

	g_return_val_if_fail(element_info, NULL);

	return purple_media_element_info_call_create(element_info,
		NULL, NULL, NULL);
}

static GstElement *
create_voice_pipeline(PidginVVPrefs *prefs)
{
	GstElement *pipeline;
	GstElement *src, *sink;
	GstElement *volume;
	GstElement *level;
	GstElement *valve;

	pipeline = gst_pipeline_new("voicetest");

	src = create_test_element(PURPLE_MEDIA_ELEMENT_AUDIO | PURPLE_MEDIA_ELEMENT_SRC);
	sink = create_test_element(PURPLE_MEDIA_ELEMENT_AUDIO | PURPLE_MEDIA_ELEMENT_SINK);
	volume = gst_element_factory_make("volume", "volume");
	level = gst_element_factory_make("level", "level");
	valve = gst_element_factory_make("valve", "valve");

	g_object_set(volume, "volume",
	             gtk_scale_button_get_value(GTK_SCALE_BUTTON(prefs->voice.volume)) / 100.0,
	             NULL);

	gst_bin_add_many(GST_BIN(pipeline), src, volume, level, valve, sink, NULL);
	gst_element_link_many(src, volume, level, valve, sink, NULL);

	purple_debug_info("gtkprefs", "create_voice_pipeline: setting pipeline "
		"state to GST_STATE_PLAYING - it may hang here on win32\n");
	gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PLAYING);
	purple_debug_info("gtkprefs", "create_voice_pipeline: state is set\n");

	return pipeline;
}

static void
on_volume_change_cb(GtkWidget *w, G_GNUC_UNUSED gdouble value, gpointer data)
{
	PidginVVPrefs *prefs = PIDGIN_VV_PREFS(data);
	GstElement *volume;

	if (!prefs->voice.pipeline) {
		return;
	}

	volume = gst_bin_get_by_name(GST_BIN(prefs->voice.pipeline), "volume");
	g_object_set(volume, "volume",
	             gtk_scale_button_get_value(GTK_SCALE_BUTTON(w)) / 100.0, NULL);
}

static gdouble
gst_msg_db_to_percent(GstMessage *msg, gchar *value_name)
{
	const GValue *list;
	const GValue *value;
	gdouble value_db;
	gdouble percent;

	list = gst_structure_get_value(gst_message_get_structure(msg), value_name);
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
	value = g_value_array_get_nth(g_value_get_boxed(list), 0);
G_GNUC_END_IGNORE_DEPRECATIONS
	value_db = g_value_get_double(value);
	percent = pow(10, value_db / 20);
	return (percent > 1.0) ? 1.0 : percent;
}

static gboolean
gst_bus_cb(G_GNUC_UNUSED GstBus *bus, GstMessage *msg, gpointer data)
{
	PidginVVPrefs *prefs = PIDGIN_VV_PREFS(data);

	if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ELEMENT &&
		gst_structure_has_name(gst_message_get_structure(msg), "level")) {

		GstElement *src = GST_ELEMENT(GST_MESSAGE_SRC(msg));
		gchar *name = gst_element_get_name(src);

		if (purple_strequal(name, "level")) {
			gdouble percent;
			gdouble threshold;
			gboolean drop;
			GstElement *valve;

			percent = gst_msg_db_to_percent(msg, "rms");
			gtk_level_bar_set_value(GTK_LEVEL_BAR(prefs->voice.level),
			                        percent * MAX_AUDIO_LEVEL);

			percent = gst_msg_db_to_percent(msg, "decay");
			threshold = gtk_range_get_value(GTK_RANGE(
			                    prefs->voice.threshold)) /
			            100.0;
			drop = percent < threshold;

			valve = gst_bin_get_by_name(GST_BIN(GST_ELEMENT_PARENT(src)), "valve");
			g_object_set(valve, "drop", drop, NULL);
			gtk_label_set_text(GTK_LABEL(prefs->voice.drop),
			                   drop ? _("DROP") : "");
		}

		g_free(name);
	}

	return TRUE;
}

static void
voice_test_destroy_cb(G_GNUC_UNUSED GtkWidget *w, gpointer data)
{
	PidginVVPrefs *prefs = PIDGIN_VV_PREFS(data);

	if (!prefs->voice.pipeline) {
		return;
	}

	gst_element_set_state(prefs->voice.pipeline, GST_STATE_NULL);
	g_clear_pointer(&prefs->voice.pipeline, gst_object_unref);
}

static void
enable_voice_test(PidginVVPrefs *prefs)
{
	GstBus *bus;

	prefs->voice.pipeline = create_voice_pipeline(prefs);
	bus = gst_pipeline_get_bus(GST_PIPELINE(prefs->voice.pipeline));
	gst_bus_add_signal_watch(bus);
	g_signal_connect(bus, "message", G_CALLBACK(gst_bus_cb), prefs);
	gst_object_unref(bus);
}

static void
toggle_voice_test_cb(GtkToggleButton *test, gpointer data)
{
	PidginVVPrefs *prefs = PIDGIN_VV_PREFS(data);

	if (gtk_toggle_button_get_active(test)) {
		enable_voice_test(prefs);

		g_signal_connect(prefs->voice.volume, "value-changed",
		                 G_CALLBACK(on_volume_change_cb), prefs);
		g_signal_connect(test, "destroy",
		                 G_CALLBACK(voice_test_destroy_cb), prefs);
	} else {
		gtk_level_bar_set_value(GTK_LEVEL_BAR(prefs->voice.level), 0.0);
		gtk_label_set_text(GTK_LABEL(prefs->voice.drop), "");
		g_object_disconnect(prefs->voice.volume,
		                    "any-signal::value-changed",
		                    G_CALLBACK(on_volume_change_cb), prefs, NULL);
		g_object_disconnect(test, "any-signal::destroy",
		                    G_CALLBACK(voice_test_destroy_cb), prefs,
		                    NULL);
		voice_test_destroy_cb(NULL, prefs);
	}
}

static void
volume_changed_cb(G_GNUC_UNUSED GtkScaleButton *button, gdouble value,
                  G_GNUC_UNUSED gpointer data)
{
	purple_prefs_set_int("/purple/media/audio/volume/input", value * 100);
}

static void
threshold_value_changed_cb(GtkScale *scale, gpointer data)
{
	PidginVVPrefs *prefs = data;
	int value;
	char *tmp;

	value = (int)gtk_range_get_value(GTK_RANGE(scale));
	tmp = g_strdup_printf(_("Silence threshold: %d%%"), value);
	adw_preferences_row_set_title(ADW_PREFERENCES_ROW(prefs->voice.threshold_row),
	                              tmp);
	g_free(tmp);

	gtk_level_bar_add_offset_value(GTK_LEVEL_BAR(prefs->voice.level),
	                               GTK_LEVEL_BAR_OFFSET_LOW,
	                               value / 100.0 * MAX_AUDIO_LEVEL);

	purple_prefs_set_int("/purple/media/audio/silence_threshold", value);
}

static void
bind_voice_test(PidginVVPrefs *prefs)
{
	char *tmp;

	gtk_scale_button_set_value(GTK_SCALE_BUTTON(prefs->voice.volume),
			purple_prefs_get_int("/purple/media/audio/volume/input") / 100.0);

	tmp = g_strdup_printf(_("Silence threshold: %d%%"),
	                      purple_prefs_get_int("/purple/media/audio/silence_threshold"));
	adw_preferences_row_set_title(ADW_PREFERENCES_ROW(prefs->voice.threshold_row),
	                              tmp);
	g_free(tmp);

	/* Move the default high levels to the end (low is set by
	 * threshold_value_changed_cb when set below.) */
	gtk_level_bar_add_offset_value(GTK_LEVEL_BAR(prefs->voice.level),
	                               GTK_LEVEL_BAR_OFFSET_HIGH,
	                               MAX_AUDIO_LEVEL);
	gtk_level_bar_add_offset_value(GTK_LEVEL_BAR(prefs->voice.level),
	                               GTK_LEVEL_BAR_OFFSET_FULL,
	                               MAX_AUDIO_LEVEL);

	gtk_range_set_value(GTK_RANGE(prefs->voice.threshold),
			purple_prefs_get_int("/purple/media/audio/silence_threshold"));
}

static GstElement *
create_video_pipeline(void)
{
	GstElement *pipeline;
	GstElement *src, *sink;
	GstElement *videoconvert;
	GstElement *videoscale;

	pipeline = gst_pipeline_new("videotest");
	src = create_test_element(PURPLE_MEDIA_ELEMENT_VIDEO | PURPLE_MEDIA_ELEMENT_SRC);
	sink = create_test_element(PURPLE_MEDIA_ELEMENT_VIDEO | PURPLE_MEDIA_ELEMENT_SINK);
	videoconvert = gst_element_factory_make("videoconvert", NULL);
	videoscale = gst_element_factory_make("videoscale", NULL);

	g_object_set_data(G_OBJECT(pipeline), "sink", sink);

	gst_bin_add_many(GST_BIN(pipeline), src, videoconvert, videoscale, sink,
			NULL);
	gst_element_link_many(src, videoconvert, videoscale, sink, NULL);

	return pipeline;
}

static void
video_test_destroy_cb(G_GNUC_UNUSED GtkWidget *w, gpointer data)
{
	PidginVVPrefs *prefs = PIDGIN_VV_PREFS(data);

	if (!prefs->video.pipeline) {
		return;
	}

	gst_element_set_state(prefs->video.pipeline, GST_STATE_NULL);
	g_clear_pointer(&prefs->video.pipeline, gst_object_unref);
}

static void
enable_video_test(PidginVVPrefs *prefs)
{
	GtkWidget *video = NULL;
	GstElement *sink = NULL;

	prefs->video.pipeline = create_video_pipeline();

	sink = g_object_get_data(G_OBJECT(prefs->video.pipeline), "sink");
	g_object_get(sink, "widget", &video, NULL);
	gtk_widget_show(video);

	gtk_widget_set_size_request(prefs->video.frame, 400, 300);
	gtk_aspect_frame_set_child(GTK_ASPECT_FRAME(prefs->video.frame), video);

	gst_element_set_state(GST_ELEMENT(prefs->video.pipeline),
	                      GST_STATE_PLAYING);

	g_object_unref(video);
}

static void
toggle_video_test_cb(GtkToggleButton *test, gpointer data)
{
	PidginVVPrefs *prefs = PIDGIN_VV_PREFS(data);

	if (gtk_toggle_button_get_active(test)) {
		enable_video_test(prefs);
		g_signal_connect(test, "destroy",
		                 G_CALLBACK(video_test_destroy_cb), prefs);
	} else {
		g_object_disconnect(test, "any-signal::destroy",
		                    G_CALLBACK(video_test_destroy_cb), prefs,
		                    NULL);
		video_test_destroy_cb(NULL, prefs);
	}
}

static void
vv_device_changed_cb(const gchar *name, G_GNUC_UNUSED PurplePrefType type,
                     gconstpointer value, gpointer data)
{
	PidginVVPrefs *prefs = PIDGIN_VV_PREFS(data);

	PurpleMediaManager *manager;
	PurpleMediaElementInfo *info;

	manager = purple_media_manager_get();
	info = purple_media_manager_get_element_info(manager, value);
	purple_media_manager_set_active_element(manager, info);

	/* Refresh test viewers */
	if (strstr(name, "audio") && prefs->voice.pipeline) {
		voice_test_destroy_cb(NULL, prefs);
		enable_voice_test(prefs);
	} else if (strstr(name, "video") && prefs->video.pipeline) {
		video_test_destroy_cb(NULL, prefs);
		enable_video_test(prefs);
	}
}

static const char *
purple_media_type_to_preference_key(PurpleMediaElementType type)
{
	if (type & PURPLE_MEDIA_ELEMENT_AUDIO) {
		if (type & PURPLE_MEDIA_ELEMENT_SRC) {
			return PIDGIN_PREFS_ROOT "/vvconfig/audio/src/device";
		} else if (type & PURPLE_MEDIA_ELEMENT_SINK) {
			return PIDGIN_PREFS_ROOT "/vvconfig/audio/sink/device";
		}
	} else if (type & PURPLE_MEDIA_ELEMENT_VIDEO) {
		if (type & PURPLE_MEDIA_ELEMENT_SRC) {
			return PIDGIN_PREFS_ROOT "/vvconfig/video/src/device";
		} else if (type & PURPLE_MEDIA_ELEMENT_SINK) {
			return PIDGIN_PREFS_ROOT "/vvconfig/video/sink/device";
		}
	}

	return NULL;
}

static void
bind_vv_dropdown(PidginPrefCombo *combo, PurpleMediaElementType element_type)
{
	const gchar *preference_key;
	GtkTreeModel *model;

	preference_key = purple_media_type_to_preference_key(element_type);
	model = gtk_combo_box_get_model(GTK_COMBO_BOX(combo->combo));
	populate_vv_device_menuitems(element_type, GTK_LIST_STORE(model));

	combo->type = PURPLE_PREF_STRING;
	combo->key = preference_key;
	pidgin_prefs_bind_dropdown(combo);
}

static void
bind_vv_frame(PidginVVPrefs *prefs, PidginPrefCombo *combo,
              PurpleMediaElementType type)
{
	bind_vv_dropdown(combo, type);

	purple_prefs_connect_callback(combo->combo,
	                              purple_media_type_to_preference_key(type),
	                              vv_device_changed_cb, prefs);
	g_signal_connect_swapped(combo->combo, "destroy",
	                         G_CALLBACK(purple_prefs_disconnect_by_handle),
	                         combo->combo);

	g_object_set_data(G_OBJECT(combo->combo), "vv_media_type",
	                  (gpointer)type);
	g_object_set_data(G_OBJECT(combo->combo), "vv_combo", combo);
}

static void
device_list_changed_cb(G_GNUC_UNUSED PurpleMediaManager *manager,
                       GtkWidget *widget)
{
	PidginPrefCombo *combo;
	PurpleMediaElementType media_type;
	const gchar *preference_key;
	guint signal_id;
	GtkTreeModel *model;

	combo = g_object_get_data(G_OBJECT(widget), "vv_combo");
	media_type = (PurpleMediaElementType)GPOINTER_TO_INT(g_object_get_data(
			G_OBJECT(widget),
			"vv_media_type"));
	preference_key = purple_media_type_to_preference_key(media_type);

	/* Block signals so pref doesn't get re-saved while changing UI. */
	signal_id = g_signal_lookup("changed", GTK_TYPE_COMBO_BOX);
	g_signal_handlers_block_matched(combo->combo, G_SIGNAL_MATCH_ID, signal_id,
	                                0, NULL, NULL, NULL);

	model = gtk_combo_box_get_model(GTK_COMBO_BOX(combo->combo));
	populate_vv_device_menuitems(media_type, GTK_LIST_STORE(model));
	gtk_combo_box_set_active_id(GTK_COMBO_BOX(combo->combo),
	                            purple_prefs_get_string(preference_key));

	g_signal_handlers_unblock_matched(combo->combo, G_SIGNAL_MATCH_ID,
	                                  signal_id, 0, NULL, NULL, NULL);
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
pidgin_vv_prefs_class_init(PidginVVPrefsClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	gtk_widget_class_set_template_from_resource(
		widget_class,
		"/im/pidgin/Pidgin3/Prefs/vv.ui"
	);

	gtk_widget_class_bind_template_child(widget_class, PidginVVPrefs,
	                                     voice.input.combo);
	gtk_widget_class_bind_template_child(widget_class, PidginVVPrefs,
	                                     voice.output.combo);
	gtk_widget_class_bind_template_child(widget_class, PidginVVPrefs,
	                                     voice.volume);
	gtk_widget_class_bind_template_child(widget_class, PidginVVPrefs,
	                                     voice.threshold_row);
	gtk_widget_class_bind_template_child(widget_class, PidginVVPrefs,
	                                     voice.threshold);
	gtk_widget_class_bind_template_child(widget_class, PidginVVPrefs,
	                                     voice.level);
	gtk_widget_class_bind_template_child(widget_class, PidginVVPrefs,
	                                     voice.drop);
	gtk_widget_class_bind_template_child(widget_class, PidginVVPrefs,
	                                     voice.test);
	gtk_widget_class_bind_template_callback(widget_class, volume_changed_cb);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        threshold_value_changed_cb);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        toggle_voice_test_cb);

	gtk_widget_class_bind_template_child(widget_class, PidginVVPrefs,
	                                     video.input.combo);
	gtk_widget_class_bind_template_child(widget_class, PidginVVPrefs,
	                                     video.output.combo);
	gtk_widget_class_bind_template_child(widget_class, PidginVVPrefs,
	                                     video.frame);
	gtk_widget_class_bind_template_child(widget_class, PidginVVPrefs,
	                                     video.test);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        toggle_video_test_cb);
}

static void
pidgin_vv_prefs_init(PidginVVPrefs *prefs)
{
	PurpleMediaManager *manager = NULL;

	gtk_widget_init_template(GTK_WIDGET(prefs));

	manager = purple_media_manager_get();

	bind_vv_frame(prefs, &prefs->voice.input,
	              PURPLE_MEDIA_ELEMENT_AUDIO | PURPLE_MEDIA_ELEMENT_SRC);
	g_signal_connect_object(manager, "elements-changed::audiosrc",
	                        G_CALLBACK(device_list_changed_cb),
	                        prefs->voice.input.combo, 0);

	bind_vv_frame(prefs, &prefs->voice.output,
	              PURPLE_MEDIA_ELEMENT_AUDIO | PURPLE_MEDIA_ELEMENT_SINK);
	g_signal_connect_object(manager, "elements-changed::audiosink",
	                        G_CALLBACK(device_list_changed_cb),
	                        prefs->voice.output.combo, 0);

	bind_voice_test(prefs);

	bind_vv_frame(prefs, &prefs->video.input,
	              PURPLE_MEDIA_ELEMENT_VIDEO | PURPLE_MEDIA_ELEMENT_SRC);
	g_signal_connect_object(manager, "elements-changed::videosrc",
	                        G_CALLBACK(device_list_changed_cb),
	                        prefs->video.input.combo, 0);

	bind_vv_frame(prefs, &prefs->video.output,
	              PURPLE_MEDIA_ELEMENT_VIDEO | PURPLE_MEDIA_ELEMENT_SINK);
	g_signal_connect_object(manager, "elements-changed::videosink",
	                        G_CALLBACK(device_list_changed_cb),
	                        prefs->video.output.combo, 0);
}

/******************************************************************************
 * API
 *****************************************************************************/
GtkWidget *
pidgin_vv_prefs_new(void) {
	return GTK_WIDGET(g_object_new(PIDGIN_TYPE_VV_PREFS, NULL));
}

void
pidgin_vv_prefs_disable_test_pipelines(PidginVVPrefs *prefs) {
	g_return_if_fail(PIDGIN_IS_VV_PREFS(prefs));

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(prefs->voice.test), FALSE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(prefs->video.test), FALSE);
}
