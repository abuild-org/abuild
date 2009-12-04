# This is a cross compiler but it's in platform-type native, so we
# have to force autoconf to treat it as a cross-compiler.  It will be
# necessary to pass --cross to verify-compiler to verify it.
CONFIGURE_ARGS += --host=non-native

CC = $(abDIR_pseudo-cross)/../fake-compiler

CCPP = $(CC)
CXX = $(CC)
CXXPP = $(CC)

AR = echo test >
RANLIB = @:
PIC_FLAGS =
SHARED_FLAGS =
soname_args =

# move header test will fail
CCXX_GEN_DEPS = @:

include $(abMK)/toolchains/unix_compiler.mk
