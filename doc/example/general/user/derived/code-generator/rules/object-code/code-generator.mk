# Make sure that the user has provided values for all variables
_UNDEFINED := $(call undefined_vars,\
		DERIVED_CODEGEN_HDR \
		DERIVED_CODEGEN_HDR \
		DERIVED_CODEGEN_INFILE)
ifneq ($(words $(_UNDEFINED)),0)
$(error The following variables are undefined: $(_UNDEFINED))
endif

all:: $(DERIVED_CODEGEN_HDR) $(DERIVED_CODEGEN_SRC)

$(DERIVED_CODEGEN_SRC) $(DERIVED_CODEGEN_HDR): $(DERIVED_CODEGEN_INFILE) \
				$(abDIR_derived-code-generator)/gen_code
	@$(PRINT) Generating $(DERIVED_CODEGEN_HDR) \
		and $(DERIVED_CODEGEN_SRC)
	perl $(abDIR_derived-code-generator)/gen_code \
		$(DERIVED_CODEGEN_HDR) \
		$(DERIVED_CODEGEN_SRC) \
		$(SRCDIR)/$(DERIVED_CODEGEN_INFILE)
