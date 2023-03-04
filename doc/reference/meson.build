if get_option('doc')

subdir('libpurple')
subdir('protocols')

if ENABLE_GTK
	subdir('pidgin')
else
	pidgin_doc = []
endif

if enable_consoleui
	subdir('finch')
else
	finch_doc = []
endif

alias_target('doc', libpurple_doc, protocols_doc, pidgin_doc, finch_doc)

endif

