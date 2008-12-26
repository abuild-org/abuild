TARGETS_lib := shared2
SRCS_lib_shared2 := Shared2.cc
SHLIB_shared2 := 2 1 3
ifdef SKIP_LINK
override LIBS :=
endif
RULES := ccxx
