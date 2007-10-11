# Flags that cause gcc to do what gen_deps does
GDFLAGS = -MD -MF $(value *).$(DEP) -MP

# XXX Add -Werror; must first disable warnings on flex, bison, and
# rpcgen files
DFLAGS = -g
OFLAGS = -O2
WFLAGS = -Wall

CC = gcc $(GDFLAGS)
CCPP = gcc -E
CXX = g++ $(GDFLAGS)
CXXPP = g++ -E

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
