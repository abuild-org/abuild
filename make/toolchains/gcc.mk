# Flags that cause gcc to do what gen_deps does
GDFLAGS = -MD -MF $(value *).$(DEP) -MP

DFLAGS ?= -g
OFLAGS ?= -O2
WFLAGS ?= -Wall

CC = gcc $(GDFLAGS)
CCPP = gcc -E
CXX = g++ $(GDFLAGS)
CXXPP = g++ -E

ifeq ($(call value_if_defined,ABUILD_FORCE_32BIT,0),1)
 CC += -m32
 CCPP += -m32
 CXX += -m32
 CXXPP += -m32
endif
ifeq ($(call value_if_defined,ABUILD_FORCE_64BIT,0),1)
 CC += -m64
 CCPP += -m64
 CXX += -m64
 CXXPP += -m64
endif

AR = ar cru
RANLIB = ranlib

# Assumes gnu linker
PIC_FLAGS = -fPIC
SHARED_FLAGS = -shared
define soname_args
-Wl,-soname -Wl,$(1)
endef
SYSTEM_INCLUDE_FLAG := -isystem$(_space)

CCXX_GEN_DEPS = @:

include $(abMK)/toolchains/unix_compiler.mk

# Override whole_link_with
define whole_link_with
-Wl,--whole-archive -l$(1) -Wl,--no-whole-archive
endef
