if x11.found()
	library('gntclipboard', 'gntclipboard.c',
	    c_args : ['-DG_LOG_USE_STRUCTURED', '-DG_LOG_DOMAIN="Gnt-Clipboard"'],
	    dependencies : [x11, libpurple_dep, libfinch_dep, glib],
	    name_prefix : '',
	    install : true, install_dir : FINCH_PLUGINDIR)
endif

devenv.append('FINCH_PLUGIN_PATH', meson.current_build_dir())
