/*
 * Copyright (C) 2011-2022 Gary Kramlich <grim@reaperworld.com>
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

#include <gplugin/gplugin-file-source.h>

#include <gplugin/gplugin-file-tree.h>
#include <gplugin/gplugin-private.h>

/**
 * GPluginFileSource:
 *
 * A [iface@GPlugin.Source] that will query plugins on disk.
 *
 * Since: 0.39.0
 */

struct _GPluginFileSource {
	GObject parent;

	GPluginManager *manager;

	GHashTable *plugin_filenames;
	GHashTable *loaders_by_extension;
	GNode *root;

	GList *error_messages;
};

enum {
	PROP_ZERO,
	PROP_MANAGER,
	N_PROPERTIES,
};
static GParamSpec *properties[N_PROPERTIES] = {
	NULL,
};

/******************************************************************************
 * Helpers
 *****************************************************************************/
static guint
gplugin_file_source_str_hash(gconstpointer v)
{
	if(v == NULL) {
		return g_str_hash("");
	}

	return g_str_hash(v);
}

static void
gplugin_file_source_set_manager(
	GPluginFileSource *source,
	GPluginManager *manager)
{
	if(g_set_object(&source->manager, manager)) {
		g_object_notify_by_pspec(G_OBJECT(source), properties[PROP_MANAGER]);
	}
}

static GPluginManager *
gplugin_file_source_get_manager(GPluginFileSource *source)
{
	return source->manager;
}

static void
gplugin_file_source_add_loader(GPluginFileSource *source, GPluginLoader *loader)
{
	GSList *exts = NULL;
	const gchar *loader_id = NULL;

	loader_id = gplugin_loader_get_id(loader);

	exts = gplugin_loader_get_supported_extensions(loader);
	while(exts != NULL) {
		GSList *loaders = NULL;
		const gchar *extension = exts->data;

		/* Grab any existing loaders that are registered for this extension so
		 * that we can prepend our loader. But before we add ours, we remove any
		 * old copies we might have of ours.
		 */
		loaders = g_hash_table_lookup(source->loaders_by_extension, extension);
		for(GSList *ll = loaders; ll != NULL; ll = ll->next) {
			GPluginLoader *existing = ll->data;
			const gchar *existing_id = gplugin_loader_get_id(existing);

			if(g_str_equal(loader_id, existing_id)) {
				loaders = g_slist_remove(loaders, existing);

				g_clear_object(&existing);

				break;
			}
		}

		loaders = g_slist_prepend(loaders, g_object_ref(loader));

		/* Now insert the updated slist back into the hash table */
		g_hash_table_insert(
			source->loaders_by_extension,
			g_strdup(extension),
			loaders);

		/* Move exts to the next one. */
		exts = g_slist_delete_link(exts, exts);
	}
}

static void
gplugin_file_source_remove_loader(
	GPluginFileSource *source,
	GPluginLoader *loader)
{
	GSList *exts = NULL;
	const gchar *loader_id = NULL;

	loader_id = gplugin_loader_get_id(loader);

	exts = gplugin_loader_get_supported_extensions(loader);
	while(exts != NULL) {
		GSList *loaders = NULL;
		const gchar *extension = exts->data;

		loaders = g_hash_table_lookup(source->loaders_by_extension, extension);
		for(GSList *ll = loaders; ll != NULL; ll = ll->next) {
			GPluginLoader *existing = ll->data;
			const gchar *existing_id = gplugin_loader_get_id(existing);

			if(g_str_equal(loader_id, existing_id)) {
				loaders = g_slist_remove(loaders, existing);

				if(loaders == NULL) {
					g_hash_table_remove(
						source->loaders_by_extension,
						extension);
				} else {
					g_hash_table_insert(
						source->loaders_by_extension,
						g_strdup(extension),
						loaders);
				}

				g_clear_object(&existing);

				break;
			}
		}

		/* Move exts to the next one. */
		exts = g_slist_delete_link(exts, exts);
	}
}

