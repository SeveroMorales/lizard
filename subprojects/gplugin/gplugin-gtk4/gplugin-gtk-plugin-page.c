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

#include <gplugin-gtk-plugin-closures.h>
#include <gplugin-gtk-plugin-page.h>
#include <gplugin-gtk-plugin-settings-list.h>

#define G_SETTINGS_ENABLE_BACKEND
#include <gio/gsettingsbackend.h>

/**
 * GPluginGtkPluginPage:
 *
 * A widget that displays a single [iface@GPlugin.Plugin] in a user friendly
 * way, intended to be placed in a [class@GPluginGtk4.View].
 *
 * Since: 0.39.0
 */

/******************************************************************************
 * Structs
 *****************************************************************************/
struct _GPluginGtkPluginPage {
	GtkBox parent;

	GPluginPlugin *plugin;
	GSettingsBackend *backend;

	GPluginGtkPluginSettingsList *settings;
};

/******************************************************************************
 * Enums
 *****************************************************************************/
enum {
	PROP_ZERO,
	PROP_PLUGIN,
	PROP_SETTINGS_BACKEND,
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
 * Helpers
 *****************************************************************************/
static void
gplugin_gtk_plugin_page_update_settings_list(GPluginGtkPluginPage *page)
{
	GSettings *settings = NULL;

	if(GPLUGIN_IS_PLUGIN(page->plugin)) {
		GPluginPluginInfo *info = NULL;

		info = gplugin_plugin_get_info(page->plugin);
		if(GPLUGIN_IS_PLUGIN_INFO(info)) {
			const gchar *schema = NULL;

			schema = gplugin_plugin_info_get_settings_schema(info);
			if(schema != NULL) {
				if(page->backend != NULL) {
					settings =
						g_settings_new_with_backend(schema, page->backend);
				} else {
					settings = g_settings_new(schema);
				}
			}
		}
	}

	gplugin_gtk_plugin_settings_list_set_settings(page->settings, settings);
}

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static gboolean
gplugin_gtk_plugin_page_enable_state_set_cb(
	G_GNUC_UNUSED GtkSwitch *widget,
	gboolean state,
	gpointer data)
{
	GPluginGtkPluginPage *page = GPLUGIN_GTK_PLUGIN_PAGE(data);

	g_signal_emit(G_OBJECT(page), signals[SIG_PLUGIN_STATE_SET], 0, state);

	return TRUE;
}

static gchar *
gplugin_gtk_plugin_page_newline_strjoinv_cb(
	G_GNUC_UNUSED GtkClosureExpression *expression,
	gchar **str_array,
	G_GNUC_UNUSED gpointer data)
{
	if(str_array == NULL) {
		return g_strdup(_("(none)"));
	}

	return g_strjoinv("\n", str_array);
}

static gchar *
gplugin_gtk_plugin_page_lookup_website_cb(
	G_GNUC_UNUSED GtkClosureExpression *expression,
	const gchar *website,
	G_GNUC_UNUSED gpointer data)
{
	if(website == NULL) {
		return NULL;
	}

	return g_markup_printf_escaped("<a href=\"%s\">%s</a>", website, website);
}

static gchar *
gplugin_gtk_plugin_page_lookup_error_cb(
	G_GNUC_UNUSED GtkClosureExpression *expression,
	GError *error,
	G_GNUC_UNUSED gpointer data)
{
	if(error == NULL) {
		return g_strdup(_("(none)"));
	}

	return g_strdup(error->message);
}

static gchar *
gplugin_gtk_plugin_page_lookup_abi_version_cb(
	G_GNUC_UNUSED GtkClosureExpression *expression,
	guint32 abi_version,
	G_GNUC_UNUSED gpointer data)
{
	return g_strdup_printf("%08x", abi_version);
}

static gchar *
gplugin_gtk_plugin_page_lookup_loader_cb(
	G_GNUC_UNUSED GtkClosureExpression *expression,
	GPluginPlugin *plugin,
	G_GNUC_UNUSED gpointer data)
{
	if(GPLUGIN_IS_PLUGIN(plugin)) {
		GPluginLoader *plugin_loader = gplugin_plugin_get_loader(plugin);

		if(GPLUGIN_IS_LOADER(plugin_loader)) {
			return g_strdup(G_OBJECT_TYPE_NAME(plugin_loader));
		}
	}

	return g_strdup(_("Unknown"));
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_TYPE(GPluginGtkPluginPage, gplugin_gtk_plugin_page, GTK_TYPE_BOX)

static void
gplugin_gtk_plugin_page_set_property(
	GObject *obj,
	guint prop_id,
	const GValue *value,
	GParamSpec *pspec)
{
	GPluginGtkPluginPage *page = GPLUGIN_GTK_PLUGIN_PAGE(obj);

	switch(prop_id) {
		case PROP_PLUGIN:
			gplugin_gtk_plugin_page_set_plugin(page, g_value_get_object(value));
			break;
		case PROP_SETTINGS_BACKEND:
			gplugin_gtk_plugin_page_set_settings_backend(
				page,
				g_value_get_object(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
			break;
	}
}

static void
gplugin_gtk_plugin_page_get_property(
	GObject *obj,
	guint prop_id,
	GValue *value,
	GParamSpec *pspec)
{
	GPluginGtkPluginPage *page = GPLUGIN_GTK_PLUGIN_PAGE(obj);

	switch(prop_id) {
		case PROP_PLUGIN:
			g_value_set_object(value, gplugin_gtk_plugin_page_get_plugin(page));
			break;
		case PROP_SETTINGS_BACKEND:
			g_value_set_object(
				value,
				gplugin_gtk_plugin_page_get_settings_backend(page));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
			break;
	}
}

static void
gplugin_gtk_plugin_page_dispose(GObject *obj)
{
	GPluginGtkPluginPage *page = GPLUGIN_GTK_PLUGIN_PAGE(obj);

	g_clear_object(&page->plugin);

	G_OBJECT_CLASS(gplugin_gtk_plugin_page_parent_class)->dispose(obj);
}

static void
gplugin_gtk_plugin_page_init(GPluginGtkPluginPage *page)
{
	gtk_widget_init_template(GTK_WIDGET(page));
}

static void
gplugin_gtk_plugin_page_class_init(GPluginGtkPluginPageClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->get_property = gplugin_gtk_plugin_page_get_property;
	obj_class->set_property = gplugin_gtk_plugin_page_set_property;
	obj_class->dispose = gplugin_gtk_plugin_page_dispose;

	/* properties */

	/**
	 * GPluginGtkPluginPage:plugin:
	 *
	 * The [iface@GPlugin.Plugin] whose info should be displayed.
	 *
	 * Since: 0.39.0
	 */
	properties[PROP_PLUGIN] = g_param_spec_object(
		"plugin",
		"plugin",
		"The GPluginPlugin whose info should be displayed",
		G_TYPE_OBJECT,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

	/**
	 * GPluginGtkPluginPage:settings-backend:
	 *
	 * The [class@Gio.SettingsBackend] to use when viewing plugin settings.
	 *
	 * Since: 0.40.0
	 */
	properties[PROP_SETTINGS_BACKEND] = g_param_spec_object(
		"settings-backend",
		"settings-backend",
		"The settings backend to use when viewing plugin settings",
		G_TYPE_SETTINGS_BACKEND,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);

	/* signals */

	/**
	 * GPluginGtkPluginPage::plugin-state-set:
	 * @page: The [class@GPluginGtk.PluginPage] instance.
	 * @enabled: Whether the plugin was requested to be enabled or disabled.
	 *
	 * Emitted when the plugin page enable switch is toggled.
	 *
	 * Since: 0.39.0
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
		"/org/imfreedom/keep/gplugin/gplugin-gtk/plugin-page.ui");

	gtk_widget_class_bind_template_child(
		widget_class,
		GPluginGtkPluginPage,
		settings);

	gtk_widget_class_bind_template_callback(
		widget_class,
		gplugin_gtk_lookup_plugin_name);
	gtk_widget_class_bind_template_callback(
		widget_class,
		gplugin_gtk_lookup_plugin_state);
	gtk_widget_class_bind_template_callback(
		widget_class,
		gplugin_gtk_lookup_plugin_state_sensitivity);
	gtk_widget_class_bind_template_callback(
		widget_class,
		gplugin_gtk_plugin_page_enable_state_set_cb);
	gtk_widget_class_bind_template_callback(
		widget_class,
		gplugin_gtk_plugin_page_newline_strjoinv_cb);
	gtk_widget_class_bind_template_callback(
		widget_class,
		gplugin_gtk_plugin_page_lookup_website_cb);
	gtk_widget_class_bind_template_callback(
		widget_class,
		gplugin_gtk_plugin_page_lookup_error_cb);
	gtk_widget_class_bind_template_callback(
		widget_class,
		gplugin_gtk_plugin_page_lookup_abi_version_cb);
	gtk_widget_class_bind_template_callback(
		widget_class,
		gplugin_gtk_plugin_page_lookup_loader_cb);
}

/******************************************************************************
 * Public API
 *****************************************************************************/

/**
 * gplugin_gtk_plugin_page_new:
 *
 * Create a new [class@GPluginGtk4.PluginPage] which can be used to display
 * info about a [iface@GPlugin.Plugin].
 *
 * Returns: (transfer full): The new widget.
 *
 * Since: 0.39.0
 */
GtkWidget *
gplugin_gtk_plugin_page_new(void)
{
	return g_object_new(GPLUGIN_GTK_TYPE_PLUGIN_PAGE, NULL);
}

/**
 * gplugin_gtk_plugin_page_set_plugin:
 * @page: The plugin page instance.
 * @plugin: (nullable): The plugin instance.
 *
 * Sets the [iface@GPlugin.Plugin] that should be displayed.
 *
 * A @plugin value of %NULL will clear the widget.
 *
 * Since: 0.39.0
 */
void
gplugin_gtk_plugin_page_set_plugin(
	GPluginGtkPluginPage *page,
	GPluginPlugin *plugin)
{
	g_return_if_fail(GPLUGIN_GTK_IS_PLUGIN_PAGE(page));

	if(g_set_object(&page->plugin, plugin)) {
		gplugin_gtk_plugin_page_update_settings_list(page);

		g_object_notify_by_pspec(G_OBJECT(page), properties[PROP_PLUGIN]);
	}
}

/**
 * gplugin_gtk_plugin_page_get_plugin:
 * @page: The plugin page instance.
 *
 * Gets the [iface@GPlugin.Plugin] that's being displayed.
 *
 * Returns: (transfer none) (nullable): The plugin that's being displayed, or
 *          %NULL if the page is empty.
 *
 * Since: 0.39.0
 */
GPluginPlugin *
gplugin_gtk_plugin_page_get_plugin(GPluginGtkPluginPage *page)
{
	g_return_val_if_fail(GPLUGIN_GTK_IS_PLUGIN_PAGE(page), NULL);

	return page->plugin;
}

/**
 * gplugin_gtk_plugin_page_set_settings_backend:
 * @page: The plugin page instance.
 * @backend: (transfer none) (nullable): The settings backend to use. If %NULL,
 *           the default GSettings backend will be used.
 *
 * Sets the settings backend to use when displaying plugin settings.
 *
 * Note, because we do not want to leak `G_SETTINGS_ENABLE_BACKEND` into
 * GPlugin users, this function takes a `gpointer` instead of a
 * `GSettingsBackend *` but the type will be checked internally.
 *
 * Since: 0.40.0
 */
void
gplugin_gtk_plugin_page_set_settings_backend(
	GPluginGtkPluginPage *page,
	gpointer backend)
{
	g_return_if_fail(GPLUGIN_GTK_IS_PLUGIN_PAGE(page));
	g_return_if_fail(G_IS_SETTINGS_BACKEND(backend) || backend == NULL);

	if(g_set_object(&page->backend, backend)) {
		gplugin_gtk_plugin_page_update_settings_list(page);

		g_object_notify_by_pspec(
			G_OBJECT(page),
			properties[PROP_SETTINGS_BACKEND]);
	}
}

/**
 * gplugin_gtk_plugin_page_get_settings_backend:
 * @page: The plugin page instance.
 *
 * Gets the settings backend used when displaying plugin settings.
 *
 * Note, because we do not want to leak `G_SETTINGS_ENABLE_BACKEND` into
 * GPlugin users, this function returns a `gpointer`, and you should cast to
 * `GSettingsBackend *` after setting `G_SETTINGS_ENABLE_BACKEND` for the files
 * where you need it.
 *
 * Returns: (transfer none): The settings backend in use.
 *
 * Since: 0.40.0
 */
gpointer
gplugin_gtk_plugin_page_get_settings_backend(GPluginGtkPluginPage *page)
{
	g_return_val_if_fail(GPLUGIN_GTK_IS_PLUGIN_PAGE(page), NULL);

	return page->backend;
}
