# This makefile rules fragment supports compilation of C and C++ code
# into static libraries and dynamically linked executables.  Shared
# libraries are not presently supported but will be in the future.
# Please see the file make/README.shared-libraries for details.
#
# Please see rules-help and interface-help below for details on how to
# use these rules.

# ---------------

# Notes to implementors

# CCXX_TOOLCHAIN contains the name of a makefile fragment (without the
# .mk; loaded from abuild-specified search path) that defines the
# functions that these rules use to perform actual compiles.  The best
# place to learn about how these work, in addition to carefully
# reading these notes, is to look at the built-in compiler support
# files included with abuild.  gcc.mk is a good UNIX example, and
# msvc.mk is a good Windows example.  The compiler support file
# provide the following:

#   .LIBPATTERNS: gnu make variable which must contain patterns to
#   match library names so that dependencies on -llib will work.

#   OBJ: the suffix of non-library object files

#   LOBJ: the suffix of library object files; may be the same as OBJ

#   PREPROCESS_c: a command used to invoke the C preprocessor

#   PREPROCESS_cxx: a command used to invoke the C++ preprocessor

#   COMPILE_c: a command used to invoke the C compiler

#   COMPILE_cxx: a command used to invoke the C++ compiler

#   LINK_c: a command used to invoke the linker for a C program that
#   uses no C++ libraries

#   LINK_cxx: a command used to invoke the linker for a C++ program

#   DFLAGS: default debugging flags

#   OFLAGS: default optimization flags

#   WFLAGS: default warning flags

#   CCXX_GEN_DEPS: if it is possible to make COMPILE_c and COMPILE_cxx
#   generate correct dependency information as a side effect of
#   compilation, add the appropriate flags to COMPILE_c and
#   COMPILE_cxx and set CCXX_GEN_DEPS to @: to suppress the running of
#   gen_deps.  Otherwise, don't set this variable, in which case it
#   will default to $(GEN_DEPS).  Abuild requires dependency files
#   that contain an empty rule with each object file depending on all
#   of its dependencies as well as an empty rule for each dependency
#   that depends on nothing.  This way, missing header files will
#   cause the target to rebuild instead of fail.  Our own gen_deps
#   does this, as does gcc's -MP option.

#   IGNORE_TARGETS: optional: a list of targets (object files,
#   libraries, etc.)  that should be ignored when determining whether
#   there are orphan targets.

#   $(call libname,libbase): a function that returns the full name of
#   a library from its base.  For example, $(call libname,moo) would
#   typically return libmoo.a on a UNIX system and moo.lib on a
#   Windows system.

#   $(call shlibname,libbase,major,minor,revision): a function that
#   returns the full name of a shared library from its base, major
#   version, minor version, and revision number.  For example, $(call
#   shlibname,moo,1,2,3) might return libmoo.so.1.2.3 on a UNIX system
#   and moo1.dll on a Windows system.  The version arguments are
#   optional.  Each one must be ignored if the ones before it are
#   omitted.

#   $(call binname,binbase): a function that returns the full name of
#   an executable from its base.  For example, $(call binname,moo)
#   would typically return moo on a UNIX system and moo.exe on a
#   Windows system.

#   $(call include_flags,include-dirs): a function that returns
#   include flags forthe given include directories.  This result
#   should be suitable to passing as flags to the preprocessor and C
#   or C++ compiler.

#   $(call make_obj,compiler,pic,flags,src,obj): a function that uses
#   the given compiler to convert src to obj.  The first argument will
#   always be either $(COMPILE_c) or $(COMPILE_cxx).  The second
#   argument will be 1 if we need position-independent code for shared
#   libraries (or static libraries that might be linked into shared
#   libraries) and empty otherwise.

#   $(call make_lib,objects,libbase): a function that creates a
#   library with the given base name.  Note that libbase has not been
#   passed to $(libname).

