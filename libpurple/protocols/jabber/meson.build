#######################################################################
# Check for Internationalized Domain Name support
#######################################################################

JABBER_SOURCES = [
	'adhoccommands.c',
	'adhoccommands.h',
	'auth.c',
	'auth.h',
	'auth_digest_md5.c',
	'auth_digest_md5.h',
	'auth_plain.c',
	'auth_scram.c',
	'auth_scram.h',
	'auth_webex.c',
	'buddy.c',
	'buddy.h',
	'bosh.c',
	'bosh.h',
	'caps.c',
	'caps.h',
	'chat.c',
	'chat.h',
	'data.c',
	'data.h',
	'disco.c',
	'disco.h',
	'ibb.c',
	'ibb.h',
	'iq.c',
	'iq.h',
	'jabber.c',
	'jabber.h',
	'jingle/jingle.c',
	'jingle/jingle.h',
	'jingle/content.c',
	'jingle/content.h',
	'jingle/iceudp.c',
	'jingle/iceudp.h',
	'jingle/rawudp.c',
	'jingle/rawudp.h',
	'jingle/rtp.c',
	'jingle/rtp.h',
	'jingle/session.c',
	'jingle/session.h',
	'jingle/transport.c',
	'jingle/transport.h',
	'jutil.c',
	'jutil.h',
	'message.c',
	'message.h',
	'namespaces.h',
	'oob.c',
	'oob.h',
	'parser.c',
	'parser.h',
	'pep.c',
	'pep.h',
	'ping.c',
	'ping.h',
	'presence.c',
	'presence.h',
	'roster.c',
	'roster.h',
	'si.c',
	'si.h',
	'useravatar.c',
	'useravatar.h',
	'usermood.c',
	'usermood.h',
	'usernick.c',
	'usernick.h',
	'usertune.c',
	'usertune.h',
	'xdata.c',
	'xdata.h',
	'xmpp.c',
	'xmpp.h'
]

if IS_WIN32
	jabber_link_args = ['-Wl,--export-all-symbols']
else
	jabber_link_args = []
endif

if DYNAMIC_JABBER
	idn = dependency('libidn', version : '>= 0.0.0')

	xmpp_resources = gnome.compile_resources('xmppresource',
		'resources/xmpp.gresource.xml',
		source_dir : 'resources',
		c_name : 'xmpp')
	JABBER_SOURCES += xmpp_resources

	jabber_prpl = shared_library('jabber', JABBER_SOURCES,
	    c_args : ['-DG_LOG_USE_STRUCTURED', '-DG_LOG_DOMAIN="Purple-XMPP"'],
	    link_args : jabber_link_args,
	    dependencies : [gstreamer, idn, libxml, libpurple_dep, libsoup, glib, gio, math, ws2_32],
	    install : true,
	    install_dir : PURPLE_PLUGINDIR)

	devenv.append('PURPLE_PLUGIN_PATH', meson.current_build_dir())

	subdir('tests')
endif
