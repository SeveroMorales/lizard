/*
 * Copyright (C) 2011-2021 Gary Kramlich <grim@reaperworld.com>
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

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "gplugin-python3-loader.h"

#include <glib/gi18n-lib.h>

#include <pygobject.h>

#include "gplugin-python3-plugin.h"
#include "gplugin-python3-utils.h"

struct _GPluginPython3Loader {
	GPluginLoader parent;
};

G_DEFINE_DYNAMIC_TYPE(
	GPluginPython3Loader,
	gplugin_python3_loader,
	GPLUGIN_TYPE_LOADER)

/******************************************************************************
 * GPluginLoader Implementation
 *****************************************************************************/
static GSList *
gplugin_python3_loader_supported_extensions(G_GNUC_UNUSED GPluginLoader *l)
{
	return g_slist_append(NULL, "py");
}

static GPluginPlugin *
gplugin_python3_loader_query(
	GPluginLoader *loader,
	const gchar *filename,
	G_GNUC_UNUSED GError **error)
{
	GPluginPlugin *plugin = NULL;
	GObject *info = NULL;
	PyObject *pyinfo = NULL, *args = NULL;
	PyObject *module = NULL, *package_list = NULL, *module_dict = NULL;
	PyObject *query = NULL, *load = NULL, *unload = NULL;
	PyGILState_STATE state;
	gchar *module_name = NULL, *dir_name = NULL;

	/* lock the gil */
	state = pyg_gil_state_ensure();

	/* create package_list as a tuple to handle 'import foo.bar' */
	package_list = PyTuple_New(0);

	/* now figure out the module name from the filename */
	module_name = gplugin_python3_filename_to_module(filename);

	/* grab the dirname since we need it on sys.path to import the module */
	dir_name = g_path_get_dirname(filename);
	gplugin_python3_add_module_path(dir_name);
	g_free(dir_name);

	/* import the module */
	module = PyImport_ImportModuleEx(module_name, NULL, NULL, package_list);
	if(PyErr_Occurred()) {
		g_warning(_("Failed to query %s"), filename);

		if(error != NULL) {
			*error = gplugin_python3_exception_to_gerror();
		}

		/* clean some stuff up */
		g_free(module_name);
		Py_DECREF(package_list);

		pyg_gil_state_release(state);

		return NULL;
	}

	/* clean some stuff up */
	g_free(module_name);
	Py_DECREF(package_list);

	/* at this point we have the module, lets find the query, load, and unload
	 * functions.
	 */
	module_dict = PyModule_GetDict(module);

	query = PyDict_GetItemString(module_dict, "gplugin_query");
	if(query == NULL) {
		g_warning(
			_("Failed to find the gplugin_query function in %s"),
			filename);

		Py_DECREF(module);
		pyg_gil_state_release(state);

		return NULL;
	}
	if(!PyCallable_Check(query)) {
		g_warning(
			_("Found gplugin_query in %s but it is not a "
			  "function"),
			filename);

		Py_DECREF(module);
		pyg_gil_state_release(state);

		return NULL;
	}

	load = PyDict_GetItemString(module_dict, "gplugin_load");
	if(load == NULL) {
		g_warning(
			_("Failed to find the gplugin_load function in %s"),
			filename);

		Py_DECREF(module);
		pyg_gil_state_release(state);

		return NULL;
	}
	if(!PyCallable_Check(load)) {
		g_warning(
			_("Found gplugin_load in %s but it is not a "
			  "function"),
			filename);

		Py_DECREF(module);
		pyg_gil_state_release(state);

		return NULL;
	}

	unload = PyDict_GetItemString(module_dict, "gplugin_unload");
	if(unload == NULL) {
		g_warning(
			_("Failed to find the gplugin_unload function in %s"),
			filename);

		Py_DECREF(module);
		pyg_gil_state_release(state);

		return NULL;
	}
	if(!PyCallable_Check(unload)) {
		g_warning(
			_("Found gplugin_unload in %s but it is not a "
			  "function"),
			filename);

		Py_DECREF(module);
		pyg_gil_state_release(state);

		return NULL;
	}

	/* now that we have everything, call the query method and get the plugin's
	 * info.
	 */
	args = PyTuple_New(0);
	pyinfo = PyObject_Call(query, args, NULL);
	Py_DECREF(args);

	info = pygobject_get(pyinfo);

	/* now that we have everything, create the plugin */
	/* clang-format off */
	plugin = g_object_new(
		GPLUGIN_PYTHON3_TYPE_PLUGIN,
		"filename", filename,
		"loader", loader,
		"module", module,
		"info", info,
		"load-func", load,
		"unload-func", unload,
		NULL);
	/* clang-format on */

	Py_DECREF(pyinfo);
	Py_DECREF(module);

	/* unlock the gil */
	pyg_gil_state_release(state);

	return plugin;
}

static gboolean
gplugin_python3_loader_load(
	G_GNUC_UNUSED GPluginLoader *loader,
	GPluginPlugin *plugin,
	GError **error)
{
	PyObject *load = NULL, *pyplugin = NULL, *result = NULL;
	gboolean ret = FALSE;

	g_object_get(G_OBJECT(plugin), "load-func", &load, NULL);

	pyplugin = pygobject_new(G_OBJECT(plugin));

	result = PyObject_CallFunctionObjArgs(load, pyplugin, NULL);
	Py_DECREF(pyplugin);

	if(PyErr_Occurred()) {
		Py_XDECREF(result);

		if(error != NULL) {
			*error = gplugin_python3_exception_to_gerror();
		}

		return FALSE;
	}

	ret = PyObject_IsTrue(result);
	Py_DECREF(result);

	if(!ret) {
		g_set_error_literal(
			error,
			GPLUGIN_DOMAIN,
			0,
			_("Failed to load plugin"));
	}

	return ret;
}

