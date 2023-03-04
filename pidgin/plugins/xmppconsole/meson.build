xmppconsole_resource = gnome.compile_resources('xmppconsoleresources',
    'xmppconsole.gresource.xml',
    c_name : 'xmppconsole')

library('xmppconsole',
    'xmppconsole.c', xmppconsole_resource,
    c_args : ['-DG_LOG_USE_STRUCTURED', '-DG_LOG_DOMAIN="PidginPlugin-XMPPConsole"'],
    dependencies : [libpurple_dep, libpidgin_dep, glib],
    name_prefix : '',
    install : true, install_dir : PIDGIN_PLUGINDIR)

devenv.append('PIDGIN_PLUGIN_PATH', meson.current_build_dir())
