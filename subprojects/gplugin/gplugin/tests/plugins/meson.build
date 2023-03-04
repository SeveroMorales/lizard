LIBRARIES = [
  'basic-plugin',
  'dependent-plugin',
  'broken-dependent-plugin',
  'load-failed',
  'load-exception',
  'unload-failed',
  'unload-shutdown',
]

foreach lib : LIBRARIES
	shared_library(lib, f'@lib@.c',
	               name_prefix: '', dependencies : [gplugin_dep, GLIB])
endforeach

subdir('settings')
