if not get_option('doc')
  subdir_done()
endif

hasl_doc_content_files = []

hasl_gidocgen_toml = configure_file(
  input : 'hasl.toml.in',
  output : 'hasl.toml',
  configuration : version_config,
  install : true,
  install_dir : docs_dir / 'hasl')

hasl_doc = custom_target('hasl-doc',
  input : [ hasl_gidocgen_toml, hasl_gir[0] ],
  output : 'hasl',
  command : [
    gidocgen,
    'generate',
    '--quiet',
    '--fatal-warnings',
    '--config=@INPUT0@',
    '--output-dir=@OUTPUT@',
    '--no-namespace-dir',
    '--content-dir=@0@'.format(meson.current_source_dir()),
    '@INPUT1@'
  ],
  depend_files : [ hasl_doc_content_files ],
  build_by_default : true,
  install : true,
  install_dir : docs_dir,
)

doc_targets += hasl_doc
