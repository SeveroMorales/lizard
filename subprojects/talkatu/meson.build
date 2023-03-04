###############################################################################
# Project Info
###############################################################################
project('talkatu', 'c', version : '0.2.0',
	license : 'GPL-2.0-or-later',
	meson_version : '>=0.58.0',
	default_options : ['c_std=c17', 'warning_level=2'])

parts = meson.project_version().split('-')
if parts.length() > 1
	extra = parts[1]
else
	extra = ''
endif

parts = parts[0].split('.')
TALKATU_MAJOR_VERSION = parts[0]

version_conf = configuration_data()
version_conf.set('TALKATU_MAJOR_VERSION', TALKATU_MAJOR_VERSION)
version_conf.set('TALKATU_MINOR_VERSION', parts[1])
version_conf.set('TALKATU_MICRO_VERSION', parts[2])
version_conf.set('TALKATU_EXTRA_VERSION', extra)
version_conf.set('TALKATU_VERSION', meson.project_version())

LOCALE_DIR = get_option('prefix') / get_option('localedir')
add_project_arguments(f'-DLOCALEDIR="@LOCALE_DIR@"', language : 'c')

devenv = environment()

###############################################################################
# Dependencies
###############################################################################
gnome = import('gnome')
pkgconfig = import('pkgconfig')

GLIB = dependency('glib-2.0', version : '>=2.52.0')
GOBJECT = dependency('gobject-2.0')

GTK4 = dependency('gtk4', version : '>=4.6.0')

GUMBO = dependency('gumbo', version : '>=0.10')

CMARK = dependency('libcmark', version : '>=0.28.0')

if get_option('help2man')
	HELP2MAN = find_program('help2man')
endif

###############################################################################
# NLS
###############################################################################
GETTEXT_PACKAGE = 'talkatu'

add_project_arguments(f'-DGETTEXT_PACKAGE="@GETTEXT_PACKAGE@"', language : 'c')

if get_option('nls')
	i18n = import('i18n')
endif

###############################################################################
# Documentation
###############################################################################
if get_option('doc') and not get_option('introspection')
	error('Documentation requires GObject Introspection.')
endif

gidocgen_dep = dependency(
	'gi-docgen', version: '>= 2021.1',
	fallback: ['gi-docgen', 'dummy_dep'],
	required: get_option('doc')
)

gidocgen = find_program('gi-docgen', required : get_option('doc'))
docs_dir = get_option('prefix') / get_option('datadir') / 'doc'

###############################################################################
# Build Info
###############################################################################
compiler = meson.get_compiler('c')

top_srcdir = include_directories('.')

add_project_arguments(
	'-DPREFIX="@0@"'.format(get_option('prefix')),
	'-DLIBDIR="@0@"'.format(get_option('libdir')),
	language : 'c'
)

if compiler.has_argument('-Wformat')
	add_project_arguments('-Wformat', language : 'c')
	if compiler.has_multi_arguments(['-Wformat', '-Werror=format-security'])
		add_project_arguments('-Werror=format-security', language : 'c')
	endif
endif

toplevel_inc = include_directories('.')

doc_targets = []

###############################################################################
# Subdirectories
###############################################################################
subdir('talkatu')
subdir('po')
subdir('demo')
subdir('vapi')

###############################################################################
# doc alias target
###############################################################################
if get_option('doc')
	alias_target('doc', doc_targets)
endif

###############################################################################
# devenv
###############################################################################
meson.add_devenv(devenv)

###############################################################################
# Install stuff
###############################################################################
# documentation
install_data('ChangeLog', 'INSTALL', 'README.md', 'HACKING', 'logo.png',
	install_dir : get_option('datadir') / 'doc/talkatu',
)
