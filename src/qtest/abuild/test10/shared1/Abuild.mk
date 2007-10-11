TARGETS_lib := shared1
SRCS_lib_shared1 := Shared1.cc
SHLIB_shared1 := 1 2 3
ifndef SKIP_LINK
LINK_SHLIBS := 1
endif
RULES := ccxx
