# For this to work, the environment must be set up for Visual C++
# using %VS71CMNTOOLS%\vsvars32.bat (or whatever file is appropriate
# for your version of Microsoft Visual C++).  For details, see the
# abuild documentation.

# .LIBPATTERNS doesn't include %.dll: msvc creates a .lib for each .dll.
.LIBPATTERNS = %.lib
OBJ = obj
LOBJ = obj
define libname
$(1).lib
endef
define binname
$(1).exe
endef
define shlibname
$(1).dll
endef

# General-purpose flags supported by all compiler toolchains
DFLAGS ?=
# /O2 is for generation of faster code.  Other options are available
# for different types of optimization.
OFLAGS ?= /O2
# /Wall is impractical with msvc because too many system headers
# generate warnings.
WFLAGS ?=

# MSVC-specific flags

# /Zi enables debugging and causes debugging information to be written
# to the .pdb file.  We have observed that cl has trouble with long
# path names when invoked without /Zi.  Microsoft support suggests
# that we should use /Zi for all builds, including release builds.
# See http://msdn.microsoft.com/en-us/library/xe4t6fc1.aspx for
# additional discussion.

# /Gy enables function-level linking.

# /nologo suppresses printing of the Visual C++ banner in the output
# of every compilation.

MSVC_GLOBAL_FLAGS = /Zi /Gy /nologo

# /EHsc enables synchronous exception handling and assumes that
# functions declared extern "C" will not throw exceptions.  To compile
# for .NET, use /clr instead of /EHsc.

MSVC_MANAGEMENT_FLAGS = /EHsc

# /MD causes executables and DLLs to be linked against a dynamically
# loaded, multithreaded, runtime environment.  Programs built this way
# will require MSVCRT.dll (or, if debugging is used via /MDd,
# MSVCRTD.dll) at runtime.  Note that MSVCRT.dll is redistributable,
# but MSVCRTD.dll is not.  You could also set MSVC_RUNTIME_FLAGS
# to /MT for a static, multithreaded runtime environment.

MSVC_RUNTIME_FLAGS = /MD

# End users should not change MSVC_RUNTIME_SUFFIX
MSVC_RUNTIME_SUFFIX = d
ifeq ($(ABUILD_PLATFORM_OPTION), debug)
OFLAGS =
endif
ifeq ($(ABUILD_PLATFORM_OPTION), release)
DFLAGS =
MSVC_RUNTIME_SUFFIX =
endif

CC = cl $(MSVC_GLOBAL_FLAGS) $(MSVC_MANAGEMENT_FLAGS) $(MSVC_RUNTIME_FLAGS)$(MSVC_RUNTIME_SUFFIX)
CCPP = $(CC) /E
# /TP forces C++; /GR enables RTTI (runtime type identification)
CXX = $(CC) /TP /GR
CXXPP = $(CXX) /E

PREPROCESS_c = $(CCPP)
PREPROCESS_cxx = $(CXXPP)
COMPILE_c = $(CC)
COMPILE_cxx = $(CXX)
LINK_c = $(CC)
LINK_cxx = $(CC)

define link_with
$(if $(value WHOLE_lib_$(1)),\
    $(error WHOLE_lib is not supported by $(CCXX_TOOLCHAIN)),\
    $(1).lib)
endef

# Usage: $(call include_flags,include-dirs)
define include_flags
	$(foreach I,$(1),/I$(I))
endef

# Usage: $(call make_obj,compiler,pic,flags,src,obj)
define make_obj
	$(1) $(3) /c /Fo$(5) $(4)
endef
# Usage: $(call make_lib,objects,library-filename)
define make_lib
	lib /nologo /OUT:$(call libname,$(2)) $(1)
endef
# Usage: $(call make_bin,linker,compiler-flags,link-flags,objects,libdirs,libs,binary-filename)
define make_bin
	$(LINKWRAPPER) $(1) $(2) $(4) /link /incremental:no \
		/OUT:$(call binname,$(7)) \
		$(foreach L,$(5),/LIBPATH:$(L)) \
		$(foreach L,$(6),$(call link_with,$(L))) $(3)
	if [ -f $(call binname,$(7)).manifest ]; then \
		mt.exe -nologo -manifest $(call binname,$(7)).manifest \
			-outputresource:$(call binname,$(7))\;1; \
	fi
endef

# Usage: $(call make_shlib,linker,compiler-flags,link-flags,objects,libdirs,libs,binary-filename,major,minor,revision)
define make_shlib
	$(RM) $(call shlibname,$(7))
	$(LINKWRAPPER) $(1) $(2) $(4) /LD /Fe$(call shlibname,$(7)) \
		/link /incremental:no \
		$(foreach L,$(5),/LIBPATH:$(L)) \
		$(foreach L,$(6),$(call link_with,$(L))) $(3)
	if [ -f $(call shlibname,$(7)).manifest ]; then \
		mt.exe -nologo -manifest $(call shlibname,$(7)).manifest \
			-outputresource:$(call shlibname,$(7))\;2; \
	fi
endef
