MODULES = [
  'multiple-semantic-1.0.0',
  'multiple-semantic-1.1.0',
  'no-version-and-semantic-no-version',
  'no-version-and-semantic-semantic',
  'non-semantic-and-semantic-non-semantic',
  'non-semantic-and-semantic-semantic',
  'solo-no-version',
  'solo-non-semantic',
]

foreach module : MODULES
	shared_module(module, f'@module@.c',
	              name_prefix: '', dependencies : [gplugin_dep, GLIB])
endforeach
