###############################################################################
# Library
###############################################################################

TALKATU_LIBRARY_VERSION = '0.1.0'
TALKATU_HEADERS = [
	'talkatuactiongroup.h',
	'talkatuattachment.h',
	'talkatuattachmentdialog.h',
	'talkatuattachmentpreview.h',
	'talkatuautoscroller.h',
	'talkatubuffer.h',
	'talkatucodeset.h',
	'talkatucore.h',
	'talkatueditor.h',
	'talkatuformat.h',
	'talkatuhistory.h',
	'talkatuhistoryrow.h',
	'talkatuhtmlpangorenderer.h',
	'talkatuhtmlrenderer.h',
	'talkatuinput.h',
	'talkatulinkdialog.h',
	'talkatumarkdown.h',
	'talkatumarkup.h',
	'talkatumessage.h',
	'talkatusimpleattachment.h',
	'talkatutag.h',
	'talkatutagtable.h',
	'talkatutoolbar.h',
	'talkatutypinglabel.h',
	'talkatuview.h',
]

TALKATU_SOURCES = [
	'talkatuactiongroup.c',
	'talkatuattachment.c',
	'talkatuattachmentdialog.c',
	'talkatuattachmentpreview.c',
	'talkatuautoscroller.c',
	'talkatubuffer.c',
	'talkatucodeset.c',
	'talkatucore.c',
	'talkatueditor.c',
	'talkatuhistory.c',
	'talkatuhistoryrow.c',
	'talkatuhtmlpangorenderer.c',
	'talkatuhtmlrenderer.c',
	'talkatuinput.c',
	'talkatulinkdialog.c',
	'talkatumarkdown.c',
	'talkatumarkup.c',
	'talkatumessage.c',
	'talkatusimpleattachment.c',
	'talkatutag.c',
	'talkatutagtable.c',
	'talkatutoolbar.c',
	'talkatutypinglabel.c',
	'talkatuview.c',
]

TALKATU_PUBLIC_BUILT_HEADERS = [
	# Modified below.
]

TALKATU_PUBLIC_BUILT_SOURCES = [
	# Modified below.
]

TALKATU_PRIVATE_HEADERS = [
]

TALKATU_PRIVATE_SOURCES = [
]

TALKATU_PRIVATE_BUILT_HEADERS = [
	# Modified below.
]

TALKATU_PRIVATE_BUILT_SOURCES = [
	# Modified below.
]

TALKATU_GENERATED_TARGETS = [
	# Modified below.
]

TALKATU_ENUM_HEADERS = [
	'talkatubuffer.h',
	'talkatuformat.h',
	'talkatumessage.h',
	'talkatutag.h',
	'talkatuinput.h',
]

###############################################################################
# Resources
###############################################################################
talkatu_resources = gnome.compile_resources(
	'talkaturesources',
	'data/talkatu.gresource.xml',
	c_name: 'talkatu',
	source_dir: 'data')

###############################################################################
# talkatuversion.h
###############################################################################
talkatuversion_h = configure_file(
	input : 'talkatuversion.h.in',
	output : 'talkatuversion.h',
	configuration : version_conf,
	install : true,
	install_dir : get_option('includedir') / 'talkatu-1.0/talkatu')

TALKATU_PUBLIC_BUILT_HEADERS += [talkatuversion_h]

###############################################################################
# talkatuenums.[ch]
###############################################################################
talkatuenums = gnome.mkenums_simple(
	'talkatuenums',
	sources : TALKATU_ENUM_HEADERS,
	install_header : true,
	install_dir : get_option('includedir') / 'talkatu-1.0/talkatu')

talkatuenums_c = talkatuenums[0]
talkatuenums_h = talkatuenums[1]

TALKATU_PUBLIC_BUILT_SOURCES += [talkatuenums_c]
TALKATU_PUBLIC_BUILT_HEADERS += [talkatuenums_h]

###############################################################################
# talkatu.h
###############################################################################
TALKATU_H_INCLUDES = []

foreach header : TALKATU_HEADERS + ['talkatuenums.h', 'talkatuversion.h']
	TALKATU_H_INCLUDES += f'#include <talkatu/@header@>'
endforeach

conf = configuration_data()
conf.set('TALKATU_H_INCLUDES', '\n'.join(TALKATU_H_INCLUDES))
talkatu_h = configure_file(
	input : 'talkatu.h.in',
	output : 'talkatu.h',
	configuration : conf,
	install : true,
	install_dir : get_option('includedir') / 'talkatu-1.0')

###############################################################################
# Library target
###############################################################################
talkatu = library('talkatu',
	TALKATU_SOURCES,
	TALKATU_PUBLIC_BUILT_SOURCES,
	TALKATU_PRIVATE_SOURCES,
	TALKATU_PRIVATE_BUILT_SOURCES,
	TALKATU_HEADERS,
	TALKATU_PUBLIC_BUILT_HEADERS,
	talkatuversion_h,
	talkatu_h,
	talkatu_resources,
	c_args : ['-DTALKATU_COMPILATION', '-DG_LOG_USE_STRUCTURED', '-DG_LOG_DOMAIN="Talkatu"'],
	include_directories : toplevel_inc,
	dependencies : [CMARK, GLIB, GOBJECT, GUMBO, GTK4],
	version : TALKATU_LIBRARY_VERSION,
	install : true
)

talkatu_inc = include_directories('.')

pkgconfig.generate(
	name : 'talkatu',
	description : 'A set of instant message related widgets',
	version : meson.project_version(),
	filebase : 'talkatu',
	subdirs : 'talkatu-1.0',
	libraries : talkatu,
	requires : ['glib-2.0', 'gobject-2.0', 'gtk4'],
)

###############################################################################
# GObject Introspection
###############################################################################
if get_option('introspection')
	talkatu_gir = gnome.generate_gir(talkatu,
		sources : TALKATU_SOURCES + TALKATU_HEADERS +
		          TALKATU_PUBLIC_BUILT_SOURCES +
		          TALKATU_PUBLIC_BUILT_HEADERS,
		export_packages : ['talkatu'],
		extra_args : ['--quiet', '--warn-all', '-DTALKATU_COMPILATION'],
		header : 'talkatu/talkatu.h',
		includes : ['GModule-2.0', 'GObject-2.0', 'Gtk-4.0'],
		install : true,
		namespace : 'Talkatu',
		nsversion : f'@TALKATU_MAJOR_VERSION@.0',
		symbol_prefix : 'talkatu',
	)
	TALKATU_GENERATED_TARGETS += talkatu_gir
endif

###############################################################################
# Library Dependency Object
###############################################################################
talkatu_dep = declare_dependency(
	include_directories : [toplevel_inc, talkatu_inc],
	link_with : talkatu,
	sources : TALKATU_PUBLIC_BUILT_HEADERS + TALKATU_GENERATED_TARGETS,
	dependencies : [GLIB, GOBJECT, GTK4]
)

meson.override_dependency('talkatu', talkatu_dep)

###############################################################################
# Install Stuff
###############################################################################
# install the normal includes into the talkatuhtml subdirectory
install_headers(
	TALKATU_HEADERS,
	subdir : 'talkatu-1.0/talkatu'
)

###############################################################################
# Subdirectories
###############################################################################
subdir('tests')
subdir('reference')

