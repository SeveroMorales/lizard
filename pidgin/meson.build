if not ENABLE_GTK
  subdir_done()
endif

libpidgin_SOURCES = [
	'gtkaccount.c',
	'gtkblist.c',
	'gtkconv.c',
	'gtkdialogs.c',
	'gtkidle.c',
	'gtkmedia.c',
	'gtknotify.c',
	'gtkpluginpref.c',
	'gtkrequest.c',
	'gtkroomlist.c',
	'gtksavedstatuses.c',
	'gtkutils.c',
	'gtkwhiteboard.c',
	'gtkxfer.c',
	'libpidgin.c',
	'pidginabout.c',
	'pidginaccountchooser.c',
	'pidginaccounteditor.c',
	'pidginaccountfilterconnected.c',
	'pidginaccountfilterprotocol.c',
	'pidginaccountmanager.c',
	'pidginaccountrow.c',
	'pidginaccountsdisabledmenu.c',
	'pidginaccountsenabledmenu.c',
	'pidginactiongroup.c',
	'pidginaddbuddydialog.c',
	'pidginaddchatdialog.c',
	'pidginapplication.c',
	'pidginattachment.c',
	'pidginavatar.c',
	'pidgincolor.c',
	'pidgincommands.c',
	'pidgincontactlist.c',
	'pidgincontactlistwindow.c',
	'pidgindebug.c',
	'pidgindisplayitem.c',
	'pidgindisplaywindow.c',
	'pidginiconname.c',
	'pidgininfopane.c',
	'pidgininvitedialog.c',
	'pidginkeypad.c',
	'pidginmessage.c',
	'pidginmooddialog.c',
	'pidginnotificationaddcontact.c',
	'pidginnotificationauthorizationrequest.c',
	'pidginnotificationconnectionerror.c',
	'pidginnotificationlist.c',
	'pidginplugininfo.c',
	'pidginpluginsdialog.c',
	'pidginpluginsmenu.c',
	'pidginpresenceicon.c',
	'pidginprotocolchooser.c',
	'pidginstatusbox.c',
	'pidginstatuseditor.c',
	'pidginstatusmanager.c',
	'pidginstatusprimitivechooser.c',
	'pidgintalkatu.c',
	'pidginui.c',
	'prefs/pidginprefs.c',
	'prefs/pidginawayprefs.c',
	'prefs/pidginconversationprefs.c',
	'prefs/pidgincredentialproviderrow.c',
	'prefs/pidgincredentialprefs.c',
	'prefs/pidginnetworkprefs.c',
	'prefs/pidginproxyprefs.c',
	'prefs/pidginvvprefs.c',
]

libpidgin_headers = [
	'gtkaccount.h',
	'gtkblist.h',
	'gtkconv.h',
	'gtkdialogs.h',
	'gtkidle.h',
	'gtkmedia.h',
	'gtknotify.h',
	'gtkpluginpref.h',
	'gtkrequest.h',
	'gtkroomlist.h',
	'gtksavedstatuses.h',
	'gtkutils.h',
	'gtkwhiteboard.h',
	'gtkxfer.h',
	'pidginabout.h',
	'pidginaccountchooser.h',
	'pidginaccounteditor.h',
	'pidginaccountfilterconnected.h',
	'pidginaccountfilterprotocol.h',
	'pidginaccountmanager.h',
	'pidginaccountrow.h',
	'pidginaccountsdisabledmenu.h',
	'pidginaccountsenabledmenu.h',
	'pidginactiongroup.h',
	'pidginaddbuddydialog.h',
	'pidginaddchatdialog.h',
	'pidginapplication.h',
	'pidginattachment.h',
	'pidginavatar.h',
	'pidgincolor.h',
	'pidgincontactlist.h',
	'pidgincontactlistwindow.h',
	'pidgincore.h',
	'pidgindisplayitem.h',
	'pidgindisplaywindow.h',
	'pidgindebug.h',
	'pidginiconname.h',
	'pidgininfopane.h',
	'pidgininvitedialog.h',
	'pidginkeypad.h',
	'pidginmessage.h',
	'pidginmooddialog.h',
	'pidginnotificationaddcontact.h',
	'pidginnotificationauthorizationrequest.h',
	'pidginnotificationconnectionerror.h',
	'pidginnotificationlist.h',
	'pidginplugininfo.h',
	'pidginpluginsdialog.h',
	'pidginpluginsmenu.h',
	'pidginpresenceicon.h',
	'pidginprotocolchooser.h',
	'pidginstatusbox.h',
	'pidginstatuseditor.h',
	'pidginstatusmanager.h',
	'pidginstatusprimitivechooser.h',
	'pidgintalkatu.h',
	'pidginui.h',
]

libpidgin_prefs_headers = [
	'prefs/pidginprefs.h',
	'prefs/pidginawayprefs.h',
	'prefs/pidginconversationprefs.h',
	'prefs/pidgincredentialproviderrow.h',
	'prefs/pidgincredentialprefs.h',
	'prefs/pidginnetworkprefs.h',
	'prefs/pidginproxyprefs.h',
	'prefs/pidginvvprefs.h',
]

pidgin_SOURCES = [
	'pidgin.c'
]

pidgin_resource = gnome.compile_resources('pidginresources',
    'resources/pidgin.gresource.xml',
    source_dir : 'resources',
    c_name : 'pidgin')
libpidgin_SOURCES += pidgin_resource

pidgin_filebase = 'pidgin-3'
pidgin_include_base = pidgin_filebase / 'pidgin'

