# UPDATING VERSION NUMBERS FOR RELEASES
#
# The version number is:
#   <major>.<minor>.<micro><suffix>
#
# If any code has changed in libgnt:
#   micro += 1
#
# If any functions have been added to libgnt:
#   micro = 0
#   minor += 1
#
# If backwards compatibility has been broken in libgnt:
#   micro = 0
#   minor = 0
#   major += 1
#   gnt_soversion += 1
#
# suffix should be similar to one of the following:
#   For beta releases:          '-beta2'
#   For code under development: '-devel'
#   For production releases:    ''
#

# the last version for Finch 2 was 2.8.10,
# the first version for Finch 3 was 3.0.0

project('libgnt', 'c',
	license : 'GPL-2.0-or-later',
	version : '3.0.0-devel',
	meson_version : '>=0.58.0',
	default_options : ['c_std=c17', 'warning_level=2'])
gnt_soversion = 1

parts = meson.project_version().split('-')
if parts.length() > 1
  gnt_extra_version = parts[1]
else
  gnt_extra_version = ''
endif

parts = parts[0].split('.')
gnt_major_version = parts[0]
gnt_minor_version = parts[1]
gnt_micro_version = parts[2]

gnt_config = configuration_data()
gnt_config.set('GNT_MAJOR_VERSION', gnt_major_version)
gnt_config.set('GNT_MINOR_VERSION', gnt_minor_version)
gnt_config.set('GNT_MICRO_VERSION', gnt_micro_version)
gnt_config.set_quoted('GNT_EXTRA_VERSION', gnt_extra_version)
gnt_config.set_quoted('GNT_VERSION', meson.project_version())

compiler = meson.get_compiler('c')
pkgconfig = import('pkgconfig')

devenv = environment()

# #######################################################################
# # Check for GLib 2.44
# #######################################################################
glib = dependency('glib-2.0', version : '>= 2.44.0')
gobject = dependency('gobject-2.0')
gnome = import('gnome')

#######################################################################
# Check for LibXML2
#######################################################################
libxml = dependency('libxml-2.0', version : '>= 2.6.0')

#######################################################################
# Check for ncurses and other things used by it
#######################################################################
ncurses_available = true
ncurses_widechar = true
ncurses_header = 'ncurses.h'
# Some distros put the headers in ncursesw/, some don't. These are ordered to
# pick the last available as most-specific version.
ncursesw_header_paths = ['', 'ncursesw/']

ncurses = [
	dependency('ncursesw', required : false),
	dependency('panelw', required : false),
]
if ncurses[0].found() and ncurses[1].found()
	foreach location : ncursesw_header_paths
		f = location + 'ncurses.h'
		if compiler.has_header_symbol(f, 'get_wch',
			prefix : '#define _XOPEN_SOURCE_EXTENDED')
			ncurses_header = f
		endif
	endforeach
else
	ncurses_available = false
	ncurses_inc = []
	ncurses_libs = [
		compiler.find_library('ncursesw', required : false),
		compiler.find_library('panelw', required : false)
	]
	if ncurses_libs[0].found() and ncurses_libs[1].found()
		foreach location : ncursesw_header_paths
			f = location + 'ncurses.h'
			if compiler.has_header_symbol(f, 'get_wch',
				prefix : '#define _XOPEN_SOURCE_EXTENDED')
				ncurses_available = true
				ncurses_header = f
			endif
		endforeach

		if ncurses_available
			ncurses = declare_dependency(
				include_directories : ncurses_inc,
				dependencies : ncurses_libs
			)
		endif
	endif
endif

if not ncurses_available
	# ncursesw was not found. Look for plain old ncurses
	ncurses = [
		dependency('ncurses', required : false),
		dependency('panel', required : false),
	]
	if ncurses[0].found() and ncurses_libs[1].found()
		ncurses_available = true
	else
		ncurses_libs = [
			compiler.find_library('ncurses', required : false),
			compiler.find_library('panel', required : false),
		]
		ncurses_available = ncurses_libs[0].found() and ncurses_libs[1].found()
		ncurses = declare_dependency(dependencies : ncurses_libs)
	endif
	ncurses_widechar = false
endif

if not ncurses_available and host_machine.system() == 'windows'
	# Try pdcurses too.
	ncurses_header = 'curses.h'
	ncurses_libs = compiler.find_library('pdcurses', required : false)
	ncurses_available = compiler.has_header(ncurses_header) and ncurses_libs.found()
	ncurses = declare_dependency(dependencies : ncurses_libs)
endif

if not ncurses_available
	error('ncurses could not be found!')
endif
gnt_config.set('NCURSES_HEADER', ncurses_header)
gnt_config.set10('NCURSES_WIDECHAR', ncurses_widechar)

libgnt_SOURCES = [
	'gntbindable.c',
	'gntbox.c',
	'gntbutton.c',
	'gntcheckbox.c',
	'gntclipboard.c',
	'gntcolors.c',
	'gntcombobox.c',
	'gntentry.c',
	'gntfilesel.c',
	'gntkeys.c',
	'gntlabel.c',
	'gntline.c',
	'gntmain.c',
	'gntmenu.c',
	'gntmenuitem.c',
	'gntmenuitemcheck.c',
	'gntprogressbar.c',
	'gntslider.c',
	'gntstyle.c',
	'gnttextview.c',
	'gnttree.c',
	'gntutils.c',
	'gntwidget.c',
	'gntwindow.c',
	'gntwm.c',
	'gntws.c'
]

