if not get_option('doc')
  subdir_done()
endif

gplugin_doc_content_files = [
	'embedding.md',
	'genie.md',
	'lua.md',
	'native-plugins.md',
	'python3.md',
	'vala.md',
]

gplugin_toml = configure_file(
	input : 'gplugin.toml.in',
	output : 'gplugin.toml',
	configuration : version_conf,
	install : true,
	install_dir : docs_dir / 'gplugin',
)

gplugin_doc = custom_target('gplugin-doc',
	input : [ gplugin_toml, gplugin_gir[0] ],
	output : 'gplugin',
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
	depend_files : [ gplugin_doc_content_files ],
	build_by_default : true,
	install : true,
	install_dir : docs_dir,
)

doc_targets += gplugin_doc
