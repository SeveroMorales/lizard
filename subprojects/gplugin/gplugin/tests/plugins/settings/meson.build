shared_library('settings', 'settings.c',
               name_prefix: '', dependencies : [gplugin_dep, GLIB])
devenv.append('GPLUGIN_PLUGIN_PATH', meson.current_build_dir())

settings_schemas = [
  'im.pidgin.GPlugin.plugin.SettingsPlugin.gschema.xml',
]

# Compile the schemas in the current directory for testing.
gnome.compile_schemas(depend_files: files(settings_schemas))