libgnt_headers = [
	'gntbindable.h',
	'gntbox.h',
	'gntbutton.h',
	'gntcheckbox.h',
	'gntclipboard.h',
	'gntcolors.h',
	'gntcombobox.h',
	'gntentry.h',
	'gntfilesel.h',
	'gntkeys.h',
	'gntlabel.h',
	'gntline.h',
	'gntmain.h',
	'gntmenu.h',
	'gntmenuitem.h',
	'gntmenuitemcheck.h',
	'gntprogressbar.h',
	'gntslider.h',
	'gntstyle.h',
	'gnttextview.h',
	'gnttree.h',
	'gntutils.h',
	'gntwidget.h',
	'gntwindow.h',
	'gntwm.h',
	'gntws.h',
]

libgnt_generated_targets = []

# Check for Python headers
_PYTHONS = [
	['python-3.10-embed', '>=3.10'],
	['python-3.9-embed', '>=3.9'],
	['python-3.8-embed', '>=3.8'],
	['python3-embed', '>=3.8'],
	['python3', '>=3.8'],
	['python-3.7', '>=3.7'],
	['python-3.7m', '>=3.7'],
]
foreach _PYTHON : _PYTHONS
	python_dep = dependency(_PYTHON[0], version : _PYTHON[1], required: false)
	if python_dep.found()
		break
	endif
endforeach
gnt_config.set('USE_PYTHON', python_dep.found())

configure_file(output : 'gntconfig.h',
	configuration : gnt_config)
configure_file(input : 'gntversion.h.in', output : 'gntversion.h',
	configuration : gnt_config,
	install : true,
	install_dir : get_option('includedir') / 'gnt3')
gntncurses_h = configure_file(input : 'gntncurses.h.in', output : 'gntncurses.h',
	configuration : gnt_config,
	install : true,
	install_dir : get_option('includedir') / 'gnt3')

GNT_H_INCLUDES = []
foreach header : libgnt_headers + ['gntncurses.h', 'gntversion.h']
	GNT_H_INCLUDES += '#include <@0@>'.format(header)
endforeach
gnt_h_config = configuration_data()
gnt_h_config.set('GNT_H_INCLUDES', '\n'.join(GNT_H_INCLUDES))
gnt_h = configure_file(input : 'gnt.h.in', output : 'gnt.h',
	configuration : gnt_h_config,
	install : true,
	install_dir : get_option('includedir') / 'gnt3')

gmodule = dependency('gmodule-2.0')

install_headers(libgnt_headers, subdir : 'gnt3')

if host_machine.system() == 'windows'
	windows = import('windows')
	libgnt_winres = configure_file(
		input : 'libgnt_winres.rc.in',
		output : 'libgnt_winres.rc',
		configuration : gnt_config)
	libgnt_SOURCES += windows.compile_resources(libgnt_winres)
endif

libgnt_inc = include_directories('.')
libgnt = library(
	'gnt',
	libgnt_SOURCES,
	libgnt_headers,
	gnt_h,
	gntncurses_h,
	c_args : [
		'-DGNT_COMPILATION',
		'-DG_LOG_USE_STRUCTURED',
		'-DG_LOG_DOMAIN="Gnt"',
	],
	install : true,
	version : '@0@.@1@.@2@'.format(
		gnt_soversion,
		gnt_minor_version,
		gnt_micro_version),
	dependencies : [ncurses, libxml, glib, gobject, gmodule, python_dep])

pkgconfig.generate(
	libgnt,
	name : 'LibGNT',
	description : 'Glib Ncurses Toolkit is a collection of curses-widgets.',
	filebase : 'gnt3',
	subdirs : 'gnt3',
	requires : [glib],
	variables : ['plugindir = ${libdir}/gnt3'],
	)

if get_option('introspection')
	libgnt_gir = gnome.generate_gir(libgnt,
		sources : libgnt_headers + [gnt_h],
		includes : ['GLib-2.0', 'GObject-2.0'],
		namespace : 'Gnt',
		symbol_prefix : 'gnt',
		identifier_prefix : 'Gnt',
		nsversion : '@0@.@1@'.format(gnt_major_version, gnt_minor_version),
		install : true,
		extra_args : ['-DGNT_COMPILATION', '--quiet'],
		export_packages : ['gnt3'],
		header : 'gnt.h')
	libgnt_generated_targets += libgnt_gir
endif

libgnt_dep = declare_dependency(
	include_directories : libgnt_inc,
	link_with : libgnt,
	dependencies : [ncurses, glib],
	sources : libgnt_generated_targets)

meson.override_dependency('gnt3', libgnt_dep)

subdir('wms')
subdir('test')
subdir('doc')

config_home = get_option('devenv-config-dir')
if config_home == ''
    config_home = meson.global_build_root() / 'config'
endif
devenv.set('XDG_CONFIG_HOME', config_home)

meson.add_devenv(devenv)

