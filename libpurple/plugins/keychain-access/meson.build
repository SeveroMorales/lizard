if host_machine.system() == 'darwin'
	KEYCHAIN_ACCESS = dependency('appleframeworks',
	                             modules : ['CoreFoundation', 'Security'])

	keychain_access_plugin = library('keychain-access', 'keychain-access.c',
		c_args : ['-DG_LOG_USE_STRUCTURED', '-DG_LOG_DOMAIN="Purple-KeychainAccess"'],
		dependencies : [KEYCHAIN_ACCESS, libpurple_dep],
		name_prefix : '',
		install : true,
		install_dir : PURPLE_PLUGINDIR)

	devenv.append('PURPLE_PLUGIN_PATH', meson.current_build_dir())
endif
