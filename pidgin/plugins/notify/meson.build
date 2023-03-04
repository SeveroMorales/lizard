notify_plugin_deps = [libpurple_dep, libpidgin_dep, glib]
if x11.found()
  notify_plugin_deps += x11
endif

notify = library('notify', 'notify.c',
    c_args : ['-DG_LOG_USE_STRUCTURED', '-DG_LOG_DOMAIN="PidginPlugin-Notify"'],
    dependencies : notify_plugin_deps,
    name_prefix : '',
    build_by_default: false,
    install : false, install_dir : PIDGIN_PLUGINDIR)

settings_schemas = [
  'im.pidgin.Pidgin.plugin.Notify.gschema.xml',
]

install_data(settings_schemas, install_dir: schemas_dir)
gnome.post_install(glib_compile_schemas: true)

# Compile the schemas in the current directory; this is only useful for testing
gnome.compile_schemas(depend_files: files(settings_schemas))

devenv.append('PIDGIN_PLUGIN_PATH', meson.current_build_dir())
