HASL_SOURCES = [
  'haslcontext.c',
  'haslmechanism.c',
  'haslmechanismexternal.c',
  'haslmechanismplain.c',
]

HASL_HEADERS = [
  'haslcontext.h',
  'haslcore.h',
  'haslmechanism.h',
  'haslmechanismexternal.h',
  'haslmechanismplain.h',
]

HASL_BUILT_SOURCES = []
HASL_BUILT_HEADERS = []
HASL_GENERATED_TARGETS = []

###############################################################################
# Single Header
###############################################################################
HASL_H_INCLUDES = []
foreach header : HASL_HEADERS + ['haslenums.h']
    HASL_H_INCLUDES += f'#include <hasl/@header@>'
endforeach

header_config = configuration_data()
header_config.set('HASL_H_INCLUDES', '\n'.join(HASL_H_INCLUDES))
hasl_h = configure_file(
    input : 'hasl.h.in',
    output : 'hasl.h',
    configuration : header_config,
    install : true,
    install_dir : get_option('includedir') / 'hasl-1.0')

###############################################################################
# Enums
###############################################################################
HASL_ENUM_HEADERS = [
  'haslmechanism.h',
]

hasl_enums = gnome.mkenums_simple(
  'haslenums',
  sources : HASL_ENUM_HEADERS,
  install_header : true,
  install_dir : get_option('includedir') / 'hasl-1.0' / 'hasl')

HASL_BUILT_SOURCES += hasl_enums[0]
HASL_BUILT_HEADERS += hasl_enums[1]

###############################################################################
# Library Target
###############################################################################
libhasl = library('hasl',
  HASL_SOURCES,
  HASL_HEADERS,
  HASL_BUILT_SOURCES,
  HASL_BUILT_HEADERS,
  c_args : ['-DHASL_COMPILATION', '-DG_LOG_USE_STRUCTURED',
            '-DG_LOG_DOMAIN="Hasl"'],
  dependencies: [glib, gobject],
  include_directories : toplevel_inc,
  install : true)

install_headers(
  HASL_HEADERS,
  subdir : 'hasl-1.0/hasl')

pkgconfig.generate(
  libhasl,
  name : 'hasl',
  description : 'Hassle free SASL client library',
  filebase : 'hasl',
  subdirs : 'hasl-1.0',
  requires : [glib, gobject],
)

###############################################################################
# GObject Introspection
###############################################################################
if get_option('introspection')
  hasl_gir = gnome.generate_gir(libhasl,
    sources : HASL_SOURCES + HASL_HEADERS,
    includes : ['GObject-2.0'],
    header : 'hasl.h',
    namespace : 'Hasl',
    symbol_prefix : 'hasl',
    nsversion : '1.0',
    install : true,
    export_packages : ['hasl'],
    extra_args : ['--quiet', '-DHASL_COMPILATION'])
  HASL_GENERATED_TARGETS += hasl_gir
endif

###############################################################################
# Library Dependency Object
###############################################################################
hasl_dep = declare_dependency(
    include_directories : [toplevel_inc, include_directories('.')],
    link_with : libhasl,
    sources : [HASL_GENERATED_TARGETS, HASL_BUILT_HEADERS],
    dependencies : [glib, gobject]
)

meson.override_dependency('hasl', hasl_dep)

###############################################################################
# Sub Directories
###############################################################################
subdir('reference')
subdir('tests')