static void
gplugin_file_source_update_loaders(GPluginFileSource *source)
{
	GList *loaders = NULL;

	loaders = gplugin_manager_get_loaders(source->manager);
	while(loaders != NULL) {
		GPluginLoader *loader = loaders->data;

		gplugin_file_source_add_loader(source, loader);

		loaders = g_list_delete_link(loaders, loaders);
	}
}

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static void
gplugin_file_source_loader_registered_cb(
	G_GNUC_UNUSED GPluginManager *manager,
	GPluginLoader *loader,
	gpointer data)
{
	gplugin_file_source_add_loader(data, loader);
}

static void
gplugin_file_source_loader_unregistered_cb(
	G_GNUC_UNUSED GPluginManager *manager,
	GPluginLoader *loader,
	gpointer data)
{
	gplugin_file_source_remove_loader(data, loader);
}

/******************************************************************************
 * GPluginSource implementation
 *****************************************************************************/
static gboolean
gplugin_file_source_scan(GPluginSource *source)
{
	GPluginFileSource *file_source = GPLUGIN_FILE_SOURCE(source);
	gboolean refresh = FALSE;
	gint errors = 0;

	/* Clear any error messages from our last scan. */
	g_list_free_full(file_source->error_messages, (GDestroyNotify)g_free);
	file_source->error_messages = NULL;

	for(GNode *dir = file_source->root->children; dir; dir = dir->next) {
		GPluginFileTreeEntry *e = dir->data;
		GNode *file = NULL;
		const gchar *path = e->filename;

		for(file = dir->children; file; file = file->next) {
			GPluginPlugin *plugin = NULL;
			GPluginLoader *loader = NULL;
			GError *error = NULL;
			GSList *l = NULL;
			gchar *filename = NULL;

			e = (GPluginFileTreeEntry *)file->data;

			/* Build the path and see if we need to probe it! */
			filename = g_build_filename(path, e->filename, NULL);
			plugin =
				g_hash_table_lookup(file_source->plugin_filenames, filename);

			if(plugin && GPLUGIN_IS_PLUGIN(plugin)) {
				GPluginPluginState state = gplugin_plugin_get_state(plugin);

				/* The plugin is in our "view", check its state.  If it's
				 * queried or loaded, move on to the next one.
				 */
				if(state == GPLUGIN_PLUGIN_STATE_QUERIED ||
				   state == GPLUGIN_PLUGIN_STATE_LOADED) {
					g_free(filename);
					continue;
				}
			}

			/* grab the list of loaders for this extension */
			l = g_hash_table_lookup(
				file_source->loaders_by_extension,
				e->extension);
			for(; l; l = l->next) {
				if(!GPLUGIN_IS_LOADER(l->data)) {
					continue;
				}

				loader = GPLUGIN_LOADER(l->data);

				/* Try to probe the plugin with the current loader */
				plugin = gplugin_loader_query_plugin(loader, filename, &error);

				/* Check the GError, if it's set, output its message and
				 * try the next loader.
				 */
				if(error) {
					gchar *error_message = NULL;

					errors++;

					error_message = g_strdup_printf(
						"failed to query '%s' with loader '%s': %s",
						filename,
						G_OBJECT_TYPE_NAME(loader),
						error->message);

					file_source->error_messages = g_list_prepend(
						file_source->error_messages,
						error_message);

					g_clear_error(&error);
					g_clear_object(&plugin);

					loader = NULL;

					continue;
				}

				/* if the plugin instance is good, then break out of this
				 * loop.
				 */
				if(GPLUGIN_IS_PLUGIN(plugin)) {
					break;
				}

				g_object_unref(G_OBJECT(plugin));

				loader = NULL;
			}

			/* check if our plugin instance is good.  If it's not good we
			 * don't need to do anything but free the filename which we'll
			 * do later.
			 */
			if(GPLUGIN_IS_PLUGIN(plugin)) {
				/* We have a good plugin, huzzah! We need to add it to our hash
				 * table.
				 */

				gchar *real_filename = gplugin_plugin_get_filename(plugin);

				/* We also need the GPluginPluginInfo for a bunch of stuff. */
				GPluginPluginInfo *info = gplugin_plugin_get_info(plugin);

				const gchar *id = gplugin_plugin_info_get_id(info);
				GSList *l = NULL, *ll = NULL;
				gboolean seen = FALSE;

				/* Throw a warning if the info->id is NULL. */
				if(id == NULL) {
					gchar *error_message = NULL;

					error_message = g_strdup_printf(
						"plugin %s has a NULL id",
						real_filename);

					g_free(real_filename);
					g_object_unref(G_OBJECT(info));

					file_source->error_messages = g_list_prepend(
						file_source->error_messages,
						error_message);

					continue;
				}

				/* Now insert into our hash table. */
				g_hash_table_replace(
					file_source->plugin_filenames,
					real_filename,
					g_object_ref(G_OBJECT(plugin)));

				/* Grab the list of plugins with our id and prepend the new
				 * plugin to it before updating it.
				 */
				l = gplugin_manager_find_plugins(file_source->manager, id);
				for(ll = l; ll; ll = ll->next) {
					GPluginPlugin *splugin = GPLUGIN_PLUGIN(ll->data);
					gchar *sfilename = gplugin_plugin_get_filename(splugin);

					if(!g_strcmp0(real_filename, sfilename)) {
						seen = TRUE;
					}

					g_free(sfilename);
				}
				g_slist_free_full(l, g_object_unref);
				if(!seen) {
					gplugin_manager_add_plugin(
						file_source->manager,
						id,
						plugin);
				}

				/* Check if the plugin is supposed to be loaded on query, and
				 * if so, load it.
				 */
				if(gplugin_plugin_info_get_auto_load(info)) {
					GError *error = NULL;
					gboolean loaded;

					loaded = gplugin_loader_load_plugin(loader, plugin, &error);

					if(!loaded) {
						gchar *error_message = NULL;

						error_message = g_strdup_printf(
							"failed to load %s during query: %s",
							filename,
							(error) ? error->message : "unknown");

						file_source->error_messages = g_list_prepend(
							file_source->error_messages,
							error_message);

						errors++;

						g_clear_error(&error);
					}
				} else {
					if(errors > 0) {
						refresh = TRUE;
					}
				}

				g_object_unref(G_OBJECT(info));

				/* Since the plugin is now stored in our hash tables we need to
				 * remove this function's reference to it.
				 */
				g_object_unref(G_OBJECT(plugin));
			}

			g_free(filename);
		}
	}

	return refresh;
}