#   $(call make_bin,linker,compiler-flags,link-flags,objects,lib-dirs,libs,binbase):
#   a function that generates an executable file with the given base
#   name from the given objects linking from libs that are found from
#   the given libdirs.  The first argument will always be either
#   $(LINK_c) or $(LINK_cxx).  Note that that binbase has not been
#   passed to $(binname).  Compiler support implementors are
#   encouraged to prepend the variable $(LINKWRAPPER) to link
#   statements.  This makes it possible for the user to set
#   LINKWRAPPER to some program that wraps the link step.  Examples of
#   programs that do this include Purify and Quantify.  NOTE: Your
#   make_bin function should do something with the
#   WHOLE_lib_$(libname) variables: either it should link in the whole
#   library or issue an error that it is not supported.  See
#   toolchains/unix_compiler.mk and toolchains/msvc.mk for examples of
#   each case.
#
#   $(call make_shlib,linker,compiler-flags,link-flags,objects,lib-dirs,libs,shlibbase,major,minor,revision):
#   function that creates a library with the given base name.  This
#   function must take the same arguments as make_bin plus the shared
#   library version information.  Compiler support authors are
#   encouraged to prepend the link statement with $(LINKWRAPPER) as
#   with make_bin.

# When preparing to use a specific toolchain, please see comments in
# that toolchain's makefile fragment for any requirements that it may
# have.

# ---------------

rules-help::
	@cat $(ABUILD_TOP)/rules/object-code/ccxx-rules-help.txt

interface-help::
	@cat $(ABUILD_TOP)/rules/object-code/ccxx-interface-help.txt

TARGETS_lib ?=
TARGETS_bin ?=

# Make sure the user has asked for some targets.
ifeq ($(words $(TARGETS_lib) $(TARGETS_bin)), 0)
_qtx_dummy := $(call QTC.TC,abuild,ccxx.mk no targets,0)
$(error No ccxx targets are defined)
endif

# Separate TARGETS_lib into _TARGETS_static_lib and
# _TARGETS_shared_lib
_TARGETS_shared_lib := $(filter-out %:static,$(foreach L,$(TARGETS_lib),$(L)$(if $(filter undefined,$(origin SHLIB_$(L))),:static)))
_TARGETS_static_lib := $(filter-out $(_TARGETS_shared_lib),$(TARGETS_lib))

# Define ccxx_shlibname to call shlibname with the right arguments
define ccxx_shlibname
$(call shlibname,$(1),$(word 1,$(SHLIB_$(1))),$(word 2,$(SHLIB_$(1))),$(word 3,$(SHLIB_$(1))))
endef

# Define ccxx_all_shlibnames to get all variants of the shared library name
define ccxx_all_shlibnames
$(sort $(call shlibname,$(1),$(word 1,$(SHLIB_$(1))),$(word 2,$(SHLIB_$(1))),$(word 3,$(SHLIB_$(1)))) \
       $(call shlibname,$(1),$(word 1,$(SHLIB_$(1))),$(word 2,$(SHLIB_$(1))),) \
       $(call shlibname,$(1),$(word 1,$(SHLIB_$(1))),,) \
       $(call shlibname,$(1),,,))
endef

# Add each target to the "all" and "clean" rules
_static_lib_TARGETS := $(foreach T,$(_TARGETS_static_lib),$(call libname,$(T)))
_shared_lib_TARGETS := $(foreach T,$(_TARGETS_shared_lib),$(call ccxx_shlibname,$(T)))
_lib_TARGETS := $(_static_lib_TARGETS) $(_shared_lib_TARGETS)
_bin_TARGETS := $(foreach T,$(TARGETS_bin),$(call binname,$(T)))

all:: $(_lib_TARGETS) $(_bin_TARGETS)

# Add all local libraries to LIBS and all local library directories to
# LIBDIRS.
ifneq ($(words $(TARGETS_lib)),0)
LIBS := $(filter-out $(LIBS),$(_TARGETS_static_lib) $(_TARGETS_shared_lib)) $(LIBS)
LIBDIRS := $(filter-out $(LIBDIRS),.) $(LIBDIRS)
endif

