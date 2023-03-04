if not get_option('vapi')
	subdir_done()
endif

if not get_option('introspection')
	error('Vala generation requires GObject Introspection.')
endif

add_languages('vala', native : false)

talkatu_vapi = gnome.generate_vapi('talkatu',
	sources : talkatu_gir[0],
	packages : [ 'gtk4' ],
	install : true,
	gir_dirs : meson.current_build_dir() / '..' / 'talkatu',
)