static void
gplugin_file_source_source_iface_init(GPluginSourceInterface *iface)
{
	iface->scan = gplugin_file_source_scan;
}

/******************************************************************************
 * GObject implementation
 *****************************************************************************/
G_DEFINE_TYPE_WITH_CODE(
	GPluginFileSource,
	gplugin_file_source,
	G_TYPE_OBJECT,
	G_IMPLEMENT_INTERFACE(
		GPLUGIN_TYPE_SOURCE,
		gplugin_file_source_source_iface_init))

static void
gplugin_file_source_get_property(
	GObject *obj,
	guint param_id,
	GValue *value,
	GParamSpec *pspec)
{
	GPluginFileSource *source = GPLUGIN_FILE_SOURCE(obj);

	switch(param_id) {
		case PROP_MANAGER:
			g_value_set_object(value, gplugin_file_source_get_manager(source));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
gplugin_file_source_set_property(
	GObject *obj,
	guint param_id,
	const GValue *value,
	GParamSpec *pspec)
{
	GPluginFileSource *source = GPLUGIN_FILE_SOURCE(obj);

	switch(param_id) {
		case PROP_MANAGER:
			gplugin_file_source_set_manager(source, g_value_get_object(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
	}
}

static void
gplugin_file_source_dispose(GObject *obj)
{
	GPluginFileSource *source = GPLUGIN_FILE_SOURCE(obj);

	g_clear_object(&source->manager);

	G_OBJECT_CLASS(gplugin_file_source_parent_class)->dispose(obj);
}

static void
gplugin_file_source_slist_unref(gpointer data)
{
	GSList *slist;

	slist = data;
	g_slist_free_full(slist, g_object_unref);
}

static void
gplugin_file_source_constructed(GObject *obj)
{
	GPluginFileSource *source = GPLUGIN_FILE_SOURCE(obj);
	GList *paths = NULL;

	G_OBJECT_CLASS(gplugin_file_source_parent_class)->constructed(obj);

	/* The plugin_filenames hash table is keyed on the filename of the plugin
	 * with a value of the plugin itself.
	 */
	source->plugin_filenames =
		g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_object_unref);

	/* The loaders_by_extension hash table is keyed on the supported extensions
	 * of the loader.  Which means that a loader that supports multiple
	 * extensions will be in the table multiple times.
	 *
	 * We deal with collisions by using a GSList for the value which will hold
	 * references to instances of the actual loaders.
	 *
	 * Storing this in this method allows us to quickly figure out which loader
	 * to use by the filename and helps us to avoid iterating the loaders table
	 * again and again.
	 */
	source->loaders_by_extension = g_hash_table_new_full(
		gplugin_file_source_str_hash,
		g_str_equal,
		g_free,
		gplugin_file_source_slist_unref);

	/* Connect to the loader-registered and loader-unregistered signals so we
	 * can keep our loaders_by_extension hash table up to date.
	 */
	g_signal_connect_object(
		source->manager,
		"loader-registered",
		G_CALLBACK(gplugin_file_source_loader_registered_cb),
		source,
		0);
	g_signal_connect_object(
		source->manager,
		"loader-unregistered",
		G_CALLBACK(gplugin_file_source_loader_unregistered_cb),
		source,
		0);

	gplugin_file_source_update_loaders(source);

	/* Get the paths from the manager and create our initial file tree. */
	paths = gplugin_manager_get_paths(source->manager);
	source->root = gplugin_file_tree_new(paths);
}

static void
gplugin_file_source_finalize(GObject *obj)
{
	GPluginFileSource *source = GPLUGIN_FILE_SOURCE(obj);

	g_clear_pointer(&source->root, gplugin_file_tree_free);
	g_clear_pointer(&source->plugin_filenames, g_hash_table_destroy);
	g_clear_pointer(&source->loaders_by_extension, g_hash_table_destroy);

	while(source->error_messages != NULL) {
		gchar *error_message = source->error_messages->data;

		g_warning("%s", error_message);

		g_free(error_message);

		source->error_messages =
			g_list_delete_link(source->error_messages, source->error_messages);
	}

	G_OBJECT_CLASS(gplugin_file_source_parent_class)->finalize(obj);
}

static void
gplugin_file_source_init(G_GNUC_UNUSED GPluginFileSource *source)
{
}

static void
gplugin_file_source_class_init(GPluginFileSourceClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->get_property = gplugin_file_source_get_property;
	obj_class->set_property = gplugin_file_source_set_property;
	obj_class->constructed = gplugin_file_source_constructed;
	obj_class->dispose = gplugin_file_source_dispose;
	obj_class->finalize = gplugin_file_source_finalize;

	/**
	 * GPluginFileSource::manager:
	 *
	 * The [class@GPlugin.Manager] that this source is working for.
	 *
	 * Since: 0.39.0
	 */
	properties[PROP_MANAGER] = g_param_spec_object(
		"manager",
		"manager",
		"The manager this source is working for.",
		GPLUGIN_TYPE_MANAGER,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

/******************************************************************************
 * Public API
 *****************************************************************************/

/**
 * gplugin_file_source_new:
 * @manager: The [class@GPlugin.Manager] instance.
 *
 * Creates a [iface@GPlugin.Source] that will query plugins on disk using the
 * paths from @manager.
 *
 * Returns: (transfer full): The new source.
 *
 * Since: 0.39.0
 */
GPluginSource *
gplugin_file_source_new(GPluginManager *manager)
{
	g_return_val_if_fail(GPLUGIN_IS_MANAGER(manager), NULL);

	return g_object_new(GPLUGIN_TYPE_FILE_SOURCE, "manager", manager, NULL);
}
