dnl Process this file with autoconf to produce a configure script.
dnl Created by Anjuta application wizard.

AC_INIT(configure.ac)
AM_INIT_AUTOMAKE(bitswash, 0.0.7)
#libtorrent is not using config.h .. guess we've to sticked to it
#AM_CONFIG_HEADER(config.h)
#AM_MAINTAINER_MODE


AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC


# Check for wx
AM_OPTIONS_WXCONFIG
AM_PATH_WXCONFIG(3.1.0, wx31=1, wxcheck26=0 )
WXAUILIB="\$(top_builddir)/compat/wxaui/lib/libwxaui.a"
WXABOUTDLG="\$(top_builddir)/compat/wx/aboutdlgg.a"
if test "$wx28" = 1; then
  AC_DEFINE_UNQUOTED(WXVER, ${WX_VERSION//./}, [WXVERSION wxwidgets version])
  WXAUI=""
  WXAUIDEP=""
  WXAUIINCL=""
  WXAUIDIR=""

  WXABOUTDLGDEP=""
  WXABOUTDLGDIR=""
  WXABOUTDLGINCL=""
fi

if test "$wxcheck26" = 1 ; then 
  AM_PATH_WXCONFIG(2.6.0, wx26=1)
  if test "$wx26" != 1 ; then
	  AC_MSG_ERROR([
		wxWidgets must be installed on your system but wx-config 
		script couldn't be found. Please check that wx-config is 
		in path or specified by --with-wx-config=path flag, the 
		directory where wxWidgets libraries are installed (returned 
		by 'wx-config --libs' command) is in LD_LIBRARY_PATH or
		equivalent variable and wxWidgets version is 2.6.0 or above.
	])

  fi
  AC_DEFINE_UNQUOTED(WXVER, ${WX_VERSION//./}, [WXVERSION wxwidgets version])

  WXAUI="\$(WXAUILIB)"
  WXAUIDEP="\$(WXAUILIB)"
  WXAUIINCL="-I\$(top_builddir)/compat/wxaui/include"
  WXAUIDIR="\$(top_builddir)/compat/wxaui"

  WXABOUTDLGDEP="\$(WXABOUTDLG)"
  WXABOUTDLGDIR="\$(top_builddir)/compat/wx"
  WXABOUTDLGINCL="-I\$(top_builddir)/compat"
fi

dnl make WXAUI and WXAUIDIR visible to Makefiles

AC_SUBST([WXAUILIB])
AC_SUBST([WXAUI])
AC_SUBST([WXAUIDEP])
AC_SUBST([WXAUIDIR])
AC_SUBST([WXAUIINCL])

dnl uses wxaboutdialog
AC_SUBST([WXABOUTDLG])
AC_SUBST([WXABOUTDLGDEP])
AC_SUBST([WXABOUTDLGDIR])
AC_SUBST([WXABOUTDLGINCL])
AC_DEFINE_UNQUOTED(wxUSE_ABOUTDLG, 1, [wxUSE_ABOUTDLG use wxAboutDlg])

AC_PROG_CPP
AC_PROG_CXX

AC_CACHE_SAVE
AC_PROG_INSTALL
AC_PROG_MAKE_SET

dnl Set program prefix 
test "$prefix" = "NONE" && prefix=/usr/local
PREFIX=$prefix
AC_SUBST(PREFIX)
AC_DEFINE_UNQUOTED(PREFIX, "$PREFIX", [PREFIX program prefix])

dnl Set gettext package name
GETTEXT_PACKAGE=bitswash
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [GETTEXT package name])

dnl Add the languages which your application supports here.
ALL_LINGUAS="de_DE ru_RU zh_CN zh_TW"
AM_GLIB_GNU_GETTEXT
AC_SUBST(ALL_LINGUAS)



AC_ARG_WITH(
	    [libtorrent],
	    AS_HELP_STRING([--with-libtorrent=shipped|system],[Use libttorent shipped or installed libtorrent]),
	    [[libtorrent=$withval]],
	    [[libtorrent=shipped]]
)

dnl Check the value for --with-libtorrent switch
AC_MSG_CHECKING([where libtorrent is install])
case "$libtorrent" in
	"shipped")
		AC_MSG_RESULT(shipped)
		;;
	"system")
		AC_MSG_RESULT(system)
		AC_CHECK_LIB(z, main, ,
			     AC_MSG_ERROR([bitswash depends on libtorrent but libtorrent was not found on your system.])
		)
		;;
	"no")
		AC_MSG_RESULT()
		AC_MSG_ERROR([bitswash depends on libtorrent, you must specify either "system" or "shipped".])
		;;
	*)
		AC_MSG_RESULT()
		AC_MSG_ERROR([Unknown libtorrent option "$libtorrent". Use either "system" or "shipped".])
		;;
esac

