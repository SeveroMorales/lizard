/*
 * Copyright (C) 2022 Elliott Sales de Andrade <quantum.analyst@gmail.com>
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

#include <gplugin-gtk-plugin-settings-list.h>

/**
 * GPluginGtkPluginSettingslist:
 *
 * A [class@Gtk.ListBox] widget that displays all the settings from a plugin.
 *
 * Since: 0.40.0
 */

/******************************************************************************
 * Structs
 *****************************************************************************/
struct _GPluginGtkPluginSettingsList {
	GtkBox parent;

	GSettings *settings;
	GtkListBox *list_box;
	GList *rows;
};

/******************************************************************************
 * Enums
 *****************************************************************************/
enum {
	PROP_ZERO,
	PROP_SETTINGS,
	N_PROPERTIES,
};
static GParamSpec *properties[N_PROPERTIES] = {
	NULL,
};

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static gboolean
gplugin_gtk_plugin_settings_row_byte_to_double(
	GValue *value,
	GVariant *variant,
	G_GNUC_UNUSED gpointer data)
{
	g_value_set_double(value, (gdouble)g_variant_get_byte(variant));
	return TRUE;
}

static GVariant *
gplugin_gtk_plugin_settings_row_double_to_byte(
	const GValue *value,
	G_GNUC_UNUSED const GVariantType *expected_type,
	G_GNUC_UNUSED gpointer data)
{
	return g_variant_new_byte((char)g_value_get_double(value));
}

static gboolean
gplugin_gtk_plugin_settings_row_enum_to_selected(
	GValue *value,
	GVariant *variant,
	gpointer data)
{
	GListModel *model = data;
	const char *string = NULL;
	guint n_items;

	string = g_variant_get_string(variant, NULL);
	n_items = g_list_model_get_n_items(model);
	for(guint index = 0; index < n_items; index++) {
		GtkStringObject *obj = g_list_model_get_item(model, index);

		if(g_str_equal(string, gtk_string_object_get_string(obj))) {
			g_value_set_uint(value, index);
			g_object_unref(obj);
			return TRUE;
		}

		g_object_unref(obj);
	}

	return FALSE;
}

static GVariant *
gplugin_gtk_plugin_settings_row_selected_to_enum(
	const GValue *value,
	G_GNUC_UNUSED const GVariantType *expected_type,
	gpointer data)
{
	GListModel *model = data;
	GtkStringObject *obj = NULL;
	GVariant *result = NULL;

	obj = g_list_model_get_item(model, g_value_get_uint(value));
	result = g_variant_new_string(gtk_string_object_get_string(obj));
	g_object_unref(obj);

	return result;
}

/******************************************************************************
 * Helpers
 *****************************************************************************/
static GtkWidget *
gplugin_gtk_plugin_settings_row_new_for_enum(
	GSettings *settings,
	const char *name,
	GVariant *range_value)
{
	GtkWidget *dropdown = NULL;
	const gchar **allowed_values = NULL;
	GListModel *model = NULL;

	allowed_values = g_variant_get_strv(range_value, NULL);
	dropdown = gtk_drop_down_new_from_strings(allowed_values);
	g_free(allowed_values);

	model = gtk_drop_down_get_model(GTK_DROP_DOWN(dropdown));
	g_settings_bind_with_mapping(
		settings,
		name,
		dropdown,
		"selected",
		G_SETTINGS_BIND_DEFAULT,
		gplugin_gtk_plugin_settings_row_enum_to_selected,
		gplugin_gtk_plugin_settings_row_selected_to_enum,
		g_object_ref(model),
		g_object_unref);

	return dropdown;
}

static GtkWidget *
gplugin_gtk_plugin_settings_row_new_for_flags(
	G_GNUC_UNUSED GSettings *settings,
	G_GNUC_UNUSED const char *name,
	G_GNUC_UNUSED GVariant *range_value)
{
	GtkWidget *dropdown = NULL;

	/* TODO: Implement this. */
	dropdown = gtk_label_new("Unimplemented flags setting");

	return dropdown;
}

