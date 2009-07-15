CC = $(abDIR_cross)/xcc
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
