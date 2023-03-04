if not get_option('vapi')
  subdir_done()
endif

if not get_option('introspection')
	error('Vala generation requires GObject Introspection.')
endif

add_languages('vala', native: false)

gplugin_vapi = gnome.generate_vapi('gplugin',
	sources : gplugin_gir[0],
	install : true,
)

if get_option('gtk4').allowed()
	gplugin_gtk4_vapi = gnome.generate_vapi('gplugin-gtk4',
		sources : gplugin_gtk4_gir[0],
		packages : [ 'gtk4' ],
		install : true,
		gir_dirs : meson.current_build_dir() / '..' / 'gplugin',
	)
endif

subdir('tests')