if IS_WIN32
	libpidgin_SOURCES += [
		'pidgin.c',
		'win32/gtkwin32dep.c',
	]

	pidgin_dll_rc = configure_file(
	    input : 'win32/pidgin_dll_rc.rc.in',
	    output : 'pidgin_dll_rc.rc',
	    configuration : version_conf)
	libpidgin_SOURCES += windows.compile_resources(pidgin_dll_rc,
	    include_directories : include_directories('win32')
	)

	pidgin_exe_rc = configure_file(
	    input : 'win32/pidgin_exe_rc.rc.in',
	    output : 'pidgin_exe_rc.rc',
	    configuration : version_conf)

	pidgin_WIN32_RESOURCES = windows.compile_resources(pidgin_exe_rc,
		include_directories : include_directories('win32'))

	pidgin_SOURCES = [
		'win32/winpidgin.c',
		pidgin_WIN32_RESOURCES,
	]
endif

PIDGIN_H_INCLUDES = []
foreach header : libpidgin_headers + libpidgin_prefs_headers
	PIDGIN_H_INCLUDES += f'#include <pidgin/@header@>'
endforeach
pidgin_h_conf = configuration_data()
pidgin_h_conf.set('PIDGIN_H_INCLUDES', '\n'.join(PIDGIN_H_INCLUDES))
pidgin_h = configure_file(input : 'pidgin.h.in',
                          output : 'pidgin.h',
                          configuration : pidgin_h_conf,
                          install : true,
                          install_dir : get_option('includedir') / pidgin_filebase)

install_headers(libpidgin_headers, subdir : pidgin_include_base)
install_headers(libpidgin_prefs_headers, subdir : pidgin_include_base / 'prefs')

_libpidgin_dependencies = [
	glib,
	gplugin_gtk_dep,
	gtk,
	IOKIT,
	json,
	libadwaita,
	math,
	talkatu_dep,
	libpurple_dep,
]

libpidgin_built_sources = [
]

libpidgin_built_headers = [
	pidgin_h,
]

libpidgin_inc = include_directories('.')
libpidgin = shared_library('pidgin3',
    libpidgin_SOURCES + libpidgin_built_sources + libpidgin_built_headers + ['pidginprivate.h'],
    package_revision,
    c_args : ['-DPIDGIN_COMPILATION', '-DG_LOG_USE_STRUCTURED', '-DG_LOG_DOMAIN="Pidgin"'],
    include_directories : [toplevel_inc, include_directories('prefs')],
    version : PURPLE_LIB_VERSION,
    dependencies : _libpidgin_dependencies,
    install : true)
libpidgin_dep = declare_dependency(
    include_directories : [toplevel_inc, libpidgin_inc],
    link_with : libpidgin,
    sources : libpidgin_built_headers,
    dependencies : [gtk, glib, libadwaita, math, talkatu_dep, gplugin_gtk_dep])

pidgin = executable('pidgin3',
    pidgin_SOURCES,
    c_args : ['-DPIDGIN_COMPILATION', '-DG_LOG_USE_STRUCTURED', '-DG_LOG_DOMAIN="Pidgin"'],
    include_directories : toplevel_inc,
    dependencies : IS_WIN32 ? [] : [libpurple_dep, libpidgin_dep],
    win_subsystem : 'windows',
    install : true)

meson.override_dependency(pidgin_filebase, libpidgin_dep)

pkgconfig.generate(
    libpidgin,
    name : 'Pidgin',
    description : 'Pidgin is a GTK based instant messenger application.',
    version : meson.project_version(),
    filebase : pidgin_filebase,
    subdirs : pidgin_filebase,
    # NOTE: Don't use dependencies from subprojects.
    requires : [gtk, libadwaita, libpurple, 'talkatu', 'gplugin-gtk4'],
    variables : [f'plugindir=${libdir}/pidgin-@purple_major_version@'])

PIDGIN_DESKTOP_FILE = 'im.pidgin.Pidgin3.desktop'
i18n = import('i18n')
desktop_file_in = configure_file(
    input : 'data' / (PIDGIN_DESKTOP_FILE + '.in.in'),
    output : PIDGIN_DESKTOP_FILE + '.in',
    configuration : conf)
desktop_file = i18n.merge_file(
    input : desktop_file_in,
    output : PIDGIN_DESKTOP_FILE,
    po_dir : meson.project_source_root() / 'po',
    type : 'desktop',
    install : true,
    install_dir : get_option('datadir') / 'applications')

appdata = i18n.merge_file(
    input : 'data/im.pidgin.Pidgin3.appdata.xml.in',
    output : 'im.pidgin.Pidgin3.appdata.xml',
    po_dir : meson.project_source_root() / 'po',
    install : true,
    install_dir : get_option('datadir') / 'metainfo')

if enable_introspection
	introspection_sources = libpidgin_SOURCES + libpidgin_headers + libpidgin_prefs_headers

	pidgin_gir = gnome.generate_gir(libpidgin,
		sources : introspection_sources,
		header : 'pidgin.h',
		includes : ['GLib-2.0', 'GObject-2.0', 'Gtk-4.0', libpurple_gir[0], 'Talkatu-0.0'],
		namespace : 'Pidgin',
		symbol_prefix : 'pidgin',
		identifier_prefix : 'Pidgin',
		export_packages : f'pidgin-@purple_major_version@',
		nsversion : f'@purple_major_version@.@purple_minor_version@',
		dependencies : [gplugin_dep, gplugin_gtk_dep, talkatu_dep, libpurple_dep],
		install : true,
		extra_args : ['-DPIDGIN_COMPILATION', '--quiet'])
endif

subdir('data')
subdir('pixmaps')
subdir('plugins')
