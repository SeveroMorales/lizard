if get_option('nls')
	i18n.gettext(GETTEXT_PACKAGE, preset: 'glib')
else
	summary('translations',
	        'You have disabled installation of translations which means ' +
	        'English will be the only available language.',
	        section : 'Warnings')
endif
