if not get_option('install-gplugin-gtk4-viewer')
summary('gplugin-gtk4-viewer',
        'You have disabled installation of gplugin-gtk4-viewer which is a ' +
        'very useful diagnostics tool.',
        section : 'Warnings')
endif

gplugin_gtk4_viewer_resources = gnome.compile_resources(
	'gplugin-gtk-viewer-resources',
	'data/gplugin-gtk-viewer.gresource.xml',
	c_name: 'gplugin_gtk_viewer',
	source_dir: 'data')

gplugin_gtk4_viewer = executable('gplugin-gtk4-viewer',
	'gplugin-gtk-viewer.c',
	'gplugin-gtk-viewer-window.c',
	gplugin_gtk4_viewer_resources,
	dependencies : [gplugin_dep, gplugin_gtk4_dep],
	install : get_option('install-gplugin-gtk4-viewer')
)

if get_option('help2man')
	gplugin_gtk_viewer_man = custom_target('gplugin-gtk4-viewer.1',
		command : [help2man,
		           '--name=View installed plugins', '--section=1',
		           '--help-option=--help-all', '--no-info',
		           '--output', '@OUTPUT@',
		           gplugin_gtk4_viewer],
		output : 'gplugin-gtk4-viewer.1',
		install : get_option('install-gplugin-gtk4-viewer'),
		install_dir : get_option('mandir') / 'man1')
endif
