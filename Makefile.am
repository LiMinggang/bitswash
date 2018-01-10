## Process this file with automake to produce Makefile.in
## Created by Anjuta

SUBDIRS = src locale libgeoip
BUNDLE_DIST = libtorrent

bitswashdocdir = ${datadir}/bitswash
bitswashiconsdir = ${datadir}/bitswash/icons
bitswashappdir = ${datadir}/applications
bitswashappicondir = ${datadir}/pixmaps

bitswashappicon_DATA = \
		       bitswash_256x256.png\
		       bitswash_48x48.png

bitswashapp_DATA = \
		   bitswash.desktop

bitswashdoc_DATA = \
	README\
	COPYING\
	AUTHORS\
	ChangeLog\
	INSTALL\
	NEWS

bitswashicons_DATA= \
	icons/bitswash.png \
	icons/bitswash_256x256.png \
	icons/torrent_open.png\
	icons/torrent_move_up.png\
	icons/torrent_move_down.png\
	icons/torrent_remove.png\
	icons/exit.png\
	icons/torrent_open_url.png\
	icons/torrent_start.png\
	icons/option_preference.png\
	icons/torrent_pause.png\
	icons/torrent_stop.png\
	icons/torrent_create.png\
	icons/torrent_properties.png\
	icons/torrent_open_dir.png\
	icons/torrent_remove_data.png\
	icons/settinggeneral.png\
	icons/settingconnection.png\
	icons/settingtorrent.png\
	icons/settingextension.png

EXTRA_DIST = $(bitswashdoc_DATA) $(bitswashicons_DATA) $(bitswashapp_DATA) $(bitswashappicon_DATA)


# Copy all the spec files. Of cource, only one is actually used.
dist-hook: clean
	for d in $(BUNDLE_DIST) ; do \
		if test -d $$d ; then \
			cp -a $$d $(distdir) ;\
		fi; \
	done
	find $(distdir)/libtorrent -name Makefile -print | xargs rm -f
	find $(distdir)/libtorrent -name .deps -print | xargs rm -Rf

	for specfile in *.spec; do \
		if test -f $$specfile; then \
			cp -p $$specfile $(distdir); \
		fi \
	done
# Remove svn data
	find $(distdir) -name '.svn' -print  | xargs rm -Rf

libtorrent-upgrade-test:
	( libtorrent_svn="https://libtorrent.svn.sourceforge.net/svnroot/libtorrent/trunk" ;\
	swversion=`cat libtorrent/libtorrent-svn-revision` ;\
	head_revision="`svn info $${libtorrent_svn} | grep Revision`" ;\
    head_revision=`echo $${head_revision} | $(AWK) -F: '{ print $$2}' ` ;\
	head_revision=`echo $${head_revision}` ;\
	echo "svn diff -r $${swversion}:$${head_revision} $${libtorrent_svn} > libtorrent-patch-$${head_revision}" ;\
	svn diff -r $${swversion}:$${head_revision} $${libtorrent_svn} > libtorrent-patch-$${head_revision} ;\
	echo Updating libtorrent to r$${head_revision} ; \
	)

libtorrent-upgrade:
	( libtorrent_svn="https://libtorrent.svn.sourceforge.net/svnroot/libtorrent/trunk" ;\
	swversion=`cat libtorrent/libtorrent-svn-revision` ;\
	head_revision="`svn info $${libtorrent_svn} | grep Revision`" ;\
    head_revision=`echo $${head_revision} | $(AWK) -F: '{ print $$2}' ` ;\
	head_revision=`echo $${head_revision}` ;\
	echo "svn diff -r $${swversion}:$${head_revision} $${libtorrent_svn} | patch -p0 -d libtorrent --reject-file=libtorrent-reject" ;\
	svn diff -r $${swversion}:$${head_revision} $${libtorrent_svn} | patch -p0 -d libtorrent --reject-file=libtorrent-reject ;\
	[ -e libtorrent/libtorrent-reject ] && vim libtorrent/libtorrent-reject ;\
	echo Updating libtorrent to r$${head_revision} ; \
	echo $${head_revision} > libtorrent/libtorrent-svn-revision ; \
	)
# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
