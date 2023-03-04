library('gntlastlog', 'lastlog.c',
    c_args : ['-DG_LOG_USE_STRUCTURED', '-DG_LOG_DOMAIN="Gnt-LastLog"'],
    dependencies : [libpurple_dep, libfinch_dep, ncurses, glib],
    name_prefix : '',
    install : true, install_dir : FINCH_PLUGINDIR)

devenv.append('FINCH_PLUGIN_PATH', meson.current_build_dir())
