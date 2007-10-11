.LIBPATTERNS = shlib-% lib-%
OBJ := obj
LOBJ := obj
define libname
lib-$(1)
endef
define binname
print-$(1)
endef
define shlibname
shlib-$(1)$(if $(2),.$(2)$(if $(3),.$(3)$(if $(4),.$(4))))
endef

ABC := $(abDIR_plugin.printer)/bin/abc
ABCLINK := $(abDIR_plugin.printer)/bin/abc-link

DFLAGS :=
OFLAGS :=
WFLAGS :=

PREPROCESS_c := @:
PREPROCESS_cxx := @:
COMPILE_c := $(ABC)
COMPILE_cxx := $(ABC)
LINK_c := $(ABCLINK)
LINK_cxx := $(ABCLINK)
CCXX_GEN_DEPS := @:

# Usage: $(call include_flags,include-dirs)
define include_flags
	$(foreach I,$(1),-I$(I))
endef

# Usage: $(call make_obj,compiler,pic,flags,src,obj)
define make_obj
	$(1) $(3) -c $(4) -o $(5)
endef

# Usage: $(call make_lib,objects,library-filename)
define make_lib
	cat $(1) > $(call libname,$(2))
endef

# Usage: $(call make_bin,linker,compiler-flags,linker-flags,objects,libdirs,libs,binary-filename)
define make_bin
	$(1) $(2) $(3) $(foreach I,$(4),-o $(I)) \
		   $(foreach I,$(5),-L $(I)) \
		   $(foreach I,$(6),-l $(I)) \
		   -b $(call binname,$(7))
endef

# Usage: $(call make_shlib,linker,compiler-flags,linker-flags,objects,libdirs,libs,shlib-filename,major,minor,revision)
define make_shlib
	$(1) $(2) $(3) $(foreach I,$(4),-o $(I)) \
		   $(foreach I,$(5),-L $(I)) \
		   $(foreach I,$(6),-l $(I)) \
		   -b $(call shlibname,$(7),$(8),$(9),$(10))
endef
