TARGETS_lib := lib1 lib2
TARGETS_bin := bin1
SRCS_lib_lib1 := lib1.c
SRCS_lib_lib2 := lib2.c
SRCS_bin_bin1 := bin1.c

# Override DFLAGS, OFLAGS, and WFLAGS globally
DFLAGS := -new-dflags-
OFLAGS := -new-oflags-
WFLAGS := -new-wflags-

SHLIB_lib2 :=
LINK_AS_C := 1

RULES := ccxx