static GtkWidget *
gplugin_gtk_plugin_settings_row_new_for_range(
	GSettings *settings,
	const char *name,
	GVariant *range_value)
{
	GtkWidget *spin = NULL;
	const char *type = NULL;
	gdouble min, max, step;

	/* By default, for integers, pick a step of 1, which in GtkSpinButton
	 * limits it to displaying only integers. */
	step = 1.0;

	/* The range value is a tuple with a pair of the type of the expected value
	 * (e.g., "(ii)" for int16 or "(dd)" for double), so look at second
	 * character. */
	type = g_variant_get_type_string(range_value);
	switch(type[1]) {
		case 'y': {
			guint8 mini, maxi;
			g_variant_get_child(range_value, 0, "y", &mini);
			g_variant_get_child(range_value, 1, "y", &maxi);
			min = (gdouble)mini;
			max = (gdouble)maxi;
		} break;

		case 'n': {
			gint16 mini, maxi;
			g_variant_get_child(range_value, 0, "n", &mini);
			g_variant_get_child(range_value, 1, "n", &maxi);
			min = (gdouble)mini;
			max = (gdouble)maxi;
		} break;

		case 'q': {
			guint16 mini, maxi;
			g_variant_get_child(range_value, 0, "q", &mini);
			g_variant_get_child(range_value, 1, "q", &maxi);
			min = (gdouble)mini;
			max = (gdouble)maxi;
		} break;

		case 'i': {
			gint32 mini, maxi;
			g_variant_get_child(range_value, 0, "i", &mini);
			g_variant_get_child(range_value, 1, "i", &maxi);
			min = (gdouble)mini;
			max = (gdouble)maxi;
		} break;

		case 'u': {
			guint32 mini, maxi;
			g_variant_get_child(range_value, 0, "u", &mini);
			g_variant_get_child(range_value, 1, "u", &maxi);
			min = (gdouble)mini;
			max = (gdouble)maxi;
		} break;

		case 'x': {
			gint64 mini, maxi;
			g_variant_get_child(range_value, 0, "x", &mini);
			g_variant_get_child(range_value, 1, "x", &maxi);
			min = (gdouble)mini;
			max = (gdouble)maxi;
		} break;

		case 't': {
			guint64 mini, maxi;
			g_variant_get_child(range_value, 0, "t", &mini);
			g_variant_get_child(range_value, 1, "t", &maxi);
			min = (gdouble)mini;
			max = (gdouble)maxi;
		} break;

		case 'd':
			/* For doubles, arbitrarily pick a step of 1% of the range. */
			g_variant_get_child(range_value, 0, "d", &min);
			g_variant_get_child(range_value, 1, "d", &max);
			step = (max - min) / 100;
			break;

		default:
			g_warning("Unknown range type: %s", type);
			min = 0.0;
			max = 1.0;
			break;
	}

	spin = gtk_spin_button_new_with_range(min, max, step);
	if(type[1] == 'y') {
		/* For some reason, Gio's default bindings understand
		 * double-to-int conversions for any bit size other than
		 * bytes, so manually set up mappings for bytes... */
		g_settings_bind_with_mapping(
			settings,
			name,
			spin,
			"value",
			G_SETTINGS_BIND_DEFAULT,
			gplugin_gtk_plugin_settings_row_byte_to_double,
			gplugin_gtk_plugin_settings_row_double_to_byte,
			NULL,
			NULL);
	} else {
		/* ... and use default binding everywhere else. */
		g_settings_bind(settings, name, spin, "value", G_SETTINGS_BIND_DEFAULT);
	}

	return spin;
}

static GtkWidget *
gplugin_gtk_plugin_settings_row_new_for_type(
	GSettings *settings,
	const char *name,
	GVariant *range_value)
{
	GtkWidget *entry = NULL;
	char type;
	gdouble min, max;

	/* The range value is an array with type of the expected value (e.g., "ab"
	 * for boolean or "as" for string), so look at second character. */
	type = g_variant_get_type_string(range_value)[1];

	switch(type) {
		/* Boolean */
		case 'b':
			entry = gtk_switch_new();
			gtk_widget_set_halign(entry, GTK_ALIGN_END);
			gtk_widget_set_valign(entry, GTK_ALIGN_CENTER);

			g_settings_bind(
				settings,
				name,
				entry,
				"active",
				G_SETTINGS_BIND_DEFAULT);
			break;

		/* Integral numbers */
		case 'y':
		case 'n':
		case 'q':
		case 'i':
		case 'u':
		case 'x':
		case 't':
			/* Set range for the spin button. */
			switch(type) {
				case 'y':
					min = 0.0;
					max = G_MAXUINT8;
					break;
				case 'n':
					min = G_MININT16;
					max = G_MAXINT16;
					break;
				case 'q':
					min = 0.0;
					max = G_MAXUINT16;
					break;
				case 'i':
					min = G_MININT32;
					max = G_MAXINT32;
					break;
				case 'u':
					min = 0.0;
					max = G_MAXUINT32;
					break;
				case 'x':
					min = G_MININT64;
					max = (gdouble)G_MAXINT64;
					break;
				case 't':
					min = 0.0;
					max = (gdouble)G_MAXUINT64;
					break;
			}

			entry = gtk_spin_button_new_with_range(min, max, 1.0);
			if(type == 'y') {
				/* For some reason, Gio's default bindings understand
				 * double-to-int conversions for any bit size other than
				 * bytes, so manually set up mappings for bytes... */
				g_settings_bind_with_mapping(
					settings,
					name,
					entry,
					"value",
					G_SETTINGS_BIND_DEFAULT,
					gplugin_gtk_plugin_settings_row_byte_to_double,
					gplugin_gtk_plugin_settings_row_double_to_byte,
					NULL,
					NULL);
			} else {
				/* ... and use default binding everywhere else. */
				g_settings_bind(
					settings,
					name,
					entry,
					"value",
					G_SETTINGS_BIND_DEFAULT);
			}

			break;

		/* Double numbers */
		case 'd':
			entry = gtk_spin_button_new(NULL, 1.0, 5);
			g_settings_bind(
				settings,
				name,
				entry,
				"value",
				G_SETTINGS_BIND_DEFAULT);
			break;

		/* Strings */
		case 's':
			entry = gtk_entry_new();
			g_settings_bind(
				settings,
				name,
				gtk_entry_get_buffer(GTK_ENTRY(entry)),
				"text",
				G_SETTINGS_BIND_DEFAULT);
			break;

		default:
			entry = gtk_label_new(_("Unknown setting type"));
			break;
	}

	return entry;
}

