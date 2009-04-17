# This file implements rules for the use of autoconf in configuring a
# build item.  They are platform-dependent because the generated
# ./configure script depends upon the version of autoconf installed as
# well as m4 files that may be installed on the system.
#
# Please see the autoconf-help.txt for details on using these rules.

AUTOFILES ?=
AUTOCONFIGH ?=
CONFIGURE_ARGS ?=

# Use standard make trick of using a timestamp file for telling make
# when to regenreate files whose modification times may not be updated
# if they are unchanged when their source is changed.  (./configure
# doesn't update modification times of its products if the new product
# is identical to the old product.)
all:: autoconf.stamp

AC_IN := $(AUTOFILES:%=$(SRCDIR)/%.in)
AC_M4_DIR := $(wildcard $(SRCDIR)/m4)
AC_M4_FILES := $(if $(AC_M4_DIR),$(wildcard $(AC_M4_DIR)/*.m4))

AC_CPPFLAGS := $(call include_flags,$(INCLUDES) $(SRCDIR) .) $(XCPPFLAGS)
AC_CFLAGS := $(AC_CPPFLAGS) $(XCFLAGS)
AC_CXXFLAGS := $(AC_CFLAGS) $(XCXXFLAGS)

COMMONDEPS := $(SRCDIR)/configure.ac $(AC_M4_FILES)

ifneq ($(ABUILD_PLATFORM_TYPE),native)
 ifeq ($(words $(filter --host=%,$(CONFIGURE_ARGS))),0)
  CONFIGURE_ARGS += --host=non-native
 endif
endif

AC_SKIP_AUTOHEADER :=
ifeq ($(words $(AUTOCONFIGH)), 0)
  AC_SKIP_AUTOHEADER := :
endif

autoconf.stamp: $(COMMONDEPS) $(AC_IN)
	for i in $(SRCDIR)/configure.ac $(AC_IN); do cp -f $$i .; done
	aclocal -I $(SRCDIR) $(if $(AC_M4_DIR),-I $(SRCDIR)/m4)
	$(AC_SKIP_AUTOHEADER) autoheader -I $(SRCDIR)
	autoconf -I $(SRCDIR)
	CC="$(COMPILE_c) $(AC_CFLAGS)" CXX="$(COMPILE_cxx) $(AC_CXXFLAGS)" \
		./configure $(CONFIGURE_ARGS)
	touch autoconf.stamp

clean::
	$(RM) -r configure autom4te.cache config.log config.status
	$(RM) -r aclocal.m4 $(AUTOCONFIGH) $(AUTOFILES) $(AUTOCONFIGH).in
	$(RM) autoconf.stamp
