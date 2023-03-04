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

#include <gplugin.h>

#include <gplugin-gtk-plugin-page.h>
#include <gplugin-gtk-plugin-row.h>
#include <gplugin-gtk-view.h>

#define G_SETTINGS_ENABLE_BACKEND
#include <gio/gsettingsbackend.h>

/**
 * GPluginGtkView:
 *
 * A widget that displays all the plugins and some basic information about
 * them.
 */

/******************************************************************************
 * Structs
 *****************************************************************************/
struct _GPluginGtkView {
	GtkBox parent;

	GPluginManager *manager;
	GSettingsBackend *backend;
	gboolean show_internal;
	GtkWidget *stack;

	/* Overview */
	GtkWidget *list_box;
	GtkWidget *search_bar;
	GtkWidget *search_entry;

	/* Plugin details */
	GtkWidget *plugin_page;
};

/******************************************************************************
 * Enums
 *****************************************************************************/
enum {
	PROP_ZERO,
	PROP_MANAGER,
	PROP_SHOW_INTERNAL,
	PROP_SETTINGS_BACKEND,
	N_PROPERTIES,
};
static GParamSpec *properties[N_PROPERTIES] = {
	NULL,
};

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
gplugin_gtk_view_row_activated(
	G_GNUC_UNUSED GtkListBox *box,
	GtkListBoxRow *row,
	gpointer data)
{
	GPluginGtkView *view = GPLUGIN_GTK_VIEW(data);
	GPluginGtkPluginRow *plugin_row = GPLUGIN_GTK_PLUGIN_ROW(row);
	GPluginPlugin *plugin = NULL;

	plugin = gplugin_gtk_plugin_row_get_plugin(plugin_row);
	gplugin_gtk_view_show_plugin(view, plugin);
}

static void
gplugin_gtk_view_plugin_state_set_cb(
	GObject *row_or_page,
	gboolean state,
	gpointer data)
{
	GPluginGtkView *view = GPLUGIN_GTK_VIEW(data);
	GPluginPlugin *plugin = NULL;
	GError *error = NULL;

	g_object_get(row_or_page, "plugin", &plugin, NULL);
	if(state) {
		gplugin_manager_load_plugin(view->manager, plugin, &error);

		if(error != NULL) {
			g_warning("Failed to load plugin: %s", error->message);

			g_error_free(error);
		}
	} else {
		gplugin_manager_unload_plugin(view->manager, plugin, &error);

		if(error != NULL) {
			g_warning("Failed to unload plugin: %s", error->message);

			g_error_free(error);
		}
	}

	g_clear_object(&plugin);
}

static void
gplugin_gtk_view_add_plugins_to_list(
	G_GNUC_UNUSED const gchar *id,
	GSList *plugins,
	gpointer data)
{
	GPluginGtkView *view = GPLUGIN_GTK_VIEW(data);

	for(; plugins; plugins = plugins->next) {
		GtkWidget *row = gplugin_gtk_plugin_row_new();
		gplugin_gtk_plugin_row_set_plugin(
			GPLUGIN_GTK_PLUGIN_ROW(row),
			GPLUGIN_PLUGIN(plugins->data));
		g_signal_connect(
			row,
			"plugin-state-set",
			G_CALLBACK(gplugin_gtk_view_plugin_state_set_cb),
			view);
		gtk_list_box_append(GTK_LIST_BOX(view->list_box), row);
	}
}

static int
gplugin_gtk_view_sort_func(
	GtkListBoxRow *row1,
	GtkListBoxRow *row2,
	G_GNUC_UNUSED gpointer data)
{
	GPluginGtkPluginRow *plugin_row1 = NULL, *plugin_row2 = NULL;
	gchar *key1 = NULL, *key2 = NULL;
	gint ret = 0;

	plugin_row1 = GPLUGIN_GTK_PLUGIN_ROW(row1);
	key1 = gplugin_gtk_plugin_row_get_sort_key(plugin_row1);

	plugin_row2 = GPLUGIN_GTK_PLUGIN_ROW(row2);
	key2 = gplugin_gtk_plugin_row_get_sort_key(plugin_row2);

	ret = g_strcmp0(key1, key2);

	g_free(key1);
	g_free(key2);

	return ret;
}

