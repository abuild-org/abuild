TARGETS_bin := main
DERIVED_CODEGEN_SRC := auto.c
DERIVED_CODEGEN_HDR := auto.h
DERIVED_CODEGEN_INFILE := number
SRCS_bin_main := main.cpp $(DERIVED_CODEGEN_SRC)
RULES := ccxx
