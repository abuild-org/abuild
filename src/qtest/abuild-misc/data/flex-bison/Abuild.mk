TARGETS_lib := flex-bison
SRCS_lib_flex-bison := \
    fbtest.fl.cc \
    fbtest.tab.cc

fbtest.fl.cc: fbtest.tab.hh

WFLAGS_fbtest.fl.cc :=
WFLAGS_fbtest.tab.cc :=

ifndef IGNORE_CACHE
FLEX_CACHE = fcache
BISON_CACHE = bcache
endif

RULES := ccxx
