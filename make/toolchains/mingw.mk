# Flags that cause gcc to do what gen_deps does
GDFLAGS = -MD -MF $(value *).$(DEP) -MP

DFLAGS = -g
OFLAGS = -O2
WFLAGS = -Wall

CC = gcc -mno-cygwin $(GDFLAGS)
CCPP = gcc -mno-cygwin -E
CXX = g++ -mno-cygwin $(GDFLAGS)
CXXPP = g++ -mno-cygwin -E

AR = ar cru
RANLIB = ranlib

# With mingw, all code is position-independent
PIC_FLAGS =
SHARED_FLAGS = -shared
soname_args =

CCXX_GEN_DEPS = @:

include $(abMK)/toolchains/unix_compiler.mk

# Override whole_link_with
define whole_link_with
-Wl,--whole-archive -l$(1) -Wl,--no-whole-archive
endef

# Override shared library information
define shlibname
$(1).dll
endef
.LIBPATTERNS := %.dll lib%.a
# Usage: $(call make_shlib,linker,compiler-flags,link-flags,objects,libdirs,libs,shlib-base,major,minor,revision
define make_shlib
	$(RM) $(call shlibname,$(7))
	$(LINKWRAPPER) $(1) -o $(call shlibname,$(7),$(8),$(9),$(10)) $(2) $(4) \
		$(SHARED_FLAGS) \
		$(foreach L,$(5),-L$(L)) \
		$(foreach L,$(6),$(call link_with,$(L))) $(3)
endef
