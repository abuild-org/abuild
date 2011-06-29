# This file defines rules that are common to virtually all UNIX
# compilers.  It can be used by specific ccxx toolchain
# implementations.

# Users of this file must define the following:

#   CC -- invocation of the C compiler
#   CXX -- invocation of the C++ compiler
#   CCPP -- invocation of the C preprocessor (e.g. $(CC) -E)
#   CXXPP -- invocation of the C++ preprocessor
#   DFLAGS -- default debug flags
#   OFLAGS -- default optimization flags
#   WFLAGS -- default warning flags
#   AR -- command to create a library from object files
#   RANLIB -- command to run over a newly created library
#   PIC_FLAGS -- flags to pass to the compiler when creating
#       position-independent object files
#   SHARED_FLAGS -- flag to pass to the compiler to generate a shared library
#   soname_args -- $(call soname_args,soname) returns arguments to set the
#       soname to be stored in the shared library

# Users of this file may define the following:

#   INCLUDE_FLAG -- flag to pass the compiler to specify an include
#       directory.  Default is -I.  Flag is prepended to a directory
#       without a space between the flag and the directory.
#   SYSTEM_INCLUDE_FLAG -- flag to pass the compiler to specify a
#       system include directory.  Default is $(INCLUDE_FLAGS).  Flag
#       is prepended to a directory without a space between the flag
#       and the directory.

# It is assumed that the "debug" and "release" platform options are
# supported.  We clear OFLAGS and DFLAGS respectively in those cases.

# .so must precede .a so that make's dependency handling will resolve
# these in the same order as the linker tries them.
.LIBPATTERNS = lib%.so lib%.a

OBJ = o
LOBJ = o
define libname
lib$(1).a
endef

# base, major, minor, revision
define shlibname
lib$(1).so$(if $(2),.$(2)$(if $(3),.$(3)$(if $(4),.$(4))))
endef

define binname
$(1)
endef

ifeq ($(ABUILD_PLATFORM_OPTION), debug)
OFLAGS =
endif
ifeq ($(ABUILD_PLATFORM_OPTION), release)
DFLAGS =
endif

PREPROCESS_c = $(CCPP)
PREPROCESS_cxx = $(CXXPP)
COMPILE_c = $(CC)
COMPILE_cxx = $(CXX)
LINK_c = $(CC)
LINK_cxx = $(CXX)

INCLUDE_FLAG ?= -I
SYSTEM_INCLUDE_FLAG ?= $(INCLUDE_FLAG)

# Usage: $(call include_flags,include-dirs)
define include_flags
	$(foreach I,$(1),$(if $(call starts_with_any,$(SYSTEM_INCLUDES),$(I)),$(SYSTEM_INCLUDE_FLAG),$(INCLUDE_FLAG))$(I))
endef

# Usage: $(call make_obj,compiler,pic,flags,src,obj)
define make_obj
	$(1) $(if $(2),$(PIC_FLAGS)) $(3) -c -o $(5) $(4)
endef

# Usage: $(call make_lib,objects,library-base)
define make_lib
	$(AR) $(call libname,$(2)) $(1)
	$(RANLIB) $(call libname,$(2))
endef

# Override this in any user of unix_compiler.mk that supports the
# whole archive concept.  It should provide the linker command line to
# link with an entire archive rather than just the object files that
# the linker determines are used.  This can be useful to support
# libraries that contain object files that are not externally
# referenced but have side effects such as calling static
# initializers.  See gcc.mk for an example.
whole_link_with = -l$(1)

define link_with
$(if $(call value_if_defined,WHOLE_lib_$(1),),\
     $(call whole_link_with,$(1)),\
     -l$(1))
endef

#                        1      2              3          4       5       6    7
# Usage: $(call make_bin,linker,compiler-flags,link-flags,objects,libdirs,libs,binary-base)
define make_bin
	$(LINKWRAPPER) $(1) -o $(call binname,$(7)) $(2) $(4) \
		$(foreach L,$(5),-L$(L)) \
		$(foreach L,$(6),$(call link_with,$(L))) $(3)
endef

#                          1,     2              3          4       5       6    7          8     9     10
# Usage: $(call make_shlib,linker,compiler-flags,link-flags,objects,libdirs,libs,shlib-base,major,minor,revision
define make_shlib
	$(RM) $(call shlibname,$(7)) $(call shlibname,$(7)).*
	$(LINKWRAPPER) $(1) -o $(call shlibname,$(7),$(8),$(9),$(10)) $(2) $(4) \
		$(SHARED_FLAGS) $(call soname_args,$(call shlibname,$(7),$(8))) \
		$(foreach L,$(5),-L$(L)) \
		$(foreach L,$(6),$(call link_with,$(L))) $(3)
	if [ x"$(8)" != x ]; then \
	    ln -s $(call shlibname,$(7),$(8),$(9),$(10)) $(call shlibname,$(7)); \
	fi
	if [ x"$(9)" != x ]; then \
	    ln -s $(call shlibname,$(7),$(8),$(9),$(10)) $(call shlibname,$(7),$(8)); \
	fi
endef
