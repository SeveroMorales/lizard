###############################################################################
# Library
###############################################################################

GPLUGIN_LIBRARY_VERSION = '0.1.0'

GPLUGIN_HEADERS = [
	'gplugin-core.h',
	'gplugin-loader.h',
	'gplugin-manager.h',
	'gplugin-options.h',
	'gplugin-plugin.h',
	'gplugin-plugin-info.h',
	'gplugin-version.h',
]

GPLUGIN_SOURCES = [
	'gplugin-core.c',
	'gplugin-plugin.c',
	'gplugin-loader.c',
	'gplugin-manager.c',
	'gplugin-options.c',
	'gplugin-plugin-info.c',
	'gplugin-private.c',
	'gplugin-version.c',
]

GPLUGIN_PUBLIC_BUILT_HEADERS = [
	# Modified below.
]

GPLUGIN_PUBLIC_BUILT_SOURCES = [
	# Modified below.
]

GPLUGIN_PRIVATE_HEADERS = [
	'gplugin-file-source.h',
	'gplugin-file-tree.h',
	'gplugin-source.h',
]

GPLUGIN_PRIVATE_SOURCES = [
	'gplugin-file-source.c',
	'gplugin-file-tree.c',
	'gplugin-source.c',
]

GPLUGIN_PRIVATE_BUILT_HEADERS = [
	# Modified below.
]

GPLUGIN_PRIVATE_BUILT_SOURCES = [
	# Modified below.
]

GPLUGIN_NATIVE_HEADERS = [
	'gplugin-native-plugin.h',
	'gplugin-native-loader.h',
]

GPLUGIN_NATIVE_SOURCES = [
	'gplugin-native-plugin.c',
	'gplugin-native-loader.c',
]

GPLUGIN_GENERATED_TARGETS = [
	# Modified below.
]

###############################################################################
# gplugin-enum.[ch] generation
###############################################################################
ENUM_HEADERS = [
	'gplugin-core.h',
	'gplugin-plugin.h',
]

enums = gnome.mkenums_simple(
	'gplugin-enums',
	sources : ENUM_HEADERS,
	install_header : true,
	install_dir : get_option('includedir') / 'gplugin-1.0' / 'gplugin')

enums_c = enums[0]
enums_h = enums[1]

GPLUGIN_PUBLIC_BUILT_HEADERS += [
	enums_h
]

GPLUGIN_PUBLIC_BUILT_SOURCES += [
	enums_c
]

##############################################################################
# Helper Variables
###############################################################################
PRIVATE_HEADERS = GPLUGIN_PRIVATE_HEADERS + GPLUGIN_PRIVATE_BUILT_HEADERS + \
	['gplugin-private.h']

###############################################################################
# Configure Files
###############################################################################
gplugin_version_h = configure_file(
	input : 'gplugin-version-defs.h.in',
	output : 'gplugin-version-defs.h',
	configuration : version_conf,
	install : true,
	install_dir : get_option('includedir') / 'gplugin-1.0' / 'gplugin'
)

GPLUGIN_PUBLIC_BUILT_HEADERS += [
	gplugin_version_h
]

gplugin_inc = include_directories('.')

# Build gplugin.h
GPLUGIN_H_INCLUDES = []
foreach header : GPLUGIN_HEADERS + ['gplugin-version-defs.h', 'gplugin-enums.h']
	GPLUGIN_H_INCLUDES += f'#include <gplugin/@header@>'
endforeach

conf = configuration_data()
conf.set('GPLUGIN_H_INCLUDES', '\n'.join(GPLUGIN_H_INCLUDES))
gplugin_h = configure_file(
	input : 'gplugin.h.in',
	output : 'gplugin.h',
	configuration : conf,
	install : true,
	install_dir : get_option('includedir') / 'gplugin-1.0')

# Build gplugin-native.h
GPLUGIN_NATIVE_H_INCLUDES = []
foreach header : GPLUGIN_NATIVE_HEADERS
	GPLUGIN_NATIVE_H_INCLUDES += f'#include <gplugin/@header@>'
endforeach

conf = configuration_data()
conf.set('GPLUGIN_NATIVE_H_INCLUDES', '\n'.join(GPLUGIN_NATIVE_H_INCLUDES))
gplugin_native_h = configure_file(
	input : 'gplugin-native.h.in',
	output : 'gplugin-native.h',
	configuration : conf,
	install : true,
	install_dir : get_option('includedir') / 'gplugin-1.0')

###############################################################################
# Library target
###############################################################################
gplugin = library('gplugin',
	GPLUGIN_SOURCES,
	GPLUGIN_NATIVE_SOURCES,
	GPLUGIN_PUBLIC_BUILT_SOURCES,
	GPLUGIN_PRIVATE_SOURCES,
	GPLUGIN_PRIVATE_BUILT_SOURCES,
	GPLUGIN_NATIVE_HEADERS,
	GPLUGIN_HEADERS,
	GPLUGIN_PUBLIC_BUILT_HEADERS,
	PRIVATE_HEADERS,
	gplugin_h,
	gplugin_native_h,
	c_args : ['-DGPLUGIN_COMPILATION', '-DG_LOG_USE_STRUCTURED', '-DG_LOG_DOMAIN="GPlugin"'],
	include_directories : toplevel_inc,
	dependencies : [GLIB, GOBJECT, GMODULE],
	version : GPLUGIN_LIBRARY_VERSION,
	install : true
)

pkgconfig.generate(
	gplugin,
	name : 'gplugin',
	description : 'A fully featured GModule based plugin library',
	filebase : 'gplugin',
	subdirs : 'gplugin-1.0',
	requires : [GLIB, GOBJECT, GMODULE],
	variables : [
		'plugindir=${libdir}',
	],
)

###############################################################################
# GObject Introspection
###############################################################################
if get_option('introspection')
	gplugin_gir = gnome.generate_gir(gplugin,
		sources : GPLUGIN_SOURCES + GPLUGIN_HEADERS +
		          GPLUGIN_PUBLIC_BUILT_SOURCES +
		          GPLUGIN_PUBLIC_BUILT_HEADERS,
		includes : ['GObject-2.0'],
		header : 'gplugin.h',
		namespace : 'GPlugin',
		symbol_prefix : 'gplugin',
		nsversion : '1.0',
		install : true,
		export_packages : ['gplugin'],
		extra_args : ['--quiet', '-DGPLUGIN_COMPILATION'])
	GPLUGIN_GENERATED_TARGETS += gplugin_gir
endif

###############################################################################
# Library Dependency Object
###############################################################################
gplugin_dep = declare_dependency(
	include_directories : [toplevel_inc, include_directories('.')],
	link_with : gplugin,
	sources : GPLUGIN_PUBLIC_BUILT_HEADERS + GPLUGIN_GENERATED_TARGETS,
	dependencies : [GLIB, GOBJECT]
)

meson.override_dependency('gplugin', gplugin_dep)

###############################################################################
# loader-tests static library
###############################################################################
gplugin_loader_tests = static_library('gplugin-loader-tests',
	'gplugin-loader-tests.c',
	'gplugin-loader-tests.h',
	c_args : ['-DGI_TYPELIB_PATH="@0@"'.format(meson.current_build_dir())],
	dependencies : [gplugin_dep],
)

###############################################################################
# Install Stuff
###############################################################################
# install the normal includes into the gplugin subdirectory
install_headers(
	GPLUGIN_HEADERS,
	GPLUGIN_NATIVE_HEADERS,
	subdir : 'gplugin-1.0/gplugin'
)

###############################################################################
# subdirectories
###############################################################################
subdir('reference')
subdir('share')
subdir('tests')