# Make sure that the user has provided sources for each target.
_UNDEFINED := $(call undefined_vars,\
                $(foreach T,$(TARGETS_lib),SRCS_lib_$(T)) \
		$(foreach T,$(TARGETS_bin),SRCS_bin_$(T)))
ifneq ($(words $(_UNDEFINED)),0)
_qtx_dummy := $(call QTC.TC,abuild,ccxx.mk undefined variables,0)
$(error The following variables are undefined: $(_UNDEFINED))
endif

# Basic compilation functions

DFLAGS ?=
OFLAGS ?=
WFLAGS ?=
XCPPFLAGS ?=
XCFLAGS ?=
XCXXFLAGS ?=
XLINKFLAGS ?=
LINKWRAPPER ?=
LINK_AS_C ?=

ifneq ($(origin LINK_SHLIBS), undefined)
 ifeq (-$(strip $(LINK_SHLIBS))-,--)
  $(error setting LINK_SHLIBS to an empty value no longer works; override LIBS instead)
 else
  $(call deprecate,1.1,LINK_SHLIBS is deprecated; as of version 1.0.3$(_comma) abuild always links shared libraries)
 endif
endif

# These functions expand to the complete list of debug, optimization
# and warning flags that apply to a specific file.  In this case,
# file-specific values override general values.

# Usage: $(call file_dflags,src)
define file_dflags
$(call value_if_defined,DFLAGS_$(call strip_srcdir,$(1)),$(DFLAGS))
endef

# Usage: $(call file_oflags,src)
define file_oflags
$(call value_if_defined,OFLAGS_$(call strip_srcdir,$(1)),$(OFLAGS))
endef

# Usage: $(call file_wflags,src)
define file_wflags
$(call value_if_defined,WFLAGS_$(call strip_srcdir,$(1)),$(WFLAGS))
endef

# Usage: $(call file_dowflags,src)
define file_dowflags
$(call file_dflags,$(1)) $(call file_oflags,$(1)) $(call file_wflags,$(1))
endef

# These functions expand to the complete list of "extra" flags that
# apply to a specific file.  They are, from general to specific:
# XCPPFLAGS, then XCPPFLAGS_file (and similar for CFLAGS and
# CXXFLAGS).  We use $(value ...) to access the file-specific
# variables to avoid the undefined variable warning for each undefined
# variable since not defining these is the usual case.

# Usage: $(call file_cppflags,src)
define file_cppflags
$(call include_flags,$(INCLUDES) $(SRCDIR) .) $(call file_dowflags,$(1)) $(XCPPFLAGS) $(value XCPPFLAGS_$(call strip_srcdir,$(1)))
endef

# Usage: $(call file_cflags,src)
define file_cflags
$(call file_cppflags,$(1)) $(XCFLAGS) $(value XCFLAGS_$(call strip_srcdir,$(1)))
endef

# Usage: $(call file_cxxflags,src)
define file_cxxflags
$(call file_cflags,$(1)) $(XCXXFLAGS) $(value XCXXFLAGS_$(call strip_srcdir,$(1)))
endef

# Usage: $(call use_pic,src): determines the value of pic to pass to make_obj
define use_pic
$(and $(filter $(call strip_srcdir,$(<)), $(_lib_SRCS)), $(if $(value NOPIC_$(call strip_srcdir,$(<))),,1))
endef

LANGNAME_c := C
LANGNAME_cxx := C++
CCXX_GEN_DEPS ?= $(GEN_DEPS)
CCCXX_LINKER = $(if $(LINK_AS_C),$(LINK_c),$(LINK_cxx))
# Usage: $(call ccxx_compile,language): language = { c | cxx }
define ccxx_compile
	@: $(call QTC.TC,abuild,ccxx.mk ccxx_compile,0)
	@mkdir -p $(dir $@)
	$(CCXX_GEN_DEPS) \
		"$(PREPROCESS_$(1)) $(call file_cppflags,$<)" \
		"$<" "$@" "$*.$(DEP)"
	-$(RM) $@
	@$(PRINT) "Compiling $< as $(LANGNAME_$(1))"
	$(call make_obj,$(COMPILE_$(1)),$(call use_pic,$<), \
                        $(call file_$(1)flags,$<),$<,$@)
