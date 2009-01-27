TARGETS_lib := shared1
SRCS_lib_shared1 := Shared1.cc
SHLIB_shared1 := 1 2 3
ifdef SKIP_LINK
override LIBS :=
endif
RULES := ccxx
