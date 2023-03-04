if x11.found()
	library('gntgf', 'gntgf.c',
	    c_args : [
	        ncurses_header,
	        '-DG_LOG_USE_STRUCTURED',
	        '-DG_LOG_DOMAIN="Gnt-GF"',
	    ],
	    dependencies : [x11, libpurple_dep, libfinch_dep, ncurses, glib],
	    name_prefix : '',
	    install : true, install_dir : FINCH_PLUGINDIR)
endif

settings_schemas = [
  'im.pidgin.Finch.plugin.GntGf.gschema.xml',
]

install_data(settings_schemas, install_dir: schemas_dir)
gnome.post_install(glib_compile_schemas: true)

# Compile the schemas in the current directory; this is only useful for testing
gnome.compile_schemas(depend_files: files(settings_schemas))

devenv.append('FINCH_PLUGIN_PATH', meson.current_build_dir())
