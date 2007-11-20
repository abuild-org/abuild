# Our rules-help target will be run when the user runs abuild
# rules-help if they include this build item in BUILD_ITEM_RULES.

rules-help::
	@echo
	@echo "** Help for users of BUILD_ITEM_RULES=derived-code-generator **"
	@echo
	@echo "You must set these variables:"
	@echo "  DERIVED_CODEGEN_SRC -- name of a C source file to generate"
	@echo "  DERIVED_CODEGEN_HDR -- name of a C header file to generate"
	@echo "  DERIVED_CODEGEN_INFILE -- a file containing a numerical value"
	@echo
	@echo "These rules will generate a source file called"
	@echo "DERIVED_CODEGEN_SRC which will contain a function called"
	@echo "getNumber().  That function will return the number whose"
	@echo "value you have placed in the file named in"
	@echo "DERIVED_CODEGEN_INFILE.  The file DERIVED_CODEGEN_HDR will"
	@echo "contain a prototype for the function."
	@echo
	@echo "You must include DERIVED_CODEGEN_SRC in the list of sources"
	@echo "for one of your build targets in order to have it included in"
	@echo "that target."
	@echo

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

clean::
	$(RM) $(DERIVED_CODEGEN_SRC) $(DERIVED_CODEGEN_HDR)
