TARGETS_lib := shared2
SRCS_lib_shared2 := Shared2.cc
SHLIB_shared2 := 2 1 3
ifndef SKIP_LINK
LINK_SHLIBS := 1
endif
RULES := ccxx
