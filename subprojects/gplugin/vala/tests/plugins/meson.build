LIBRARIES = [
  'basic',
  'dependent',
  'load-exception',
  'load-failed',
  'unload-failed',
  'unload-shutdown',
]

foreach lib : LIBRARIES
	shared_library(f'vala-@lib@-plugin', f'@lib@.vala',
	               name_prefix: '', dependencies: [gplugin_dep, gplugin_vapi])
endforeach

# Add the test plugins path to our environment variable
devenv.append('GPLUGIN_PLUGIN_PATH', meson.current_build_dir())