static GtkWidget *
gplugin_gtk_plugin_settings_row_new_for_key(
	GSettings *settings,
	const gchar *name,
	GSettingsSchemaKey *key,
	GtkSizeGroup *sg)
{
	GtkWidget *widget = NULL;
	GtkWidget *label = NULL;
	GtkWidget *entry = NULL;
	const char *summary = NULL;
	GVariant *range = NULL;
	gchar *range_type = NULL;
	GVariant *range_value = NULL;

	widget = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);

	summary = g_settings_schema_key_get_summary(key);
	label = gtk_label_new((summary != NULL) ? summary : name);
	gtk_label_set_xalign(GTK_LABEL(label), 0);
	gtk_box_append(GTK_BOX(widget), label);
	gtk_size_group_add_widget(sg, label);

	gtk_widget_set_tooltip_text(
		widget,
		g_settings_schema_key_get_description(key));

	range = g_settings_schema_key_get_range(key);
	g_variant_get_child(range, 0, "s", &range_type);
	g_variant_get_child(range, 1, "v", &range_value);

	if(g_str_equal(range_type, "enum")) {
		entry = gplugin_gtk_plugin_settings_row_new_for_enum(
			settings,
			name,
			range_value);
	} else if(g_str_equal(range_type, "flags")) {
		entry = gplugin_gtk_plugin_settings_row_new_for_flags(
			settings,
			name,
			range_value);
	} else if(g_str_equal(range_type, "range")) {
		entry = gplugin_gtk_plugin_settings_row_new_for_range(
			settings,
			name,
			range_value);
	} else if(g_str_equal(range_type, "type")) {
		entry = gplugin_gtk_plugin_settings_row_new_for_type(
			settings,
			name,
			range_value);
	} else {
		/* No other documented types for g_settings_schema_key_get_range. */
		g_warn_if_reached();
		entry = gtk_label_new(_("Unknown setting type"));
	}

	g_variant_unref(range_value);
	g_free(range_type);
	g_variant_unref(range);

	gtk_widget_set_hexpand(entry, TRUE);
	gtk_box_append(GTK_BOX(widget), entry);

	return widget;
}

static void
gplugin_gtk_plugin_settings_list_refresh(
	GPluginGtkPluginSettingsList *list,
	GSettings *settings)
{
	GSettingsSchema *schema = NULL;
	gchar **names = NULL;
	GtkSizeGroup *sg = NULL;

	while(list->rows) {
		gtk_list_box_remove(list->list_box, list->rows->data);
		list->rows = g_list_delete_link(list->rows, list->rows);
	}

	if(settings == NULL) {
		return;
	}

	g_object_get(G_OBJECT(settings), "settings-schema", &schema, NULL);
	names = g_settings_schema_list_keys(schema);
	sg = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);

	for(gint i = 0; names[i] != NULL; i++) {
		GSettingsSchemaKey *key = NULL;
		GtkWidget *row = NULL;
		GtkWidget *widget = NULL;

		key = g_settings_schema_get_key(schema, names[i]);
		widget = gplugin_gtk_plugin_settings_row_new_for_key(
			settings,
			names[i],
			key,
			sg);

		row = gtk_list_box_row_new();
		gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), widget);
		gtk_widget_set_focusable(row, FALSE);
		gtk_list_box_append(list->list_box, row);

		list->rows = g_list_append(list->rows, row);

		g_settings_schema_key_unref(key);
	}

	g_strfreev(names);
	g_settings_schema_unref(schema);
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_TYPE(
	GPluginGtkPluginSettingsList,
	gplugin_gtk_plugin_settings_list,
	GTK_TYPE_BOX)

