# This file implements rules for the use of autoconf in configuring a
# build item.  They are platform-dependent because the generated
# ./configure script depends upon the version of autoconf installed as
# well as m4 files that may be installed on the system.
#
# Please see the help targets below for details on using these rules.

rules-help::
	@echo
	@echo "** Help for users of RULES=autoconf **"
	@echo
	@echo "These rules can be used by build items that require some"
	@echo "autoconf-based configuration either internally for to"
	@echo "provide information for their users."
	@echo
	@echo "In order for these rules to work, the following conventions"
	@echo "must be followed:"
	@echo
	@echo " - The configure input file must be called configure.ac"
	@echo
	@echo " - If the AUTOCONFIGH variable is defined, configure.ac must"
	@echo "   include the statement AC_CONFIG_HEADERS([header.h])"
	@echo "   where header.h is the value of the AUTOCONFIGH.  The"
	@echo "   header file must be named in such a way as to avoid naming"
	@echo "   clashes with those created by other build items."
	@echo
	@echo " - Any custom m4 macros used by the configure.ac script are"
	@echo "   in a directory called m4 and end with the extension .m4"
	@echo
	@echo "The following variables may be defined in Abuild.mk:"
	@echo
	@echo "  AUTOFILES: must be set to the list of files that appear in"
	@echo "      AC_CONFIG_FILES in configure.ac"
	@echo
	@echo "  AUTOCONFIGH: must be set to the name of the header file"
	@echo "      in AC_CONFIG_HEADERS in configure.ac"
	@echo
	@echo "Additionally, the following variable may be set if needed:"
	@echo
	@echo "  CONFIGURE_ARGS: additional arguments to be passed to"
	@echo "      ./configure"
	@echo

interface-help::
	@echo
	@echo "** Help for Abuild.interface for autoconf build items **"
	@echo
	@echo "Generally, Abuild.interface files for autoconf-based build"
	@echo "items will assign to INCLUDES for the benefit of any C/C++"
	@echo "build items that use this.  See interface-help for the ccxx"
	@echo "rule set for details.  It is also common for autoconf-based"
	@echo "build items to generate an autoconf.interface file to be"
	@echo "declared in Abuild.interface as an autofile.  This allows"
	@echo "additinal variables to be set based on the output of autoconf."
	@echo

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
	cp -f $(SRCDIR)/configure.ac configure.ac
	for i in $(AUTOFILES); do cp -f $(SRCDIR)/$$i.in .; done
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
