## Process this file with automake to produce Makefile.in

## Created by Anjuta

INCLUDES = \
	$(WX_CXXFLAGS) \
	$(LIBTORRENTINCL) \
	$(WXAUIINCL) \
	$(WXABOUTDLGINCL) \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" $(PACKAGE_CFLAGS)


AM_CFLAGS =\
	 -Wall @DEBUGFLAGS@\
	 $(WX_CPPFLAGS)

AM_CXXFLAGS = @SSL_CXXFLAGS@


bin_PROGRAMS = bitswash 

bitswash_SOURCES = \
	bitswash.cpp\
	bitswash.h\
	bittorrentsession.cpp \
	bittorrentsession.h \
	mainframe.cpp\
	mainframe.h\
	swashlistctrl.cpp\
	swashlistctrl.h\
	loggerctrl.cpp\
	loggerctrl.h\
	torrentlistctrl.cpp\
	torrentlistctrl.h\
	configuration.cpp\
	configuration.h\
	torrentinfo.cpp\
	torrentinfo.h\
	summarypane.cpp\
	summarypane.h\
	filelistctrl.cpp\
	filelistctrl.h\
	peerlistctrl.cpp\
	peerlistctrl.h\
	trackerlistctrl.cpp\
	trackerlistctrl.h\
	functions.cpp\
	functions.h\
	torrentproperty.cpp\
	torrentproperty.h\
	torrentconfig.cpp\
	torrentconfig.h\
	swashsetting.cpp\
	swashsetting.h\
	autosizeinput.cpp\
	autosizeinput.h\
	urldialog.cpp\
	urldialog.h\
	swashstatbar.cpp\
	swashstatbar.h\
	swashtrayicon.cpp\
	swashtrayicon.h\
	connectionsetting.cpp\
	connectionsetting.h\
	torrentsetting.cpp\
	torrentsetting.h\
	generalsetting.cpp\
	generalsetting.h\
	extensionsetting.cpp\
	extensionsetting.h\
	compat.h


@LIBTORRENTLIB@:
	if [ ! -z "$(LIBTORRENTDIR)" ] ; then \
		make -C $(LIBTORRENTDIR) ; \
	fi

@WXAUILIB@:
	if [ ! -z "$(WXAUIDIR)" ] ; then \
		make \
		CPP="$(CXX)" \
		WX_CONFIG="$(WX_CONFIG_PATH)" \
		WX_CFLAGS="$(WX_CFLAGS)" \
		WX_LIBS="$(WX_LIBS)" \
		-C $(WXAUIDIR); \
	fi

@WXABOUTDLG@:
	if [ ! -z "$(WXABOUTDLGDIR)" ] ; then \
        make \
        CPP="$(CXX)" \
        WX_CONFIG="$(WX_CONFIG_PATH)" \
        WX_CFLAGS="$(WX_CFLAGS)" \
        WX_LIBS="$(WX_LIBS)" \
        -C $(WXABOUTDLGDIR); \
    fi

	
bitswash_LDFLAGS = -l@BOOST_IOSTREAMS_LIB@ -l@BOOST_DATE_TIME_LIB@ -l@BOOST_FILESYSTEM_LIB@ -l@BOOST_THREAD_LIB@ @PTHREAD_LIBS@ @SSL_LDFLAGS@ @SSL_LIBS@

bitswash_LDADD = \
	$(PACKAGE_LIBS) \
	$(LIBTORRENT) \
	$(WXAUI) \
	$(WXABOUTDLGDEP)\
	$(WX_LIBS)

bitswash_DEPENDENCIES = version.h languages.h @LIBTORRENTDEP@ @WXAUIDEP@ @WXABOUTDLGDEP@

mainframe.$(OBJEXT): version.h
bitswash.$(OBJEXT): version.h languages.h

clean-generic:
	for dir in $(LIBTORRENTDIR) $(WXAUIDIR) $(WXABOUTDLGDIR) ; do \
		$(MAKE) -C $${dir} clean ; \
	done
	rm -f version.h
	rm -f languages.h

distclean-generic:
	for dir in $(LIBTORRRENTDIR) ; do\
		$(MAKE) -C $${dir} distclean; \
	done
	-test -z "$(CONFIG_CLEAN_FILES)" || rm -f $(CONFIG_CLEAN_FILES)
	
version.h:
	echo '//Automatic generated file, do not edit' > $@
	echo '' >> $@
	echo '#ifndef __VERSION_H__' >> $@
	echo '#define __VERSION_H__' >> $@
	echo '' >> $@
	echo '#define BITSWASH_VERSION _T("@VERSION@")' >> $@ 

	fp_tmp=@VERSION@ ;\
	fp_tmp=$${fp_tmp//./,} ;\
	fp_tmp=$${fp_tmp},0 ;\
	echo "#define BITSWASH_FINGERPRINT_VERSION $${fp_tmp}" >> $@
	echo '' >> $@

	echo '#endif //__VERSION_H__' >> $@

languages.h:
	echo '//Automatic generated file, do not edit' > $@
	echo '' >> $@
	echo '#ifndef __LANGUAGES_H__' >> $@
	echo '#define __LANGUAGES_H__' >> $@
	echo '' >> $@
	echo 'struct swashlang' >> $@
	echo '{' >> $@
	echo -e "\twxString lang;" >>$@
	echo '};' >> $@

	echo "const swashlang languages[] = {" >> $@
	echo -e "\t{ _T(\"\") }," >> $@ ;\
	for l in $(ALL_LINGUAS) ; do \
		echo -e "\t{ _T(\"$$l\") }," >> $@ ;\
	done
	echo '};' >> $@
	echo '#endif //__LANGUAGES_H__' >> $@

	