static void
gplugin_gtk_plugin_settings_list_set_property(
	GObject *obj,
	guint prop_id,
	const GValue *value,
	GParamSpec *pspec)
{
	GPluginGtkPluginSettingsList *list = GPLUGIN_GTK_PLUGIN_SETTINGS_LIST(obj);

	switch(prop_id) {
		case PROP_SETTINGS:
			gplugin_gtk_plugin_settings_list_set_settings(
				list,
				g_value_get_object(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
			break;
	}
}

static void
gplugin_gtk_plugin_settings_list_get_property(
	GObject *obj,
	guint prop_id,
	GValue *value,
	GParamSpec *pspec)
{
	GPluginGtkPluginSettingsList *list = GPLUGIN_GTK_PLUGIN_SETTINGS_LIST(obj);

	switch(prop_id) {
		case PROP_SETTINGS:
			g_value_set_object(
				value,
				gplugin_gtk_plugin_settings_list_get_settings(list));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
			break;
	}
}

static void
gplugin_gtk_plugin_settings_list_finalize(GObject *obj)
{
	GPluginGtkPluginSettingsList *list = GPLUGIN_GTK_PLUGIN_SETTINGS_LIST(obj);

	g_clear_object(&list->settings);
	g_clear_pointer(&list->rows, g_list_free);

	G_OBJECT_CLASS(gplugin_gtk_plugin_settings_list_parent_class)
		->finalize(obj);
}

static void
gplugin_gtk_plugin_settings_list_class_init(
	GPluginGtkPluginSettingsListClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	obj_class->get_property = gplugin_gtk_plugin_settings_list_get_property;
	obj_class->set_property = gplugin_gtk_plugin_settings_list_set_property;
	obj_class->finalize = gplugin_gtk_plugin_settings_list_finalize;

	gtk_widget_class_set_template_from_resource(
		widget_class,
		"/org/imfreedom/keep/gplugin/gplugin-gtk/plugin-settings-list.ui");

	gtk_widget_class_bind_template_child(
		widget_class,
		GPluginGtkPluginSettingsList,
		list_box);

	/* properties */

	/**
	 * GPluginGtkPluginSettingsList:settings:
	 *
	 * The [class@Gio.Settings] to display.
	 */
	properties[PROP_SETTINGS] = g_param_spec_object(
		"settings",
		"settings",
		"The settings to display",
		G_TYPE_SETTINGS,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

static void
gplugin_gtk_plugin_settings_list_init(GPluginGtkPluginSettingsList *self)
{
	gtk_widget_init_template(GTK_WIDGET(self));
}

/******************************************************************************
 * API
 *****************************************************************************/

/**
 * gplugin_gtk_plugin_settings_list_new:
 *
 * Creates a new [class@GPluginGtk4.PluginSettingsList].
 *
 * Returns: (transfer full): The new list.
 *
 * Since: 0.40.0
 */
GtkWidget *
gplugin_gtk_plugin_settings_list_new(void)
{
	return g_object_new(GPLUGIN_GTK_TYPE_PLUGIN_SETTINGS_LIST, NULL);
}

/**
 * gplugin_gtk_plugin_settings_list_set_settings:
 * @list: The instance.
 * @settings: (transfer none) (nullable): The plugin settings to display.
 *
 * This function will set which plugin settings to display.
 *
 * Since: 0.40.0
 */
void
gplugin_gtk_plugin_settings_list_set_settings(
	GPluginGtkPluginSettingsList *list,
	GSettings *settings)
{
	g_return_if_fail(GPLUGIN_GTK_IS_PLUGIN_SETTINGS_LIST(list));
	g_return_if_fail(G_IS_SETTINGS(settings) || settings == NULL);

	if(g_set_object(&list->settings, settings)) {
		gplugin_gtk_plugin_settings_list_refresh(list, settings);

		g_object_notify(G_OBJECT(list), "settings");
	}
}

/**
 * gplugin_gtk_plugin_settings_list_get_settings:
 * @list: The instance.
 *
 * Returns the plugin settings that are being displayed.
 *
 * Returns: (transfer none): The settings being displayed.
 *
 * Since: 0.40.0
 */
GSettings *
gplugin_gtk_plugin_settings_list_get_settings(
	GPluginGtkPluginSettingsList *list)
{
	g_return_val_if_fail(GPLUGIN_GTK_IS_PLUGIN_SETTINGS_LIST(list), NULL);

	return list->settings;
}
