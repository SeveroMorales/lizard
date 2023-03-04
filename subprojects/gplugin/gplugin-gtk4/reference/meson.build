if not get_option('doc')
  subdir_done()
endif

gplugin_gtk4_toml = configure_file(
	input : 'gplugin-gtk4.toml.in',
	output : 'gplugin-gtk4.toml',
	configuration : version_conf,
	install : true,
	install_dir : docs_dir / 'gplugin-gtk4',
)

gplugin_gtk4_doc = custom_target('gplugin-gtk4-doc',
	input : [ gplugin_gtk4_toml, gplugin_gtk4_gir[0] ],
	output : 'gplugin-gtk4',
	command : [
		gidocgen,
		'generate',
		'--quiet',
		'--fatal-warnings',
		'--add-include-path=@0@'.format(meson.project_build_root() / 'gplugin'),
		'--config=@INPUT0@',
		'--output-dir=@OUTPUT@',
		'--no-namespace-dir',
		'--content-dir=@0@'.format(meson.current_source_dir()),
		'@INPUT1@'
	],
	depends : [ gplugin_gir[0] ],
	build_by_default : true,
	install : true,
	install_dir : docs_dir,
)

doc_targets += gplugin_gtk4_doc