static gboolean
gplugin_gtk_view_filter_func(GtkListBoxRow *row, gpointer data)
{
	GPluginGtkView *view = GPLUGIN_GTK_VIEW(data);
	gboolean visible = TRUE;

	if(gtk_search_bar_get_search_mode(GTK_SEARCH_BAR(view->search_bar))) {
		const gchar *text =
			gtk_editable_get_text(GTK_EDITABLE(view->search_entry));
		if(text != NULL && text[0] != '\0') {
			GPluginGtkPluginRow *plugin_row = GPLUGIN_GTK_PLUGIN_ROW(row);
			visible = gplugin_gtk_plugin_row_matches_search(plugin_row, text);
		}
	}

	return visible;
}

static void
gplugin_gtk_view_search_changed(
	G_GNUC_UNUSED GtkSearchEntry *entry,
	gpointer data)
{
	GPluginGtkView *view = GPLUGIN_GTK_VIEW(data);

	gtk_list_box_invalidate_filter(GTK_LIST_BOX(view->list_box));
}

static void
gplugin_gtk_view_back_clicked_cb(G_GNUC_UNUSED GtkButton *button, gpointer data)
{
	GPluginGtkView *view = GPLUGIN_GTK_VIEW(data);

	gplugin_gtk_view_show_overview(view);
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_TYPE(GPluginGtkView, gplugin_gtk_view, GTK_TYPE_BOX)

static void
gplugin_gtk_view_set_property(
	GObject *obj,
	guint prop_id,
	const GValue *value,
	GParamSpec *pspec)
{
	GPluginGtkView *view = GPLUGIN_GTK_VIEW(obj);

	switch(prop_id) {
		case PROP_MANAGER:
			gplugin_gtk_view_set_manager(view, g_value_get_object(value));
			break;
		case PROP_SHOW_INTERNAL:
			gplugin_gtk_view_set_show_internal(
				view,
				g_value_get_boolean(value));
			break;
		case PROP_SETTINGS_BACKEND:
			gplugin_gtk_view_set_settings_backend(
				view,
				g_value_get_object(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
			break;
	}
}

static void
gplugin_gtk_view_get_property(
	GObject *obj,
	guint prop_id,
	GValue *value,
	GParamSpec *pspec)
{
	GPluginGtkView *view = GPLUGIN_GTK_VIEW(obj);

	switch(prop_id) {
		case PROP_MANAGER:
			g_value_set_object(value, gplugin_gtk_view_get_manager(view));
			break;
		case PROP_SHOW_INTERNAL:
			g_value_set_boolean(
				value,
				gplugin_gtk_view_get_show_internal(view));
			break;
		case PROP_SETTINGS_BACKEND:
			g_value_set_object(
				value,
				gplugin_gtk_view_get_settings_backend(view));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
			break;
	}
}

static void
gplugin_gtk_view_finalize(GObject *obj)
{
	GPluginGtkView *view = GPLUGIN_GTK_VIEW(obj);

	g_clear_object(&view->manager);

	G_OBJECT_CLASS(gplugin_gtk_view_parent_class)->finalize(obj);
}

static void
gplugin_gtk_view_class_init(GPluginGtkViewClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	obj_class->get_property = gplugin_gtk_view_get_property;
	obj_class->set_property = gplugin_gtk_view_set_property;
	obj_class->finalize = gplugin_gtk_view_finalize;

	gtk_widget_class_set_template_from_resource(
		widget_class,
		"/org/imfreedom/keep/gplugin/gplugin-gtk/view.ui");

	gtk_widget_class_bind_template_child(widget_class, GPluginGtkView, stack);

	gtk_widget_class_bind_template_child(
		widget_class,
		GPluginGtkView,
		list_box);
	gtk_widget_class_bind_template_child(
		widget_class,
		GPluginGtkView,
		search_bar);
	gtk_widget_class_bind_template_child(
		widget_class,
		GPluginGtkView,
		search_entry);

	gtk_widget_class_bind_template_child(
		widget_class,
		GPluginGtkView,
		plugin_page);

	gtk_widget_class_bind_template_callback(
		widget_class,
		gplugin_gtk_view_row_activated);
	gtk_widget_class_bind_template_callback(
		widget_class,
		gplugin_gtk_view_search_changed);
	gtk_widget_class_bind_template_callback(
		widget_class,
		gplugin_gtk_view_back_clicked_cb);
	gtk_widget_class_bind_template_callback(
		widget_class,
		gplugin_gtk_view_plugin_state_set_cb);

	/* properties */

	/**
	 * GPluginGtkView:manager:
	 *
	 * The plugin manager to display.
	 */
	properties[PROP_MANAGER] = g_param_spec_object(
		"manager",
		"manager",
		"The plugin manager to display",
		GPLUGIN_TYPE_MANAGER,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * GPluginGtkView:show-internal:
	 *
	 * Whether or not to show internal plugins.
	 */
	properties[PROP_SHOW_INTERNAL] = g_param_spec_boolean(
		"show-internal",
		"show-internal",
		"Whether or not to show internal plugins",
		FALSE,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * GPluginGtkView:settings-backend:
	 *
	 * The [class@Gio.SettingsBackend] to use when viewing plugin settings.
	 */
	properties[PROP_SETTINGS_BACKEND] = g_param_spec_object(
		"settings-backend",
		"settings-backend",
		"The settings backend to use when viewing plugin settings",
		G_TYPE_SETTINGS_BACKEND,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

static void
gplugin_gtk_view_init(GPluginGtkView *self)
{
	gtk_widget_init_template(GTK_WIDGET(self));

	gtk_list_box_set_filter_func(
		GTK_LIST_BOX(self->list_box),
		gplugin_gtk_view_filter_func,
		self,
		NULL);
	gtk_list_box_set_sort_func(
		GTK_LIST_BOX(self->list_box),
		gplugin_gtk_view_sort_func,
		NULL,
		NULL);
}

/******************************************************************************
 * API
 *****************************************************************************/

/**
 * gplugin_gtk_view_new:
 *
 * Creates a new [class@GPluginGtk4.View].
 *
 * Returns: (transfer full): The new view.
 */
GtkWidget *
gplugin_gtk_view_new(void)
{
	GObject *ret = NULL;

	/* clang-format off */
	ret = g_object_new(
		GPLUGIN_GTK_TYPE_VIEW,
		NULL);
	/* clang-format on */

	return GTK_WIDGET(ret);
}

/**
 * gplugin_gtk_view_set_manager:
 * @view: The GTK view instance.
 * @manager: The plugin manager to display.
 *
 * This function will set which plugin manager to display.
 */
void
gplugin_gtk_view_set_manager(GPluginGtkView *view, GPluginManager *manager)
{
	g_return_if_fail(GPLUGIN_GTK_IS_VIEW(view));
	g_return_if_fail(GPLUGIN_IS_MANAGER(manager));

	if(g_set_object(&view->manager, manager)) {
		GtkListBox *box = GTK_LIST_BOX(view->list_box);
		GtkWidget *child = NULL;

		while((child = gtk_widget_get_first_child(view->list_box)) != NULL) {
			gtk_list_box_remove(box, child);
		}

		gplugin_manager_foreach(
			manager,
			gplugin_gtk_view_add_plugins_to_list,
			view);

		g_object_notify(G_OBJECT(view), "manager");
	}
}

/**
 * gplugin_gtk_view_get_manager:
 * @view: The GTK view instance.
 *
 * Returns the plugin manager that is being displayed.
 *
 * Returns: (transfer none): The plugin manager to display.
 */
GPluginManager *
gplugin_gtk_view_get_manager(GPluginGtkView *view)
{
	g_return_val_if_fail(GPLUGIN_GTK_IS_VIEW(view), FALSE);

	return view->manager;
}

/**
 * gplugin_gtk_view_set_show_internal:
 * @view: The GTK view instance.
 * @show_internal: Whether or not to show internal plugins.
 *
 * This function will toggle whether or not the widget will show internal
 * plugins.
 */
void
gplugin_gtk_view_set_show_internal(GPluginGtkView *view, gboolean show_internal)
{
	g_return_if_fail(GPLUGIN_GTK_IS_VIEW(view));

	view->show_internal = show_internal;

	g_object_notify(G_OBJECT(view), "show-internal");
}

/**
 * gplugin_gtk_view_get_show_internal:
 * @view: The GTK view instance.
 *
 * Returns whether or not @view is showing internal plugins.
 */
gboolean
gplugin_gtk_view_get_show_internal(GPluginGtkView *view)
{
	g_return_val_if_fail(GPLUGIN_GTK_IS_VIEW(view), FALSE);

	return view->show_internal;
}

/**
 * gplugin_gtk_view_set_settings_backend:
 * @view: The GTK view instance.
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
gplugin_gtk_view_set_settings_backend(GPluginGtkView *view, gpointer backend)
{
	g_return_if_fail(GPLUGIN_GTK_IS_VIEW(view));
	g_return_if_fail(G_IS_SETTINGS_BACKEND(backend) || backend == NULL);

	if(g_set_object(&view->backend, backend)) {
		gplugin_gtk_plugin_page_set_settings_backend(
			GPLUGIN_GTK_PLUGIN_PAGE(view->plugin_page),
			view->backend);
		g_object_notify_by_pspec(
			G_OBJECT(view),
			properties[PROP_SETTINGS_BACKEND]);
	}
}

/**
 * gplugin_gtk_view_get_settings_backend:
 * @view: The GTK view instance.
 *
 * Gets the settings backend used when displaying plugin settings.
 *
 * Note, because we do not want to leak `G_SETTINGS_ENABLE_BACKEND` into
 * GPlugin users, this function returns a `gpointer`, and you should cast to
 * `GSettingsBackend *` after setting `G_SETTINGS_ENABLE_BACKEND` for the files
 * where you need it.
 *
 * Returns: (transfer none): The settings backend.
 *
 * Since: 0.40.0
 */
gpointer
gplugin_gtk_view_get_settings_backend(GPluginGtkView *view)
{
	g_return_val_if_fail(GPLUGIN_GTK_IS_VIEW(view), NULL);

	return view->backend;
}

/**
 * gplugin_gtk_view_show_overview:
 * @view: The GTK view instance.
 *
 * Shows the plugin list overview.
 *
 * Since: 0.39.0
 */
void
gplugin_gtk_view_show_overview(GPluginGtkView *view)
{
	g_return_if_fail(GPLUGIN_GTK_IS_VIEW(view));

	gplugin_gtk_plugin_page_set_plugin(
		GPLUGIN_GTK_PLUGIN_PAGE(view->plugin_page),
		NULL);
	gtk_stack_set_visible_child_name(GTK_STACK(view->stack), "overview");
}

/**
 * gplugin_gtk_view_show_plugin:
 * @view: The GTK view instance.
 * @plugin: (transfer none): The plugin to show.
 *
 * Shows a page for a single plugin.
 *
 * Since: 0.39.0
 */
void
gplugin_gtk_view_show_plugin(GPluginGtkView *view, GPluginPlugin *plugin)
{
	g_return_if_fail(GPLUGIN_GTK_IS_VIEW(view));
	g_return_if_fail(GPLUGIN_IS_PLUGIN(plugin));

	gplugin_gtk_plugin_page_set_plugin(
		GPLUGIN_GTK_PLUGIN_PAGE(view->plugin_page),
		plugin);
	gtk_stack_set_visible_child_name(GTK_STACK(view->stack), "plugin-page");
}
