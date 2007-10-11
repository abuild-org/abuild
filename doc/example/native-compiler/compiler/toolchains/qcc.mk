.LIBPATTERNS = lib-%
OBJ = o
LOBJ = o
define libname
lib-$(1)
endef
define binname
bin-$(1)
endef
define shlibname
shlib-$(1)$(if $(2),.$(2)$(if $(3),.$(3)$(if $(4),.$(4))))
endef

QCC = echo

DFLAGS =
OFLAGS =
WFLAGS =

# Convention: clear OFLAGS with debug option and DFLAGS with release option.
ifeq ($(ABUILD_PLATFORM_OPTION), debug)
OFLAGS =
endif
ifeq ($(ABUILD_PLATFORM_OPTION), release)
DFLAGS =
endif

PREPROCESS_c = @:
PREPROCESS_cxx = @:
COMPILE_c = $(QCC)
COMPILE_cxx = $(QCC)
LINK_c = $(QCC)
LINK_cxx = $(QCC)
CCXX_GEN_DEPS = @:

# Usage: $(call include_flags,include-dirs)
define include_flags
	$(foreach I,$(1),-I$(I))
endef

# Usage: $(call make_obj,compiler,pic,flags,src,obj)
define make_obj
	$(1) make-obj $(5)
	touch $(5)
endef

# Usage: $(call make_lib,objects,library-filename)
define make_lib
	$(QCC) make-lib $(call libname,$(2))
	touch $(call libname,$(2))
endef

# Usage: $(call make_bin,linker,compiler-flags,linker-flags,objects,libdirs,libs,binary-filename)
define make_bin
	$(1) make-bin $(call binname,$(7))
	touch $(call binname,$(7))
endef

# Usage: $(call make_shlib,linker,compiler-flags,linker-flags,objects,libdirs,libs,shlib-filename,major,minor,revision)
define make_shlib
	$(1) make-bin $(call shlibname,$(7),$(8),$(9),$(10))
	touch $(call shlibname,$(7),$(8),$(9),$(10))
endef
