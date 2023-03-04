IRCV3_SOURCES = [
	'purpleircv3capabilities.c',
	'purpleircv3connection.c',
	'purpleircv3core.c',
	'purpleircv3messagehandlers.c',
	'purpleircv3parser.c',
	'purpleircv3protocol.c',
	'purpleircv3protocolim.c',
	'purpleircv3sasl.c',
]

IRCV3_HEADERS = [
	'purpleircv3capabilities.h',
	'purpleircv3connection.h',
	'purpleircv3core.h',
	'purpleircv3messagehandlers.h',
	'purpleircv3parser.h',
	'purpleircv3protocol.h',
	'purpleircv3protocolim.h',
	'purpleircv3sasl.h',
]

if DYNAMIC_IRCV3
	ircv3_resources = gnome.compile_resources('ircv3resource',
		'resources/ircv3.gresource.xml',
		source_dir : 'resources',
		c_name : 'purple_ircv3')
	IRCV3_SOURCES += ircv3_resources

	ircv3_prpl = shared_library('ircv3', IRCV3_SOURCES + IRCV3_HEADERS,
		c_args : ['-DG_LOG_USE_STRUCTURED', '-DG_LOG_DOMAIN="Purple-IRCv3"'],
		dependencies : [libpurple_dep, glib, gio, hasl, ws2_32],
		install : true,
		install_dir : PURPLE_PLUGINDIR)

	devenv.append('PURPLE_PLUGIN_PATH', meson.current_build_dir())

	subdir('tests')
endif
