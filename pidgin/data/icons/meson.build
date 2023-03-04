ICONS = [
	'im.pidgin.Pidgin3'
]

foreach icon : ICONS
	foreach size : [16, 24, 48, 64, 96, 128, 256, 512]
		path = f'hicolor/@size@x@size@/apps'
		install_data(path / f'@icon@.png',
		    install_dir : get_option('datadir') / 'icons' / path)
	endforeach
	path = 'hicolor/scalable/apps'
	install_data(path / f'@icon@.svg',
	    install_dir : get_option('datadir') / 'icons' / path)
endforeach

# We don't want to install the application icons in multiple places, nor do we
# want to install our scalable icons, so we need to ignore them.
EXCLUDE_FILES = [
]

EXCLUDE_DIRS = [
	'16x16/apps',
	'16x16/actions/scalable',
	'16x16/status/scalable',
	'22x22/actions/scalable',
	'24x24',
	'32x32/actions/scalable',
	'48x48/actions/scalable',
	'48x48/apps',
	'64x64/apps',
	'96x96/apps',
	'128x128/apps',
	'256x256/apps',
	'512x512/apps',
]

install_subdir('hicolor',
    exclude_files : EXCLUDE_FILES,
    exclude_directories : EXCLUDE_DIRS,
    install_dir : get_option('datadir') / 'pidgin/icons')