AM_PROG_LIBTOOL
LIBTORRENTLIB="\$(top_builddir)/libtorrent/src/.libs/libtorrent.a"
if [[ "$libtorrent" = "shipped" ]]; then
enable_shared=no
	LIBTORRENT="\$(LIBTORRENTLIB)"
	LIBTORRENTDEP="\$(LIBTORRENTLIB)"
	#LIBTORRENT="-L\$(top_builddir)/libtorrent/src/.libs -ltorrent"
	LIBTORRENTDIR="\$(top_builddir)/libtorrent"
	LIBTORRENTINCL="-I\$(top_srcdir)/libtorrent/include -I\$(top_srcdir)/libtorrent/include/libtorrent"
	AC_CONFIG_FILES(libtorrent/Makefile libtorrent/src/Makefile libtorrent/include/Makefile libtorrent/test/Makefile libtorrent/libtorrent.pc)
elif [[ "$libtorrent" = "system" ]]; then
	LIBTORRENT="-ltorrent"
	LIBTORRENTDEP=""
	LIBTORRENTDIR=""
	LIBTORRENTINCL=""
fi


dnl make LIBTORRENT and LIBTORRENTDIR visible to Makefiles
AC_SUBST([LIBTORRENTLIB])
AC_SUBST([LIBTORRENTDEP])
AC_SUBST([LIBTORRENT])
AC_SUBST([LIBTORRENTDIR])
AC_SUBST([LIBTORRENTINCL])

dnl Check for pthreads and boost libraries.
ACX_PTHREAD

AX_BOOST_DATE_TIME

dnl check that Boost.DateTime was found:
if test -z "$BOOST_DATE_TIME_LIB"; then
AC_MSG_ERROR([Unable to find Boost.DateTime library, currently this is required.])
fi

AX_BOOST_FILESYSTEM

dnl check that Boost.Filesystem was found:
if test -z "$BOOST_FILESYSTEM_LIB"; then
AC_MSG_ERROR([Unable to find Boost.Filesystem library, currently this is required.])
fi

AX_BOOST_THREAD

dnl check that Boost.Thread was found:
if test -z "$BOOST_THREAD_LIB"; then
AC_MSG_ERROR([Unable to find Boost.Thread library, currently this is required.])
fi

AX_BOOST_REGEX

dnl check that Boost.Regex was found:
if test -z "$BOOST_REGEX_LIB"; then
AC_MSG_RESULT([Unable to find Boost.Regex library, example test_client will not be build.])
BUILD_TESTCLIENT=no;
fi

dnl try different ways of resolving gethostbyname
AC_CHECK_FUNC(gethostbyname, ,
AC_CHECK_LIB(resolv, gethostbyname, ,
AC_CHECK_LIB(nsl, gethostbyname, ,
AC_CHECK_LIB(ws2_32, main, ,
AC_CHECK_LIB(wsock32, main, ,
AC_MSG_ERROR([gethostbyname not found. Stopped.])))))
)

dnl find out what kind of logging to use
AC_ARG_WITH(
[logging],
AS_HELP_STRING([--with-logging=none|default|verbose],[Specify how much logging to use. Default is none.]),
[[logging=$withval]],
[[logging=none]]
)
dnl Check the value for the --with-logging switch
AC_MSG_CHECKING([what form of logging to use])
case "$logging" in
"none")
	AC_MSG_RESULT(none)
	;;
"default")
	AC_MSG_RESULT(default)
	AC_DEFINE(TORRENT_LOGGING,,[define to use some logging])
	;;
"verbose")
	AC_MSG_RESULT(verbose)
	AC_DEFINE(TORRENT_VERBOSE_LOGGING,,[define to use verbose logging])
	;;
*)
	AC_MSG_RESULT()
	AC_MSG_ERROR([Unknown logging option "$logging". Use either "none", "default" or "verbose".])
	;;
esac


dnl Check whether the examples should be build
AC_ARG_ENABLE(
[examples],
AS_HELP_STRING([--enable-examples],[Build the examples. Default is not to build them.]),
[[examples=$enableval]],
[[examples=no]]
)
dnl Check the value of the --with-examples switch
AC_MSG_CHECKING([if the examples should be build])
case "$examples" in
"yes")
	AC_MSG_RESULT(yes)
	EXAMPLESDIR="examples"
	;;
"no")
	AC_MSG_RESULT(no)
	EXAMPLESDIR=""
	;;
"")
	AC_MSG_RESULT(yes (default))
	EXAMPLESDIR="examples"
	;;
*)
	AC_MSG_RESULT()
	AC_MSG_ERROR([Unknown --enable-examples option "$examples". Use either "yes" or "no".])
	;;