endef

# Usage: $(call ccxx_preprocess,language): language = { c | cxx }
define ccxx_preprocess
	@mkdir -p $(dir $@)
	@$(PRINT) "Preprocessing $< as $(LANGNAME_$(1)) to $@"
	-$(RM) $@
	$(PREPROCESS_$(1)) $(call file_cppflags,$<) $< > $@
endef

# Usage: $(call ccxx_make_static_lib,library-base)
define ccxx_make_static_lib
	@: $(call QTC.TC,abuild,ccxx.mk ccxx_make_static_lib,0)
	-$(RM) $(call libname,$(1))
	@$(PRINT) "Creating $(1) library"
	$(call make_lib,$(OBJS_lib_$(1)),$(1))
endef

# Usage: $(call ccxx_make_shared_lib,library-base)
define ccxx_make_shared_lib
	@: $(call QTC.TC,abuild,ccxx.mk ccxx_make_shared_lib,0)
	@$(PRINT) "Creating $(1) shared library"
	$(call make_shlib,$(CCCXX_LINKER),$(XCFLAGS) $(XCXXFLAGS) $(DFLAGS) $(OFLAGS) $(WFLAGS),$(XLINKFLAGS),$(OBJS_lib_$(1)),$(LIBDIRS),$(filter-out $(_TARGETS_shared_lib),$(LIBS)),$(1),$(word 1,$(SHLIB_$(1))),$(word 2,$(SHLIB_$(1))),$(word 3,$(SHLIB_$(1))))
endef

# Usage: $(call ccxx_make_bin,executable-base)
define ccxx_make_bin
	@: $(call QTC.TC,abuild,ccxx.mk ccxx_make_bin,0)
	-$(RM) $(call binname,$(1))
	@$(PRINT) "Creating $(1) executable"
	$(call make_bin,$(CCCXX_LINKER),$(XCFLAGS) $(XCXXFLAGS) $(DFLAGS) $(OFLAGS) $(WFLAGS),$(XLINKFLAGS),$(OBJS_bin_$(1)),$(LIBDIRS),$(LIBS),$(1))
endef

c_to_o = $(call ccxx_compile,c)
cxx_to_o = $(call ccxx_compile,cxx)
lib_c_to_o = $(c_to_o)
bin_c_to_o = $(c_to_o)
lib_cxx_to_o = $(cxx_to_o)
bin_cxx_to_o = $(cxx_to_o)
c_to_i = $(call ccxx_preprocess,c)
cxx_to_i = $(call ccxx_preprocess,cxx)

# For each SRCS_lib_x and SRCS_bin_x, create corresponding OBJS_lib_x
# and OBJS_bin_x by transforming all .c, .cc, and .cpp file names to
# object file names.

$(foreach T,$(TARGETS_lib),\
   $(eval OBJS_lib_$(T) := \
       $(call x_to_y,c,$(LOBJ),SRCS_lib_$(T)) \
       $(call x_to_y,cc,$(LOBJ),SRCS_lib_$(T)) \
       $(call x_to_y,cpp,$(LOBJ),SRCS_lib_$(T))))
$(foreach T,$(TARGETS_bin),\
   $(eval OBJS_bin_$(T) := \
       $(call x_to_y,c,$(OBJ),SRCS_bin_$(T)) \
       $(call x_to_y,cc,$(OBJ),SRCS_bin_$(T)) \
       $(call x_to_y,cpp,$(OBJ),SRCS_bin_$(T))))

