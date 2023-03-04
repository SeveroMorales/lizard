/*
 * Copyright (C) 2021-2022 Elliott Sales de Andrade <quantum.analyst@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <https://www.gnu.org/licenses/>.
 */

#include <glib/gi18n-lib.h>

#include <gplugin.h>

#include <gplugin-gtk-plugin-closures.h>
#include <gplugin-gtk-plugin-row.h>

/**
 * GPluginGtkPluginRow:
 *
 * A widget that displays a [iface@GPlugin.Plugin] in a user friendly way,
 * intended to be placed in a [class@Gtk.ListBox].
 */

/******************************************************************************
 * Structs
 *****************************************************************************/
struct _GPluginGtkPluginRow {
	GtkListBoxRow parent;

	GPluginPlugin *plugin;

	/* Header */
	GtkWidget *title;
	GtkWidget *summary;
};

/******************************************************************************
 * Enums
 *****************************************************************************/
enum {
	PROP_ZERO,
	PROP_PLUGIN,
	N_PROPERTIES,
};
static GParamSpec *properties[N_PROPERTIES] = {
	NULL,
};

enum {
	SIG_PLUGIN_STATE_SET,
	N_SIGNALS,
};
static guint signals[N_SIGNALS] = {
	0,
};

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static gboolean
gplugin_gtk_plugin_row_enable_state_set_cb(
	G_GNUC_UNUSED GtkSwitch *widget,
	gboolean state,
	gpointer data)
{
	GPluginGtkPluginRow *row = GPLUGIN_GTK_PLUGIN_ROW(data);

	g_signal_emit(G_OBJECT(row), signals[SIG_PLUGIN_STATE_SET], 0, state);

	return TRUE;
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_TYPE(
	GPluginGtkPluginRow,
	gplugin_gtk_plugin_row,
	GTK_TYPE_LIST_BOX_ROW)

static void
gplugin_gtk_plugin_row_set_property(
	GObject *obj,
	guint prop_id,
	const GValue *value,
	GParamSpec *pspec)
{
	GPluginGtkPluginRow *row = GPLUGIN_GTK_PLUGIN_ROW(obj);

	switch(prop_id) {
		case PROP_PLUGIN:
			gplugin_gtk_plugin_row_set_plugin(row, g_value_get_object(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
			break;
	}
}

static void
gplugin_gtk_plugin_row_get_property(
	GObject *obj,
	guint prop_id,
	GValue *value,
	GParamSpec *pspec)
{
	GPluginGtkPluginRow *row = GPLUGIN_GTK_PLUGIN_ROW(obj);

	switch(prop_id) {
		case PROP_PLUGIN:
			g_value_set_object(value, gplugin_gtk_plugin_row_get_plugin(row));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
			break;
	}
}

static void
gplugin_gtk_plugin_row_finalize(GObject *obj)
{
	GPluginGtkPluginRow *row = GPLUGIN_GTK_PLUGIN_ROW(obj);

	g_clear_object(&row->plugin);

	G_OBJECT_CLASS(gplugin_gtk_plugin_row_parent_class)->finalize(obj);
}

static void
gplugin_gtk_plugin_row_init(GPluginGtkPluginRow *row)
{
	gtk_widget_init_template(GTK_WIDGET(row));
}

static void
gplugin_gtk_plugin_row_class_init(GPluginGtkPluginRowClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->get_property = gplugin_gtk_plugin_row_get_property;
	obj_class->set_property = gplugin_gtk_plugin_row_set_property;
	obj_class->finalize = gplugin_gtk_plugin_row_finalize;

	/* properties */

	/**
	 * GPluginGtkPluginRow:plugin:
	 *
	 * The [iface@GPlugin.Plugin] whose info should be displayed.
	 *
	 * Since: 0.38.0
	 */
	properties[PROP_PLUGIN] = g_param_spec_object(
		"plugin",
		"plugin",
		"The GPluginPlugin whose info should be displayed",
		G_TYPE_OBJECT,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);

	/* signals */

	/**
	 * GPluginGtkPluginRow::plugin-state-set:
	 * @row: The [class@GPluginGtk.PluginRow] instance.
	 * @enabled: Whether the plugin was requested to be enabled or disabled.
	 *
	 * Emitted when the plugin row enable switch is toggled.
	 *
	 * Since: 0.38.0
	 */
	signals[SIG_PLUGIN_STATE_SET] = g_signal_new_class_handler(
		"plugin-state-set",
		G_OBJECT_CLASS_TYPE(klass),
		G_SIGNAL_RUN_LAST,
		NULL,
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		1,
		G_TYPE_BOOLEAN);

	/* template stuff */
	gtk_widget_class_set_template_from_resource(
		widget_class,
		"/org/imfreedom/keep/gplugin/gplugin-gtk/plugin-row.ui");

	/* Header */
	gtk_widget_class_bind_template_child(
		widget_class,
		GPluginGtkPluginRow,
		title);
	gtk_widget_class_bind_template_child(
		widget_class,
		GPluginGtkPluginRow,
		summary);
	gtk_widget_class_bind_template_callback(
		widget_class,
		gplugin_gtk_plugin_row_enable_state_set_cb);
	gtk_widget_class_bind_template_callback(
		widget_class,
		gplugin_gtk_lookup_plugin_name);
	gtk_widget_class_bind_template_callback(
		widget_class,
		gplugin_gtk_lookup_plugin_state);
	gtk_widget_class_bind_template_callback(
		widget_class,
		gplugin_gtk_lookup_plugin_state_sensitivity);
}

/******************************************************************************
 * Public API
 *****************************************************************************/

/**
 * gplugin_gtk_plugin_row_new:
 *
 * Create a new [class@GPluginGtk4.PluginRow] which can be used to display info
 * about a [iface@GPlugin.Plugin].
 *
 * Returns: (transfer full): The new widget.
 *
 * Since: 0.38.0
 */
GtkWidget *
gplugin_gtk_plugin_row_new(void)
{
	return g_object_new(GPLUGIN_GTK_TYPE_PLUGIN_ROW, NULL);
}

/**
 * gplugin_gtk_plugin_row_set_plugin:
 * @row: The plugin row instance.
 * @plugin: (nullable): The plugin instance.
 *
 * Sets the [iface@GPlugin.Plugin] that should be displayed.
 *
 * A @plugin value of %NULL will clear the widget.
 *
 * Since: 0.38.0
 */
void
gplugin_gtk_plugin_row_set_plugin(
	GPluginGtkPluginRow *row,
	GPluginPlugin *plugin)
{
	g_return_if_fail(GPLUGIN_GTK_IS_PLUGIN_ROW(row));

	if(g_set_object(&row->plugin, plugin)) {
		g_object_notify_by_pspec(G_OBJECT(row), properties[PROP_PLUGIN]);
	}
}

/**
 * gplugin_gtk_plugin_row_get_plugin:
 * @row: The plugin row instance.
 *
 * Returns the [iface@GPlugin.Plugin] that's being displayed.
 *
 * Returns: (transfer none) (nullable): The plugin that's being displayed, or
 *          %NULL if the row is empty.
 *
 * Since: 0.38.0
 */
GPluginPlugin *
gplugin_gtk_plugin_row_get_plugin(GPluginGtkPluginRow *row)
{
	g_return_val_if_fail(GPLUGIN_GTK_IS_PLUGIN_ROW(row), NULL);

	return row->plugin;
}

/**
 * gplugin_gtk_plugin_row_get_sort_key:
 * @row: The plugin row instance.
 *
 * Returns a key that can be used to sort this row.
 *
 * Returns: The sort key.
 *
 * Since: 0.38.0
 */
gchar *
gplugin_gtk_plugin_row_get_sort_key(GPluginGtkPluginRow *row)
{
	g_return_val_if_fail(GPLUGIN_GTK_IS_PLUGIN_ROW(row), NULL);

	return g_strdup(gtk_label_get_text(GTK_LABEL(row->title)));
}

/**
 * gplugin_gtk_plugin_row_matches_search:
 * @row: The plugin row instance.
 * @text: The text to search for.
 *
 * Matches this row instance against some text to be searched for.
 *
 * Returns: Whether the row matches the text or not.
 *
 * Since: 0.38.0
 */
gboolean
gplugin_gtk_plugin_row_matches_search(
	GPluginGtkPluginRow *row,
	const gchar *text)
{
	const gchar *value = NULL;

	g_return_val_if_fail(GPLUGIN_GTK_IS_PLUGIN_ROW(row), FALSE);

	value = gtk_label_get_text(GTK_LABEL(row->title));
	if(g_strstr_len(value, -1, text)) {
		return TRUE;
	}

	value = gtk_label_get_text(GTK_LABEL(row->summary));
	if(g_strstr_len(value, -1, text)) {
		return TRUE;
	}

	if(GPLUGIN_IS_PLUGIN(row->plugin)) {
		GPluginPluginInfo *plugin_info = NULL;
		gchar *filename = NULL;

		plugin_info = gplugin_plugin_get_info(row->plugin);
		value = gplugin_plugin_info_get_description(plugin_info);
		g_clear_object(&plugin_info);
		if(value != NULL && g_strstr_len(value, -1, text)) {
			return TRUE;
		}

		filename = gplugin_plugin_get_filename(row->plugin);
		if(filename != NULL && g_strstr_len(filename, -1, text)) {
			g_free(filename);
			return TRUE;
		}
		g_free(filename);
	}

	return FALSE;
}