esac
dnl make $examples visible to Makefiles
AC_SUBST([EXAMPLESDIR])

	
dnl find out what kind of dht-support to use
AC_ARG_WITH(
	[dht],
	AS_HELP_STRING([--with-dht=on|off|logging],[Specify how to use DHT support. Option logging will add extra logging. Default is on.]),
	[[dht=$withval]],
	[[dht=on]]
)
dnl Check the value for the --with-dht switch
AC_MSG_CHECKING([how to use DHT])
case "$dht" in
	"on")
		AC_MSG_RESULT(on)
		;;
	"off")
		AC_MSG_RESULT(off)
		AC_DEFINE(TORRENT_DISABLE_DHT,,[define not to use DHT support])
		;;
	"logging")
		AC_MSG_RESULT(logging)
		AC_DEFINE(TORRENT_DHT_VERBOSE_LOGGING,,[define to use DHT support with extra logging])
		;;
	*)
		AC_MSG_RESULT()
		AC_MSG_ERROR([Unknown dht option "$dht". Use either "on", "off" or "logging".])
		;;
esac
AM_CONDITIONAL(USE_DHT, test "x$dht" != "xoff")

dnl encryption support.
AC_ARG_WITH(
       [encryption],
       AS_HELP_STRING([--with-encryption=on|off],[Specify how to use encryption support. Default is on.]),
       [[encryption=$withval]],
       [[encryption=on]]
)

dnl Check the value for the --with-encryption switch
AC_MSG_CHECKING([how to use encryption])
case "$encryption" in
       "on")
               AC_MSG_RESULT(on)
                CHECK_SSL()
               AC_DEFINE(TORRENT_USE_OPENSSL,,[define to use openssl with libtorrent])

               ;;
       "off")
               AC_MSG_RESULT(off)
               AC_DEFINE(TORRENT_DISABLE_ENCRYPTION,,[define not to use encryption support])
               ;;
       *)
               AC_MSG_RESULT()
               AC_MSG_ERROR([Unknown encryption option "$encryption". Use either "on" or "off".])
               ;;
esac
AM_CONDITIONAL(USE_ENCRYPTION, test "x$encryption" != "xoff")


dnl the user can choose which zlib to use
AC_ARG_WITH(
	[zlib],
	AS_HELP_STRING([--with-zlib=shipped|system],[Specify the zlib to use, shipped or system. Default is to autodetect system and fallback to shipped.]),
	[[zlib=$withval]],
	[[zlib=detect]]
)
dnl Check the value for the --with-zlib switch
AC_MSG_CHECKING([which zlib implementation to use])
case "$zlib" in
	"detect")
		AC_MSG_RESULT([autodetect])
		AC_CHECK_LIB(z, main,
			[zlib="system"],
			[zlib="shipped"]
		)
		;;
	"shipped")
		AC_MSG_RESULT(shipped)
		;;
	"system")
		AC_MSG_RESULT(system)
		AC_CHECK_LIB(z, main, ,
			AC_MSG_ERROR([libtorrent depends on zlib but zlib was not found on your system.])
		)
		;;
	"no")
		AC_MSG_RESULT()
		AC_MSG_ERROR([libtorrent depends on zlib, you must specify either "system" or "shipped".])
		;;
	*)
		AC_MSG_RESULT()
		AC_MSG_ERROR([Unknown zlib option "$zlib". Use either "system" or "shipped".])
		;;
esac
if [[ "$zlib" = "shipped" ]]; then
	ZLIB="\$(top_builddir)/libtorrent/zlib/libzlib.la"
	ZLIBDIR="zlib"
	ZLIBINCL="-I\$(top_srcdir)/libtorrent/zlib"
	AC_CONFIG_FILES(libtorrent/zlib/Makefile)
elif [[ "$zlib" = "system" ]]; then
	ZLIB="-lz"
	ZLIBDIR=""
	ZLIBINCL=""
fi

dnl make ZLIB and ZLIBDIR visible to Makefiles
AC_SUBST([ZLIB])
AC_SUBST([ZLIBDIR])
AC_SUBST([ZLIBINCL])


#disable example
EXAMPLESDIR=""
dnl make $examples visible to Makefiles
AC_SUBST([EXAMPLESDIR])

dnl want some debugging symbols with that?
AC_ARG_ENABLE(
	[debug],
	AS_HELP_STRING([--enable-debug],[Set compiler flags for debug symbols, default is no debugging.]),
	[case "$enableval" in
		no)
			AC_DEFINE(NDEBUG,,[Define to disable debugging])
			DEBUGFLAGS=""
			;;
		yes)
			AC_DEFINE(__WXDEBUG__,1,[Define to disable debugging])
			DEBUGFLAGS="-g"
			;;
		*)
			DEBUGFLAGS="$enableval"
			;;
	esac],
	[DEBUGFLAGS=""
	AC_DEFINE(NDEBUG,,[Define to disable debugging])]
)
AC_SUBST(DEBUGFLAGS)


AC_OUTPUT([
Makefile
src/Makefile
locale/Makefile
])