# Combine all sources from various bases into types (lib and bin) and
# then separate by suffix.  These variables are used for static pattern
# rules to invoke the correct compilation steps for files based on
# suffix and target type.

_lib_SRCS := $(sort $(foreach T,$(TARGETS_lib),$(SRCS_lib_$(T))))
_bin_SRCS := $(sort $(foreach T,$(TARGETS_bin),$(SRCS_bin_$(T))))
_all_SRCS := $(sort $(_lib_SRCS) $(_bin_SRCS))
_lib_COBJS := $(call x_to_y,c,$(LOBJ),_lib_SRCS)
_lib_CCOBJS := $(call x_to_y,cc,$(LOBJ),_lib_SRCS)
_lib_CPPOBJS :=	$(call x_to_y,cpp,$(LOBJ),_lib_SRCS)
_bin_COBJS := $(call x_to_y,c,$(OBJ),_bin_SRCS)
_bin_CCOBJS := $(call x_to_y,cc,$(OBJ),_bin_SRCS)
_bin_CPPOBJS :=	$(call x_to_y,cpp,$(OBJ),_bin_SRCS)
_Cpproc := $(call x_to_y,c,i,_lib_SRCS) $(call x_to_y,c,i,_bin_SRCS)
_CCpproc := $(call x_to_y,cc,i,_lib_SRCS) $(call x_to_y,cc,i,_bin_SRCS)
_CPPpproc :=	$(call x_to_y,cpp,i,_lib_SRCS) $(call x_to_y,cpp,i,_bin_SRCS)

# Make sure ".." doesn't appear in any source file names.
ifneq ($(words $(findstring /../,$(_all_SRCS)) $(filter ../%,$(_all_SRCS))), 0)
_qtx_dummy := $(call QTC.TC,abuild,ccxx.mk ERR .. in srcs,0)
$(error The path component ".." may not appear in any source file names)
endif

# Include dependency files for each source file
_lib_OBJS := $(_lib_COBJS) $(_lib_CCOBJS) $(_lib_CPPOBJS)
_bin_OBJS := $(_bin_COBJS) $(_bin_CCOBJS) $(_bin_CPPOBJS)
_all_DEPS := $(call x_to_y,$(LOBJ),$(DEP),_lib_OBJS) \
	     $(call x_to_y,$(OBJ),$(DEP),_bin_OBJS)

# Remove any extraneous dep files
_extra_deps := $(filter-out $(_all_DEPS),$(wildcard *.$(DEP)))
ifneq ($(words $(_extra_deps)),0)
_qtx_dummy := $(call QTC.TC,abuild,ccxx.mk remove extra deps,0)
DUMMY := $(shell $(PRINT) 1>&2 Removing extraneous $(DEP) files)
DUMMY := $(shell $(RM) $(_extra_deps))
endif

-include $(_all_DEPS)


# Define static pattern rules that invoke the proper compilation
# function for each object file.

$(_lib_COBJS): %.$(LOBJ): %.c
	$(lib_c_to_o)

$(_lib_CCOBJS): %.$(LOBJ): %.cc
	$(lib_cxx_to_o)

$(_lib_CPPOBJS): %.$(LOBJ): %.cpp
	$(lib_cxx_to_o)

$(_bin_COBJS): %.$(OBJ): %.c
	$(bin_c_to_o)

$(_bin_CCOBJS): %.$(OBJ): %.cc
	$(bin_cxx_to_o)

$(_bin_CPPOBJS): %.$(OBJ): %.cpp
	$(bin_cxx_to_o)

$(_Cpproc): %.i: %.c FORCE
	$(c_to_i)

$(_CCpproc): %.i: %.cc FORCE
	$(cxx_to_i)

$(_CPPpproc): %.i: %.cpp FORCE
	$(cxx_to_i)

# Lex rules

LEX := lex
define lex_to_c
	@$(PRINT) Generating $@ from $< with $(LEX)
	$(RM) $@
	$(LEX) -o$@ $<
endef

