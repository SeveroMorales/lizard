xmppdisco_SOURCES = [
	'gtkdisco.c',
	'xmppdisco.c',
	'xmppdiscoservice.c',
]

xmppdisco_HEADERS = [
	'gtkdisco.h',
	'xmppdisco.h',
	'xmppdiscoservice.h',
]

xmppdisco_enums = gnome.mkenums_simple('xmppdiscoenums',
	sources: xmppdisco_HEADERS,
	identifier_prefix: 'XmppDisco',
	symbol_prefix: 'xmpp_disco')
xmppdisco_SOURCES += xmppdisco_enums

xmppdisco_resource = gnome.compile_resources('xmppdiscoresources',
    'resources/xmppdisco.gresource.xml',
    source_dir : 'resources',
    c_name : 'xmppdisco')
xmppdisco_SOURCES += xmppdisco_resource

xmppdisco = library('xmppdisco', xmppdisco_SOURCES, xmppdisco_HEADERS,
    c_args : ['-DG_LOG_USE_STRUCTURED', '-DG_LOG_DOMAIN="PidginPlugin-XMPPDisco"'],
    dependencies : [libpurple_dep, libpidgin_dep, glib],
    name_prefix : '',
    install : true, install_dir : PIDGIN_PLUGINDIR)

devenv.append('PIDGIN_PLUGIN_PATH', meson.current_build_dir())
