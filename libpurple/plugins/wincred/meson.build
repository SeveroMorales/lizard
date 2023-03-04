if IS_WIN32
	wincred_plugin = library('wincred', 'wincred.c',
	    c_args : ['-DG_LOG_USE_STRUCTURED', '-DG_LOG_DOMAIN="Purple-WinCredKeyring"'],
	    dependencies : [libpurple_dep],
	    name_prefix : '',
	    install : true, install_dir : PURPLE_PLUGINDIR)

	devenv.append('PURPLE_PLUGIN_PATH', meson.current_build_dir())
endif
