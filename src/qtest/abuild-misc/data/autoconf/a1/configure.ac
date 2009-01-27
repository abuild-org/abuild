AC_PREREQ(2.59)
AC_INIT(a,0)
AC_PROG_CXX
AC_LANG(C++)
AC_CHECK_HEADER(header-b.hh, [HAVE_B=1], [HAVE_B=0])

if test "$HAVE_B" -eq "1"; then
  echo 1>&2 "--> TEST_MESSAGE: header-b.h was found"
else
  echo 1>&2 "--> TEST_MESSAGE: header-b.h was not found"
fi

AC_OUTPUT()
