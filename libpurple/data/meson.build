PURPLE3_URL_HANDLER_DESKTOP = 'purple3-url-handler.desktop'
i18n = import('i18n')
desktop_file_in = configure_file(
    input : PURPLE3_URL_HANDLER_DESKTOP + '.in.in',
    output : PURPLE3_URL_HANDLER_DESKTOP + '.in',
    configuration : conf)
i18n.merge_file(
    input : desktop_file_in,
    output : PURPLE3_URL_HANDLER_DESKTOP,
    po_dir : meson.project_source_root() / 'po',
    type : 'desktop',
    install : true,
    install_dir : get_option('datadir') / 'applications')

schemas_dir = get_option('prefix') / get_option('datadir') / 'glib-2.0' / 'schemas'
settings_schemas = [
  'im.pidgin.Purple.gschema.xml',
]

install_data(settings_schemas, install_dir: schemas_dir)
gnome.post_install(glib_compile_schemas: true)

# Compile the schemas in the current directory; this is only useful for testing
gnome.compile_schemas(depend_files: files(settings_schemas))
testenv.prepend('GSETTINGS_SCHEMA_DIR', meson.current_build_dir())
