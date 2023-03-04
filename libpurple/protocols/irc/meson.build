IRC_SOURCES = [
	'cmds.c',
	'dcc_send.c',
	'irc.c',
	'irc.h',
	'msgs.c',
	'parse.c'
]

if DYNAMIC_IRC
	irc_resources = gnome.compile_resources('ircresource',
		'resources/irc.gresource.xml',
		source_dir : 'resources',
		c_name : 'irc')
	IRC_SOURCES += irc_resources

	irc_prpl = shared_library('irc', IRC_SOURCES,
	    c_args : ['-DG_LOG_USE_STRUCTURED', '-DG_LOG_DOMAIN="Purple-IRC"'],
	    dependencies : [hasl, libpurple_dep, glib, gio, ws2_32],
	    install : true, install_dir : PURPLE_PLUGINDIR)

	devenv.append('PURPLE_PLUGIN_PATH', meson.current_build_dir())
endif
