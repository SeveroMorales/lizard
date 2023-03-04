GPlugin is a GObject based library that implements a reusable plugin system
that supports loading plugins in other languages via loaders.  GPlugin also
implements dependencies among the plugins.

It was started due to the infamous "Plugin Problem" for Guifications 3, which
was that I needed plugins that could depend on each other, be able to be
written in other languages, have plugins that are loaded before the main load
phase, and allow plugins to register types into the GObject type system.

After trying to fix this numerous times in the Guifications 3 tree, I decided
I should really do this the UNIX way and write it as a stand alone library.

During this time, I also got the idea that there was a good chance that someone
probably beat me to this...  So I started searching and came across libpeas.

Libpeas looked promising, but there was a few things about it that I really
didn't care for.  First of all, the plugin information (id, name, author, etc)
are in a separate data file that gets stored in a separate location on disk.

Getting people to write good plugins is difficult enough as it is, why bother
throwing more obstacles in their way?  This data file is a essentially a
GKeyFile, which means you can store the translations of all the fields right in
it.  Now this is great if your plugin doesn't have any strings to display at
runtime, but if it does, you still need to install the translation file itself.
So as long as your plugin has to display strings at runtime, all that data file
gave you was more work.  So there was STRIKE ONE!

So I continued to look at libpeas and noticed something odd in the earlier
mentioned data file.  One of the fields you have to set is the Loader!?  Yes,
the libpeas authors explicitly expect you to call out which loader you need.  I
personally find this to be very lazy.  I realize it makes it easier to code,
but come on, that should be a one time cost to the library author, instead of
an additional startup cost to the plugin author.  STRIKE TWO!

So with two strikes down, I continued researching libpeas, and discovered that
you can only use one language other than the native loader.  We, the Pidgin,
developers have always aimed to support as many languages as possible, and this
just wasn't going to cut it.  So there was STRIKE THREE!

Libpeas had struck out for me, and as such I started GPlugin the very next day!

