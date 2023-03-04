LIBRARIES = [
  'basic',
  'dependent',
  'load-exception',
  'load-failed',
  'unload-shutdown',
  'unload-failed',
]

foreach lib : LIBRARIES
	shared_library(f'genie-@lib@-plugin', f'@lib@.gs',
	               name_prefix: '', dependencies : [gplugin_dep, gplugin_vapi])
endforeach

# Add the test plugins path to our environment variable
devenv.append('GPLUGIN_PLUGIN_PATH', meson.current_build_dir())
