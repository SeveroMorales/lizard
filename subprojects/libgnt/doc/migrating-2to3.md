Title: Migrating from GNT 2.x to 3.0
Slug: 3.0 Migration

## Migrating from GNT 2.x to 3.0

GNT 3 is a major new version of GNT that breaks both API and ABI compared to
GNT 2.x. There are a number of steps that you can take to prepare your GNT 2.x
application for the switch to GNT 3. After that, there's a small number of
adjustments that you may have to do when you actually switch your application
to build against GNT 3.

### Preparation in GNT 2.x

The steps outlined in the following sections assume that your application is
working with GNT 2.14, which is the final stable release of GNT 2.x. It
includes all the necessary APIs and tools to help you port your application
to GNT 3. If you are still using an older version of GNT 2.x, you should first
get your application to build and work with the latest minor release in the
2.14 series.

#### Do not use deprecated symbols

Over the years, a number of functions have been deprecated. These deprecations
are clearly spelled out in the API reference, with hints about the recommended
replacements.

To verify that your program does not use any deprecated symbols, you can use
preprocessor defines to remove deprecated symbols from the header files. As
part of your compilation process, you may define `GNT_DISABLE_DEPRECATED` to
hide deprecated symbols as well as `GNTSEAL_ENABLE` to hide internal struct
members.

Note that some parts of our API, such as enumeration values, are not well
covered by the deprecation warnings. In most cases, using them will require you
to also use deprecated functions, which will trigger warnings.

### Changes that need to be done at the time of the switch

This section outlines porting tasks that you need to tackle when you get to the
point that you actually build your application against GNT 3. Making it
possible to prepare for these in GNT 2 would have been either impossible or
impractical.

#### Removed internal API

The following tag names have been removed from the API entirely; use the name
without the underscore instead:

 * `enum _GntFileType`
 * `enum _GntMouseEvent`
 * `enum _GntParamFlags`
 * `enum _GntProgressBarOrientation`
 * `enum _GntTreeColumnFlag`

The following items were removed entirely:

 * `enum _GntKeyPressMode`
 * `GntKeyPressMode`
 * `enum _GntWidgetFlags`
 * `GntWidgetFlags`

#### Changed API

The following items have been changed as part of the API break:

 * `ENTRY_CHAR` renamed to [const@Gnt.ENTRY_CHAR]
 * `g_hash_table_duplicate` renamed to [func@Gnt.hash_table_duplicate]
 * `GDupFunc` renamed to [callback@Gnt.DuplicateFunc]
 * `GntTreeHashFunc` removed in favor of [callback@GLib.HashFunc]
 * `GntTreeHashEqualityFunc` removed in favor of [callback@GLib.EqualFunc]

