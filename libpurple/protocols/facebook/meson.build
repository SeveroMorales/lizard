FACEBOOK_SOURCES = [
	'api.c',
	'api.h',
	'data.c',
	'data.h',
	'facebook.h',
	'facebook.c',
	'http.c',
	'http.h',
	'id.h',
	'json.c',
	'json.h',
	'mqtt.c',
	'mqtt.h',
	'thrift.c',
	'thrift.h',
	'util.c',
	'util.h'
]

FACEBOOK_BUILT_SOURCES = []

if DYNAMIC_FACEBOOK
	facebook_resources = gnome.compile_resources('facebookresource',
		'resources/facebook.gresource.xml',
		source_dir : 'resources',
		c_name : 'facebook')
	FACEBOOK_BUILT_SOURCES += facebook_resources

	facebook_prpl = shared_library('facebook',
	    FACEBOOK_SOURCES + FACEBOOK_BUILT_SOURCES,
	    c_args : ['-DG_LOG_USE_STRUCTURED', '-DG_LOG_DOMAIN="Purple-Facebook"'],
	    dependencies : [json, libpurple_dep, libsoup, glib],
	    install : true,
	    install_dir : PURPLE_PLUGINDIR)

	# Used to produce docs.
	facebook_inc = include_directories('.')
	facebook_dep = declare_dependency(
	    link_with : facebook_prpl,
	    dependencies : [json, libpurple_dep, glib])

	devenv.append('PURPLE_PLUGIN_PATH', meson.current_build_dir())

	if enable_introspection
		introspection_sources = FACEBOOK_SOURCES

		facebook_gir_includes = ['GObject-2.0', 'Gio-2.0', 'Gst-1.0',
		                         'Json-1.0', 'Soup-3.0', libpurple_gir[0]]

		facebook_gir = gnome.generate_gir(facebook_prpl,
		    sources : introspection_sources,
		    includes : facebook_gir_includes,
		    namespace : 'Facebook',
		    symbol_prefix : 'fb',
		    identifier_prefix : 'Fb',
		    nsversion : f'@purple_major_version@.@purple_minor_version@',
		    dependencies: [gplugin_dep, libpurple_dep],
		    install : false,
		    extra_args : ['-DPURPLE_COMPILATION', '--quiet'])
	endif

endif
