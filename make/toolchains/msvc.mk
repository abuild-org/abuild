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

# /Zi enables all debugging information.
DFLAGS = /Zi
OFLAGS = /O2
# /Wall is impractical with msvc because too many system headers
# generate warnings.
WFLAGS =

ifeq ($(ABUILD_PLATFORM_OPTION), debug)
OFLAGS =
endif
ifeq ($(ABUILD_PLATFORM_OPTION), release)
DFLAGS =
endif

ifeq ($(words $(DFLAGS)), 0)
THREAD_FLAGS = /MD
else
THREAD_FLAGS = /MDd
endif

CC = cl /Gy /EHsc /nologo $(THREAD_FLAGS)
CCPP = $(CC) /E
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

clean::
	$(RM) *.pdb *.exe *.ilk