%.ll.cc: %.ll
	$(lex_to_c)

%.ll.cpp: %.ll
	$(lex_to_c)

%.l.c: %.l
	$(lex_to_c)

# Flex rules that are separate from lex rules

FLEX := flex
define flex_to_c
	@$(PRINT) Generating $@ from $< with $(FLEX)
	$(RM) $@
	$(FLEX) -o$@ $<
endef

%.fl.cc: %.fl
	$(flex_to_c)

%.fl.cpp: %.fl
	$(flex_to_c)

%.fl.c: %.l
	$(flex_to_c)

FlexLexer.%.cc: %.fl
	@$(PRINT) "Generating $@ from $<"
	$(FLEX) -Pyy_$(notdir $(basename $<)) -+ -s -o$@ $<

# Bison rules

BISON := bison
%.tab.cc %.tab.hh: %.yy
	@$(PRINT) "Generating $@ from $<"
	$(BISON) -p $(notdir $(basename $<)) -t -d $<

# Sun RPC rules

RPCGEN := rpcgen
# make sure make does not remove the generated header file
.PRECIOUS: %_rpc.h

%_rpc.h: %.x
	@$(PRINT) Generating $@ from $< with $(RPCGEN)
	$(RM) $@
	$(RPCGEN) -h -o $@ $<

%_rpc_xdr.c: %_rpc.h
	@$(PRINT) Generating $@ from $< with $(RPCGEN)
	$(RM) $@
	$(RPCGEN) -c -o $@ $(SRCDIR)/$*.x
	sed -i -e 's/#include \"\.\.\/$*\.h/#include \"$*_rpc\.h/' $@

%_rpc_svc.c: %_rpc.h
	@$(PRINT) Generating $@ from $< with $(RPCGEN)
	$(RM) $@
	$(RPCGEN) -m -o $@ $(SRCDIR)/$*.x
	sed -i -e 's/#include \"\.\.\/$*\.h/#include \"$*_rpc\.h/' $@

%_rpc_clnt.c: %_rpc.h
	@$(PRINT) Generating $@ from $< with $(RPCGEN)
	$(RM) $@
	$(RPCGEN) -l -o $@ $(SRCDIR)/$*.x
	sed -i -e 's/#include \"\.\.\/$*\.h/#include \"$*_rpc\.h/' $@

# Ensure that we can use -llib dependencies properly.
.LIBPATTERNS ?=
$(foreach PAT,$(.LIBPATTERNS),$(eval vpath $(PAT) $(LIBDIRS)))

# For each library and executable target, create a rule that makes the
# target dependent on its objects.  Also make executable targets
# depend on the libraries in LIBS, which includes local libraries,
# and shared libary targets depend on the static libraries in LIBS.
# In addition, we make local executable targets explicitly depend on
# local library targets.  The reason for doing this as well as adding
# the -llib target for local libraries is that make will not try to
# build the -llib target if it doesn't exist.
l_LIBS = $(foreach L,$(LIBS),-l$(L))
l_not_local_shared = $(foreach L,$(filter-out $(_TARGETS_shared_lib),$(LIBS)),-l$(L))
$(foreach T,$(_TARGETS_static_lib),\
   $(eval $(call libname,$(T)): $(OBJS_lib_$(T)) ; \
	$(call ccxx_make_static_lib,$(T))))
$(foreach T,$(_TARGETS_shared_lib),\
   $(eval $(call ccxx_shlibname,$(T)): $(OBJS_lib_$(T)) $(l_not_local_shared) $(_static_lib_TARGETS); \
	$(call ccxx_make_shared_lib,$(T))))
$(foreach T,$(TARGETS_bin),\
   $(eval $(call binname,$(T)): $(OBJS_bin_$(T)) $(l_LIBS) $(_lib_TARGETS); \
	$(call ccxx_make_bin,$(T))))


