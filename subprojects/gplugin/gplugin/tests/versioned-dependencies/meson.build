LIBRARIES = [
  'super-dependent',
  'no-version',
  'exact1',
  'exact2',
  'greater',
  'greater-equal',
  'less',
  'less-equal',
  'bar',
  'baz',
  'fez',
]

foreach lib : LIBRARIES
	shared_library(lib, f'@lib@.c',
	               name_prefix: '', dependencies : [gplugin_dep, GLIB])
endforeach
