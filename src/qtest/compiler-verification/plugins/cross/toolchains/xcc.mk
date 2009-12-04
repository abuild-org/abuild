CC = $(abDIR_cross)/../fake-compiler
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