# For each local library target x that does not exist, make -lx depend
# on $(call libname,x).  This prevents errors about -lx not existing
# when a binary target is built explicitly from clean.  We avoid
# creating this dependency if the library already exists because
# otherwise make will translate this into a circular dependency when
# it replaces -lx with the actual library file in the rule.
$(foreach T,$(_TARGETS_static_lib),\
   $(eval -l$(T): $(if $(wildcard $(call libname,$(T))),,$(call libname,$(T)))))
$(foreach T,$(_TARGETS_shared_lib),\
   $(eval -l$(T): $(if $(wildcard $(call ccxx_shlibname,$(T))),,$(call ccxx_shlibname,$(T)))))

_all_obj := $(sort $(_lib_OBJS) $(_bin_OBJS))
# The list of all libraries includes static versions of the shared
# libraries as well since on some platforms (Windows), creating a
# shared library also creates a static library of the same name.
_all_lib := $(sort \
   $(foreach T,$(TARGETS_lib),$(call libname,$(T))) \
   $(foreach T,$(_TARGETS_shared_lib),$(call ccxx_all_shlibnames,$(T))))
_all_bin := $(foreach T,$(TARGETS_bin),$(call binname,$(T)))

# Check for and remove orphan targets
IGNORE_TARGETS ?=
_existing_obj := $(sort $(wildcard *.$(OBJ) *.$(LOBJ)))
_extra_obj := $(filter-out $(_all_obj) $(IGNORE_TARGETS),$(_existing_obj))
ifeq ($(words $(_extra_obj)),0)
 # No extra objects found; check for other extra targets.  Check for
 # libraries and shared libraries, and if we can recognize executables
 # as such (they have some recognizable suffix), check for them as
 # well.
 _all_other := $(_all_lib)
 _existing_other := $(sort $(wildcard $(call libname,*) $(call shlibname,*,,,)))
 ifneq ($(call binname,*),*)
  # If executables are recognizable as such
  _all_other += $(_all_bin)
  _existing_other += $(sort $(wildcard $(call binname,*)))
 endif
 _extra_other := $(filter-out $(_all_other) $(IGNORE_TARGETS),$(_existing_other))
 ifneq ($(words $(_extra_other)),0)
  _qtx_dummy := $(call QTC.TC,abuild,ccxx.mk found extra other,0)
  # For all binary and library targets to relink
  DUMMY := $(shell $(PRINT) 1>&2 Extra targets found: removing libraries and binaries)
  DUMMY := $(shell $(RM) $(_extra_other) $(_all_lib) $(_all_bin))
 endif
else
 # Extra object files found; remove all extra objects as well as any
 # library or binary targets which we want to force to be recreated.
 _qtx_dummy := $(call QTC.TC,abuild,ccxx.mk found extra objs,0)
 DUMMY := $(shell $(PRINT) 1>&2 Extra object files found: removing libraries and binaries)
 DUMMY := $(shell $(RM) $(_extra_obj) $(_all_lib) $(_all_bin))
endif

# This "clean" rule only gets run when abuild clean is manually run
# from the output directory itself.  The "clean" rule in the actual
# source directory just removes all the output directories.
clean::
	$(RM) *.$(OBJ) *.$(LOBJ) *.$(DEP) *.i
	$(RM) $(_lib_TARGETS) $(_bin_TARGETS) *.i
	$(RM) *.ll.cc *.ll.cpp *.l.c *.fl.cc *.fl.cpp *.fl.c
	$(RM) FlexLexer.*.cc *.tab.cc *.tab.hh
	$(RM) *_rpc.h *_rpc_xdr.c *_rpc_svc.c *_rpc_clnt.c

# Create a debugging target that shows values of some critical
# variables.
.PHONY: ccxx_debug
ccxx_debug::
	@: $(call QTC.TC,abuild,ccxx.mk ccxx_debug,0)
	@$(PRINT) INCLUDES = $(INCLUDES)
	@$(PRINT) LIBDIRS = $(LIBDIRS)
	@$(PRINT) LIBS = $(LIBS)
