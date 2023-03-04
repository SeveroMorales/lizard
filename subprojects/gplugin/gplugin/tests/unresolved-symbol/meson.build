if host_machine.system() != 'windows'

if compiler.get_id() == 'gcc'
    link_args = '-Wl,--warn-unresolved-symbols'
else
    link_args = []
endif

shared_module('unresolved-symbol', 'unresolved-symbol.c',
	name_prefix : '',
	link_args : link_args,
	dependencies : [gplugin_dep, GLIB])

endif
