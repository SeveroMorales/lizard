GTK4 = dependency('gtk4', version : '>=4.0.0', required : get_option('gtk4'))
if not GTK4.found()
  subdir_done()
endif

###############################################################################
# Build Info
###############################################################################

GPLUGIN_GTK4_LIBRARY_VERSION = '0.1.0'

GPLUGIN_GTK4_SOURCES = [
	'gplugin-gtk-plugin-page.c',
	'gplugin-gtk-plugin-row.c',
	'gplugin-gtk-plugin-settings-list.c',
	'gplugin-gtk-view.c',
]

GPLUGIN_GTK4_HEADERS = [
	'gplugin-gtk-plugin-page.h',
	'gplugin-gtk-plugin-row.h',
	'gplugin-gtk-plugin-settings-list.h',
	'gplugin-gtk-view.h',
]

GPLUGIN_GTK4_PRIVATE_SOURCES = [
	'gplugin-gtk-plugin-closures.c',
]

GPLUGIN_GTK4_PRIVATE_HEADERS = [
	'gplugin-gtk-plugin-closures.h',
]

GPLUGIN_GTK4_PUBLIC_BUILT_SOURCES = [
]

GPLUGIN_GTK4_PUBLIC_BUILT_HEADERS = [
]

GPLUGIN_GTK4_GENERATED_TARGETS = [
]

###############################################################################
# Configure Files
###############################################################################
GPLUGIN_GTK4_H_INCLUDES = []
foreach header : GPLUGIN_GTK4_HEADERS
	GPLUGIN_GTK4_H_INCLUDES += f'#include <gplugin-gtk4/@header@>'
endforeach

conf = configuration_data()
conf.set('GPLUGIN_GTK_H_INCLUDES', '\n'.join(GPLUGIN_GTK4_H_INCLUDES))
gplugin_gtk4_h = configure_file(
	input : 'gplugin-gtk.h.in',
	output : 'gplugin-gtk.h',
	configuration : conf,
	install : true,
	install_dir : get_option('includedir') / 'gplugin-gtk4-1.0')

###############################################################################
# Resource Target
###############################################################################
gplugin_gtk4_resources = gnome.compile_resources(
	'gplugin-gtk4-resources',
	'data/gplugin-gtk.gresource.xml',
	c_name: 'gplugin_gtk',
	source_dir: 'data')

###############################################################################
# Library Target
###############################################################################
gplugin_gtk4 = library('gplugin-gtk4',
	GPLUGIN_GTK4_SOURCES,
	GPLUGIN_GTK4_HEADERS,
	GPLUGIN_GTK4_PRIVATE_SOURCES,
	GPLUGIN_GTK4_PRIVATE_HEADERS,
	GPLUGIN_GTK4_PUBLIC_BUILT_SOURCES,
	GPLUGIN_GTK4_PUBLIC_BUILT_HEADERS,
	gplugin_gtk4_resources,
	c_args : ['-DGPLUGIN_GTK_COMPILATION', '-DG_LOG_USE_STRUCTURED', '-DG_LOG_DOMAIN="GPlugin-Gtk"'],
	include_directories : toplevel_inc,
	dependencies : [gplugin_dep, GTK4],
	version : GPLUGIN_GTK4_LIBRARY_VERSION,
	install : true
)

gplugin_gtk4_inc = include_directories('.')

pkgconfig.generate(
	gplugin_gtk4,
	name : 'gplugin-gtk4',
	description : 'GTK4 widgets for GPlugin',
	filebase : 'gplugin-gtk4',
	subdirs : 'gplugin-gtk4-1.0',
	libraries : [gplugin],
	requires : [GLIB, GOBJECT, GMODULE, GTK4],
)

###############################################################################
# GObject Introspection
###############################################################################
if get_option('introspection')
	gplugin_gtk4_gir = gnome.generate_gir(
		gplugin_gtk4,
		sources : [
			GPLUGIN_GTK4_SOURCES,
			GPLUGIN_GTK4_HEADERS,
			GPLUGIN_GTK4_PUBLIC_BUILT_SOURCES,
			GPLUGIN_GTK4_PUBLIC_BUILT_HEADERS,
		],
		includes : [gplugin_gir[0], 'Gtk-4.0'],
		namespace : 'GPluginGtk4',
		nsversion : '1.0',
		symbol_prefix : 'gplugin_gtk',
		identifier_prefix : 'GPluginGtk',
		install : true,
		header : 'gplugin-gtk.h',
		export_packages: ['gplugin-gtk4'],
		dependencies : [gplugin_dep],
		extra_args : ['--quiet', '-DGPLUGIN_GTK_COMPILATION'])
	GPLUGIN_GTK4_GENERATED_TARGETS += gplugin_gtk4_gir
endif

###############################################################################
# Library Dependency Object
###############################################################################
gplugin_gtk4_dep = declare_dependency(
	include_directories: [toplevel_inc, gplugin_gtk4_inc],
	link_with : [gplugin, gplugin_gtk4],
	sources : [
		GPLUGIN_GTK4_PUBLIC_BUILT_HEADERS,
		GPLUGIN_GTK4_GENERATED_TARGETS,
	],
	dependencies : [GLIB, GOBJECT, GTK4],
)

meson.override_dependency('gplugin-gtk4', gplugin_gtk4_dep)

###############################################################################
# Install Stuff
###############################################################################
install_headers(
	GPLUGIN_GTK4_HEADERS,
	subdir : 'gplugin-gtk4-1.0/gplugin-gtk4'
)

###############################################################################
# subdirectories
###############################################################################
subdir('reference')
