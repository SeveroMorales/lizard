gestures_SOURCES = [
	'gestures.c',
	'gstroke.h',
	'gstroke-internal.h',
	'stroke.c',
	'stroke-draw.c'
]

gestures = library('gestures', gestures_SOURCES,
    c_args : ['-DG_LOG_USE_STRUCTURED', '-DG_LOG_DOMAIN="PidginPlugin-Gestures"'],
    dependencies : [libpurple_dep, libpidgin_dep, glib],
    name_prefix : '',
    build_by_default : false,  # FIXME: Port to GTK4
    install : false, install_dir : PIDGIN_PLUGINDIR)

devenv.append('PIDGIN_PLUGIN_PATH', meson.current_build_dir())
