spellchk = library('spellchk', 'spellchk.c',
    c_args : ['-DG_LOG_USE_STRUCTURED', '-DG_LOG_DOMAIN="PidginPlugin-SpellCheck"'],
    dependencies : [libpurple_dep, libpidgin_dep, glib],
    name_prefix : '',
    build_by_default: false,
    install : false, install_dir : PIDGIN_PLUGINDIR)

devenv.append('PIDGIN_PLUGIN_PATH', meson.current_build_dir())
