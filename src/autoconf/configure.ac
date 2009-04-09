AC_PREREQ(2.59)
AC_INIT([boost-autoconf],0)
dnl  A more customary way to do this in abuild would be to generate an
dnl  abuild interface after-build file rather than a makefile
dnl  fragment, but we generate a makefile fragment here for
dnl  bootstrapping purposes.
AC_CONFIG_FILES([autoconf-bootstrap.mk])
AC_PROG_CC
AC_PROG_CXX

AC_MSG_CHECKING([for gnu make >= 3.81])
make_okay=0
if make --version >/dev/null 2>&1; then
  v=`make --version | grep 'GNU Make' | sed -e 's/.*Make //'`
  maj=`echo $v | cut -d. -f 1`
  min=`echo $v | cut -d. -f 2`
  if test $maj -gt 3 -o '(' $maj -eq 3 -a $min -ge 81 ')'; then
     make_okay=1
  fi
fi
if test "$make_okay" = "1"; then
  AC_MSG_RESULT([yes])
else
  MISSING_MAKE_381=1
  ISSUE_WARNINGS=1
  AC_MSG_RESULT([no])
fi

USE_MSVC=0
AC_ARG_WITH(msvc,
   AS_HELP_STRING([--with-msvc],
		  [indicate that we are using Visual C++]),
   [if test "$withval" != "no"; then USE_MSVC=1; fi])

AC_LANG([C])
if test "$USE_MSVC" = "0"; then
  ACX_PTHREAD
fi
AC_SEARCH_LIBS([connect],[socket],[],[],[])

AC_LANG([C++])
BOOST_TOP=
BOOST_INCLUDE=
BOOST_LIBDIR=
BOOST_LIB_SUFFIX=
BOOST_SP_USE_PTHREADS=0
AC_SUBST([BOOST_INCLUDE])
AC_SUBST([BOOST_LIBDIR])
AC_SUBST([BOOST_LIB_SUFFIX])
AC_SUBST([BOOST_SP_USE_PTHREADS])

AC_ARG_WITH(boost,
   AS_HELP_STRING([--with-boost=DIR],
		  [location of boost installation]),
   [BOOST_TOP=$withval],[])

if test "x$BOOST_TOP" != "x"; then
  if test -d $BOOST_TOP/include -a -d $BOOST_TOP/lib; then
    AC_MSG_CHECKING([for version of boost in $BOOST_TOP])
    BOOST_VERSION=`ls -d $BOOST_TOP/include/boost-* | \
                   tail -1 | sed -e 's,.*/boost-,,'`
    AC_MSG_RESULT([$BOOST_VERSION])
    if test "x$BOOST_VERSION" = "x"; then
      AC_MSG_ERROR([unable to determine version of boost in $BOOST_TOP])
    else
      BOOST_INCLUDE=$BOOST_TOP/include/boost-$BOOST_VERSION
      BOOST_LIBDIR=$BOOST_TOP/lib
      if ! test -f $BOOST_INCLUDE/boost/thread.hpp; then
        AC_MSG_ERROR([unable to find boost/thread.hpp in $BOOST_INCLUDE])
      fi
      CPPFLAGS="$CPPFLAGS -I$BOOST_INCLUDE"
      if test "$USE_MSVC" = "0"; then
	AC_MSG_CHECKING([for boost library suffix])
	changequote(<<,>>)
	BOOST_LIB_SUFFIX=`ls $BOOST_LIBDIR/libboost_thread-*-$BOOST_VERSION.* |\
		    sed -e 's,.*/libboost_thread\(-[^\.]*\)\..*,\1,' | \
		    sort | uniq | grep -v [-]d-$BOOST_VERSION | head -1`
	changequote([,])
	AC_MSG_RESULT([$BOOST_LIB_SUFFIX])
	if test "x$BOOST_LIB_SUFFIX" = "x"; then
	  AC_MSG_ERROR([unable to determine boost library suffix])
	fi
	LDFLAGS="$LDFLAGS -L$BOOST_LIBDIR"
      fi
      HAVE_BOOST=1
    fi
  else
    AC_MSG_ERROR([unable to find any version of boost in $BOOST_TOP])
  fi
fi

define(boost_test_prog,[
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

static void f(boost::asio::io_service& io_service)
{
    boost::asio::ip::udp::socket s(
	io_service,
	boost::asio::ip::udp::endpoint(
	    boost::asio::ip::udp::v4(), 0));
    boost::asio::ip::udp::resolver resolver(io_service);
    boost::asio::ip::udp::resolver::query
	query(boost::asio::ip::udp::v4(), "127.0.0.1", "10000");
    boost::asio::ip::udp::resolver::iterator iterator =
	resolver.resolve(query);
    s.send_to(boost::asio::buffer("test"), *iterator);
}

int main()
{
    boost::asio::io_service io_service;
    boost::thread th(boost::bind(f, boost::ref(io_service)));
    th.join();
    return 0;
}
])

if test "$USE_MSVC" = "0"; then
  dnl  reset HAVE_BOOST and BOOST_LIB_SUFFIX and try a stronger test
  dnl  for whether we have a working boost.

  TRY_BOOST_LIB=
  if test "x$BOOST_LIB_SUFFIX" = "x"; then
    TRY_BOOST_LIB="boost_%-mt boost_%"
  else
    TRY_BOOST_LIB="boost_%$BOOST_LIB_SUFFIX"
  fi

  HAVE_BOOST=0
  BOOST_LIB_SUFFIX=

  for sp_use_pthreads in 0 1; do
    for a in $TRY_BOOST_LIB; do
      if test "$HAVE_BOOST" = "0"; then
	msg="for boost with lib pattern $a"
	OLIBS="$LIBS"
	OCPPFLAGS="$CPPFLAGS"
	if test "$sp_use_pthreads" = "1"; then
	  CPPFLAGS="$CPPFLAGS -DBOOST_SP_USE_PTHREADS"
	  msg="$msg and BOOST_SP_USE_PTHREADS"
	fi
	for b in thread system; do
	  new=`echo $a | sed -e s/%/$b/`
	  LIBS="$LIBS -l$new"
	done
	AC_MSG_CHECKING([$msg])
	AC_LINK_IFELSE(boost_test_prog,[
	  AC_MSG_RESULT(yes)
	  BOOST_LIB_SUFFIX=`echo $a | sed -e s,boost_%,,`
	  HAVE_BOOST=1
	  BOOST_SP_USE_PTHREADS=$sp_use_pthreads
	],
	[
	  AC_MSG_RESULT(no)
	])
	LIBS="$OLIBS"
	CPPFLAGS="$OCPPFLAGS"
      fi
    done
  done
fi

if test "$HAVE_BOOST" = "0"; then
  ISSUE_WARNINGS=1
fi

if test "$ISSUE_WARNINGS" = "1"; then
  echo ""
  echo ""
fi

FATAL_ERRORS=0
if test "x$MISSING_MAKE_381" = "x1"; then
  AC_MSG_WARN([gnu make >= 3.81 is required])
  FATAL_ERRORS=1
fi
if test "$HAVE_BOOST" = "0"; then
  AC_MSG_WARN([unable to find a suitable boost version; >= 1.35 is required])
  FATAL_ERRORS=1
fi

if test "$ISSUE_WARNINGS" = "1"; then
  echo ""
  echo ""
fi

if test "$FATAL_ERRORS" = "1"; then
  AC_MSG_ERROR([some required items are missing; unable to continue])
fi

AC_OUTPUT()
