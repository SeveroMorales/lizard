project('hasl', 'c',
        default_options : ['c_std=c17', 'warning_level=3'],
        license : 'LGPL-2.0-or-later',
        meson_version : '>=0.58.0',
        version : '0.1.0-dev')

gnome = import('gnome')
pkgconfig = import('pkgconfig')

toplevel_inc = include_directories('.')

###############################################################################
# Version Stuff
###############################################################################
parts = meson.project_version().split('-')
if parts.length() > 1
    extra = parts[1]
else
    extra = ''
endif

parts = parts[0].split('.')
HASL_MAJOR_VERSION = parts[0]

version_config = configuration_data()
version_config.set('HASL_MAJOR_VERSION', HASL_MAJOR_VERSION)
version_config.set('HASL_MINOR_VERSION', parts[1])
version_config.set('HASL_MICRO_VERSION', parts[2])
version_config.set('HASL_EXTRA_VERSION', extra)
version_config.set('HASL_VERSION', meson.project_version())

###############################################################################
# Dependencies
###############################################################################
glib = dependency('glib-2.0', version : '>= 2.70')
gobject = dependency('gobject-2.0', version : '>= 2.70')

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
doc_targets = []

###############################################################################
# Sub-directories
###############################################################################
subdir('hasl')

# This needs to be after all of the sub-directories have been processed.
if get_option('doc')
  alias_target('doc', doc_targets)
endif
