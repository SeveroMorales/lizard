libsecret_plugin = library('libsecret', 'libsecret.c',
    c_args : ['-DG_LOG_USE_STRUCTURED', '-DG_LOG_DOMAIN="Purple-LibSecret"'],
    dependencies : [libsecret, libpurple_dep],
    name_prefix : '',
    install : true, install_dir : PURPLE_PLUGINDIR)

devenv.append('PURPLE_PLUGIN_PATH', meson.current_build_dir())
