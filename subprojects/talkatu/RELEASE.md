# Releases

This document describes how we release Talkatu.

For each release, someone will be chosen to manage the release.  This person
will be referred to as the release manager going forward in this document.

# Prerequisites

The release manager will need access to a few things to perform their duties.

 * Push access to the canonical repository
 * Transifex

# Updating translations

Like nearly all tasks, this should be done via a review request. So in a
development working copy, we want to update the translations to what is
currently on Transifex.

To do this, you need to run the following command from the project root:

```
tx pull --minimum-perc 50 --all --parallel
```

This will update any existing `.po` files and pull in any new ones that are
atleast 50% translated. Any new `.po` files will need to be added to `hg` and
have their language code added to the file `po/LINGUAS`.

Once you have verified all of that, run a build to make sure there are no
immediate issues with the `.po` files. Then post the review request like you
normally would.

# Preparing the Release

Again, we will be creating a review request for the commit that changes the
version number and release date.

For starters, make sure that the date is correct in the ChangeLog and that the
ChangeLog has been updated to reflect what is new in this release.

Update the version number in `meson.build` being sure to remove any extra
version like `-dev` which is not part of the release.

Run a test build via `ninja dist`.

If everything is fine, go ahead and post your review request.

# Landing the Release Commit

Once the above review request for the release has been approved, make sure you
are on the `default` branch and land the review request like any other.

Do a sanity build using `ninja dist`, if everything is good, go ahead and tag
the new release with `hg tag v${VERSION}`.

# Releasing

Update your working copy to the new tag you just created above with
`hg up v${VERSION}`. `cd` to the build directory and run `ninja dist`. This will
create the files `talkatu-${VERSION}.tar.xz` and
`talkatu-${VERSION}.tar.xz.sha256sum` in the `meson-dist` directory inside the
build directory.

Next, `cd` to the `meson-dist` directory so we can sign the files. Signing the
files is easy, just use the following commands.

```
gpg --armor --detach-sign talkatu-${VERSION}.tar.xz
gpg --armor --detach-sign talkatu-${VERSION}.tar.xz.sha256sum
```

Once the signatures are created, you can can `scp` the files up to SourceForge.
Note you will need to create a folder for the version on the SourceForge website
before this.

Make sure you are in the `meson-dist` directory and run the follow command.

```
scp * frs.sourceforge.net:/home/frs/project/pidgin/talkatu/${VERSION}/
```

Finally, update back to the default branch with `hg up default` and publish
the commits with `hg phase --public`. You can now push the release via
`hg push`.

Verify the files show up in SourceForge and you're done!

# Post Release

Once you've pushed the release commit, you need to updated a few more things
for the next round of development.

First make sure you are in a development check out. Next determine what the next
version will be.  Most likely it'll be a micro release.

Next add a new line to `ChangeLog` with the following

```
${NEW_VERSION}: ????/??/??
  * Nothing yet, be the first!
```

Next, update `meson.build` for the new version and append a `-dev` on the end
of it.

Finally submit these changes as a review request.
