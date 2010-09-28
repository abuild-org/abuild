TARGETS_lib := lib1 lib2
TARGETS_bin := bin1 bin2
SRCS_lib_lib1 := lib1-src1.c lib1-src2.cc lib1-src3.cpp
SRCS_lib_lib2 := lib2.cc
SRCS_bin_bin1 := bin1-src1.c bin1-src2.cc bin1-src3.cpp
SRCS_bin_bin2 := bin2.cc

# Disable PIC for one file
NOPIC_lib1-src2.cc := 1

# Override DFLAGS, OFLAGS, and WFLAGS separately for each file in lib1
DFLAGS_lib1-src1.c := -lib1-src1-dflags-
OFLAGS_lib1-src2.cc := -lib1-src2-oflags-
WFLAGS_lib1-src3.cpp := -lib1-src3-wflags-

# Override DFLAGS, OFLAGS, and WFLAGS together for bin2
DFLAGS_bin2.cc := -bin2-dflags-
OFLAGS_bin2.cc := -bin2-oflags-
WFLAGS_bin2.cc := -bin2-wflags-

# Extend all extra flags for lib1-src1.c; CXX flags ignored
XCPPFLAGS_lib1-src1.c := -lib1-src1-xcppflags-
XCFLAGS_lib1-src1.c := -lib1-src1-xcflags-
XCXXFLAGS_lib1-src1.c := -not-seen-

# Extend all extra flags for bin2.cc
EXPANSION_TEST = bin2-
XCPPFLAGS_bin2.cc = -$(EXPANSION_TEST)xcppflags-
XCFLAGS_bin2.cc = -$(EXPANSION_TEST)xcflags-
XCXXFLAGS_bin2.cc = -$(EXPANSION_TEST)xcxxflags-

# Overall extra flags
XCPPFLAGS += -xcppflags-
XCFLAGS += -xcflags-
XCXXFLAGS += -xcxxflags-
XLINKFLAGS += -xlinkflags-

RULES := ccxx