static gboolean
gplugin_python3_loader_unload(
	G_GNUC_UNUSED GPluginLoader *loader,
	GPluginPlugin *plugin,
	gboolean shutdown,
	GError **error)
{
	PyObject *unload = NULL, *result = NULL;
	PyObject *pyplugin = NULL, *pyshutdown = NULL;
	gboolean ret = FALSE;

	g_object_get(G_OBJECT(plugin), "unload-func", &unload, NULL);

	pyplugin = pygobject_new(G_OBJECT(plugin));
	pyshutdown = PyBool_FromLong(shutdown);

	result = PyObject_CallFunctionObjArgs(unload, pyplugin, pyshutdown, NULL);
	Py_DECREF(pyplugin);
	Py_DECREF(pyshutdown);

	if(PyErr_Occurred()) {
		Py_XDECREF(result);

		if(error != NULL) {
			*error = gplugin_python3_exception_to_gerror();
		}

		return FALSE;
	}

	ret = PyObject_IsTrue(result);
	Py_DECREF(result);

	if(!ret) {
		g_set_error_literal(
			error,
			GPLUGIN_DOMAIN,
			0,
			_("Failed to unload plugin"));
	}

	return ret;
}

/******************************************************************************
 * Python3 Stuff
 *****************************************************************************/
static gboolean
gplugin_python3_loader_init_pygobject(void)
{
	pygobject_init(3, 0, 0);
	if(PyErr_Occurred()) {
		PyObject *type = NULL, *value = NULL, *tb = NULL, *obj = NULL;

		PyErr_Fetch(&type, &value, &tb);
		Py_DECREF(type);
		Py_XDECREF(tb);

		obj = PyUnicode_AsUTF8String(value);
		Py_DECREF(value);

		g_warning("Failed to initialize PyGObject : %s", PyBytes_AsString(obj));
		Py_DECREF(obj);

		return FALSE;
	}

	/* enable threads */
	pyg_enable_threads();

	/* disable g_log redirections */
	pyg_disable_warning_redirections();

	return TRUE;
}

static gboolean
gplugin_python3_loader_init_gettext(void)
{
	PyObject *module_dict = NULL, *install = NULL;
	PyObject *gettext = NULL, *result = NULL;

	gettext = PyImport_ImportModule("gettext");
	if(gettext == NULL) {
		g_warning("Failed to import gettext");

		return FALSE;
	}

	module_dict = PyModule_GetDict(gettext);
	install = PyDict_GetItemString(module_dict, "install");
	result = PyObject_CallFunction(install, "ss", GETTEXT_PACKAGE, LOCALEDIR);
	Py_XDECREF(result);
	Py_DECREF(gettext);

	return TRUE;
}

static gboolean
gplugin_python3_loader_init_python(void)
{
#if PY_VERSION_HEX >= 0x03080000
	PyConfig config;
	PyStatus status;

	PyConfig_InitIsolatedConfig(&config);
	status =
		PyConfig_SetBytesString(&config, &config.program_name, g_get_prgname());
	if(PyStatus_Exception(status)) {
		g_warning("Could not convert program name to wchar_t string.");
		PyConfig_Clear(&config);
		return FALSE;
	}

	status = Py_InitializeFromConfig(&config);
	PyConfig_Clear(&config);

	if(PyStatus_Exception(status)) {
		g_warning("Could not initialize Python.");
		return FALSE;
	}
#else  /* PY_VERSION_HEX >= 0x03080000 */
	wchar_t *argv[] = {NULL, NULL};

	/* Initializes Python3 */
	if(!Py_IsInitialized()) {
		Py_InitializeEx(FALSE);
	}

	argv[0] = Py_DecodeLocale(g_get_prgname(), NULL);
	if(argv[0] == NULL) {
		g_warning("Could not convert program name to wchar_t string.");
		return FALSE;
	}

	/* setup sys.path according to
	 * https://docs.python.org/3/c-api/init.html#PySys_SetArgvEx
	 */
	PySys_SetArgvEx(1, argv, 0);
	PyMem_RawFree(argv[0]);
#endif /* PY_VERSION_HEX >= 0x03080000 */

	/* initialize pygobject */
	if(gplugin_python3_loader_init_pygobject()) {
		if(gplugin_python3_loader_init_gettext()) {
			return TRUE;
		}
	}

	return FALSE;
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
gplugin_python3_loader_init(G_GNUC_UNUSED GPluginPython3Loader *loader)
{
}

static void
gplugin_python3_loader_class_finalize(
	G_GNUC_UNUSED GPluginPython3LoaderClass *klass)
{
}

static void
gplugin_python3_loader_class_init(GPluginPython3LoaderClass *klass)
{
	GPluginLoaderClass *loader_class = GPLUGIN_LOADER_CLASS(klass);

	loader_class->supported_extensions =
		gplugin_python3_loader_supported_extensions;
	loader_class->query = gplugin_python3_loader_query;
	loader_class->load = gplugin_python3_loader_load;
	loader_class->unload = gplugin_python3_loader_unload;
}

/******************************************************************************
 * API
 *****************************************************************************/
void
gplugin_python3_loader_register(GTypeModule *module)
{
	gplugin_python3_loader_register_type(module);

	gplugin_python3_loader_init_python();
}

GPluginLoader *
gplugin_python3_loader_new(void)
{
	/* clang-format off */
	return GPLUGIN_LOADER(g_object_new(
		GPLUGIN_PYTHON3_TYPE_LOADER,
		"id", "gplugin-python3",
		NULL));
	/* clang-format on */
}
