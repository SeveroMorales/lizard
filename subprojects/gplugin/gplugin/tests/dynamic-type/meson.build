if host_machine.system() != 'windows'

dynamic_type_provider = shared_library('dynamic-type-provider',
	'dynamic-type-provider.c',
	name_prefix : '',
	dependencies : [gplugin_dep, GLIB])

shared_module('dynamic-type-user', 'dynamic-type-user.c',
	name_prefix : '',
	dependencies : [gplugin_dep, GLIB])

endif
