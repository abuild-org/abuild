# Flags that cause gcc to do what gen_deps does
GDFLAGS = -MD -MF $(value *).$(DEP) -MP

DFLAGS = -g
OFLAGS = -O2
WFLAGS = -Wall

CC = gcc $(GDFLAGS)
CCPP = gcc -E
CXX = g++ $(GDFLAGS)
CXXPP = g++ -E

# TEMPORARY: FORCE32
ifdef ABUILD_FORCE_32BIT
 ifeq ($(ABUILD_FORCE_32BIT),1)
  ifeq ($(words $(filter $(shell uname -m),x86_64 ppc64)), 1)
   CC += -m32
   CCPP += -m32
   CXX += -m32
   CXXPP += -m32
  endif
 endif
endif
# END FORCE32

AR = ar cru
RANLIB = ranlib

# Assumes gnu linker
PIC_FLAGS = -fPIC
SHARED_FLAGS = -shared
define soname_args
-Wl,-soname -Wl,$(1)
endef

CCXX_GEN_DEPS = @:

include $(abMK)/toolchains/unix_compiler.mk

# Override whole_link_with
define whole_link_with
-Wl,--whole-archive -l$(1) -Wl,--no-whole-archive
endef
