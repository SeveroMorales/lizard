enable_consoleui = false
if not get_option('consoleui')
  subdir_done()
endif

libgnt_dep = dependency('gnt3',
	version : '>= 3.0.0',
	fallback : ['libgnt', 'libgnt_dep'])

#######################################################################
# Check for ncurses and other things used by it
# FIXME: This should be temporary until libgnt wraps the functionality.
#######################################################################
ncurses_available = true
ncurses_header = 'ncurses.h'
# Some distros put the headers in ncursesw/, some don't. These are ordered to
# pick the last available as most-specific version.
ncursesw_header_paths = ['', 'ncursesw/']

ncurses = dependency('ncursesw', required : false)
if ncurses.found()
	foreach location : ncursesw_header_paths
		f = location / 'ncurses.h'
		if compiler.has_header_symbol(f, 'get_wch',
		    prefix : '#define _XOPEN_SOURCE_EXTENDED')
			ncurses_header = f
		endif
	endforeach
else
	ncurses_available = false
	ncurses_inc = []
	ncurses_libs = compiler.find_library('ncursesw', required : false)
	if ncurses_libs.found()
		foreach location : ncursesw_header_paths
			f = location / 'ncurses.h'
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
	ncurses = dependency('ncurses', required : false)
	if ncurses.found()
		ncurses_available = true
	else
		ncurses_libs = compiler.find_library('ncurses', required : false)
		ncurses_available = ncurses_libs.found()
		ncurses = declare_dependency(dependencies : ncurses_libs)
	endif
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

ncurses_header = f'-DNCURSES_HEADER="@ncurses_header@"'
enable_consoleui = true


libfinch_SOURCES = [
	'finchnotifications.c',
	'finchui.c',
	'gntaccount.c',
	'gntblist.c',
	'gntconn.c',
	'gntconv.c',
	'gntdebug.c',
	'gntidle.c',
	'gntmedia.c',
	'gntmenuutil.c',
	'gntnotify.c',
	'gntplugin.c',
	'gntprefs.c',
	'gntrequest.c',
	'gntroomlist.c',
	'gntstatus.c',
	'gntxfer.c',
	package_revision,
	'libfinch.c'
]

libfinch_headers = [
	'finchnotifications.h',
	'finchui.h',
	'gntaccount.h',
	'gntblist.h',
	'gntconn.h',
	'gntconv.h',
	'gntdebug.h',
	'gntidle.h',
	'gntmedia.h',
	'gntmenuutil.h',
	'gntnotify.h',
	'gntplugin.h',
	'gntprefs.h',
	'gntrequest.h',
	'gntroomlist.h',
	'gntstatus.h',
	'gntxfer.h',
	'libfinch.h'
]

libfinch_enum_headers = [
	'gntconv.h',
]

# the built lists are append to below
libfinch_built_sources = []
libfinch_built_headers = []
libfinch_generated_sources = []

finch_SOURCES = [
	'finch.c'
]

if IS_WIN32
	winmm = compiler.find_library('winmm')

	finch_winres = configure_file(
	    input : 'finch_winres.rc.in',
	    output : 'finch_winres.rc',
	    configuration : version_conf)
	finch_SOURCES += windows.compile_resources(finch_winres)
	libfinch_winres = configure_file(
	    input : 'libfinch_winres.rc.in',
	    output : 'libfinch_winres.rc',
	    configuration : version_conf)
	libfinch_SOURCES += windows.compile_resources(libfinch_winres)
else
	winmm = []
endif

libfinch_enums = gnome.mkenums_simple('finchenums',
	sources: libfinch_enum_headers)
libfinch_built_sources += libfinch_enums[0]
libfinch_built_headers += libfinch_enums[1]

FINCH_H_INCLUDES = []
foreach header : libfinch_headers + ['finchenums.h']
	FINCH_H_INCLUDES += f'#include <finch/@header@>'
endforeach
finch_h_conf = configuration_data()
finch_h_conf.set('FINCH_H_INCLUDES', '\n'.join(FINCH_H_INCLUDES))

finch_h = configure_file(input : 'finch.h.in',
                         output : 'finch.h',
                         configuration : finch_h_conf,
                         install : true,
			 install_dir : get_option('includedir') / 'finch-3')
libfinch_built_headers += finch_h

install_headers(libfinch_headers, subdir : 'finch-3')

libfinch_inc = include_directories('.')
libfinch = shared_library('finch3',
    libfinch_SOURCES + libfinch_built_headers + libfinch_built_sources,
    c_args : [
        '-DSTANDALONE',
        '-DGNTSEAL_ENABLE',
        '-DFINCH_COMPILATION',
        ncurses_header,
        '-DG_LOG_USE_STRUCTURED',
        '-DG_LOG_DOMAIN="Finch"',
    ],
    include_directories : [toplevel_inc],
    version : PURPLE_LIB_VERSION,
    dependencies : [libpurple_dep, libgnt_dep, ncurses, glib, winmm],
    install : true)

if enable_introspection
	introspection_sources = libfinch_headers

	libfinch_gir = gnome.generate_gir(libfinch,
	    sources : introspection_sources,
	    header : 'finch.h',
	    includes : ['GLib-2.0', 'GModule-2.0', 'GObject-2.0', libpurple_gir[0], 'Gnt-3.0'],
	    namespace : 'Finch',
	    symbol_prefix : 'finch',
	    identifier_prefix : 'Finch',
	    export_packages : 'finch-3',
	    nsversion : f'@purple_major_version@.@purple_minor_version@',
	    dependencies: [libgnt_dep, gplugin_dep, libpurple_dep],
	    install : true,
	    extra_args : ['-DFINCH_COMPILATION', '--quiet'])
	libfinch_generated_sources += libfinch_gir
endif

libfinch_dep = declare_dependency(
    include_directories : [toplevel_inc, libfinch_inc],
    link_with : libfinch,
    sources: libfinch_built_headers + libfinch_generated_sources,
    dependencies : [libpurple_dep, libgnt_dep, glib])

finch = executable('finch3',
    finch_SOURCES,
    c_args : [
        '-DSTANDALONE',
        '-DGNTSEAL_ENABLE',
        '-DG_LOG_USE_STRUCTURED',
        '-DG_LOG_DOMAIN="Finch"',
    ],
    dependencies : [libpurple_dep, libgnt_dep, libfinch_dep],
    install : true)

meson.override_dependency('finch-3', libfinch_dep)

pkgconfig.generate(
    libfinch,
    name : 'Finch',
    description : 'Finch is an instant messenger application that uses libpurple for protocol support and ncurses (libgnt) for the UI.',
    version : meson.project_version(),
    filebase : 'finch-3',
    subdirs : 'finch-3',
    # NOTE: Don't use gnt from pkgconfig, as it might be a subproject.
    requires : ['gnt', libpurple],
    variables : [f'plugindir=${libdir}/finch-@purple_major_version@'])

subdir('plugins')
