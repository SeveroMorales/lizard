if host_machine.system() != 'windows'
	CANBERRA = dependency('libcanberra', version: '>= 0.30')

	library('notification-sound', 'notification-sound.c',
		c_args : ['-DG_LOG_USE_STRUCTURED', '-DG_LOG_DOMAIN="PurplePlugin-NotificationSound"'],
		dependencies : [libpurple_dep, glib, CANBERRA],
		name_prefix : '',
		install : true, install_dir : PURPLE_PLUGINDIR)

	devenv.append('PURPLE_PLUGIN_PATH', meson.current_build_dir())

	settings_schemas = [
	  'im.pidgin.Purple.plugin.NotificationSound.gschema.xml',
	]

	install_data(settings_schemas, install_dir: schemas_dir)
	gnome.post_install(glib_compile_schemas: true)

	# Compile the schemas in the current directory; this is only useful for testing
	gnome.compile_schemas(depend_files: files(settings_schemas))
endif
