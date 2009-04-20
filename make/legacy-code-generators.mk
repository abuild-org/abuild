# Support for these code generators was added to abuild before plugins
# were implemented.  They should have been implemented as plugins.
# None of this code is exercised in abuild's test suite, though flex
# and bison support are used in its own build.

# Flex rules that are separate from lex rules

FLEX := flex
ifdef FLEX_CACHE
 define flex_to_c
	@$(PRINT) Generating $@ from $< with $(FLEX)
	$(RM) $@
	$(CODEGEN_WRAPPER) --cache $(FLEX_CACHE) \
	    --input $< --output $@ --command \
	    $(FLEX) -o$@ $<
 endef
else
 define flex_to_c
	@$(PRINT) Generating $@ from $< with $(FLEX)
	$(RM) $@
	$(FLEX) -o$@ $<
 endef
endif

%.fl.cc: %.fl
	$(flex_to_c)

%.fl.cpp: %.fl
	$(flex_to_c)

%.fl.c: %.l
	$(flex_to_c)

ifdef FLEX_CACHE
 FlexLexer.%.cc: %.fl FlexLexer.h
	@$(PRINT) "Generating $@ from $<"
	$(CODEGEN_WRAPPER) --cache $(FLEX_CACHE) \
	    --input $< --output $@ --command \
	    $(FLEX) -Pyy_$(notdir $(basename $<)) -+ -s -o$@ $<

else
 FlexLexer.%.cc: %.fl
	@$(PRINT) "Generating $@ from $<"
	$(FLEX) -Pyy_$(notdir $(basename $<)) -+ -s -o$@ $<
endif

# Bison rules

BISON := bison
ifdef BISON_CACHE
 %.tab.cc %.tab.hh: %.yy
	@$(PRINT) "Generating $@ from $<"
	$(CODEGEN_WRAPPER) --cache $(BISON_CACHE) \
	    --input $< --output $(basename $@).cc $(basename $@).hh \
	    --command $(BISON) -p $(notdir $(basename $<)) -t -d $<
else
 %.tab.cc %.tab.hh: %.yy
	@$(PRINT) "Generating $@ from $<"
	$(BISON) -p $(notdir $(basename $<)) -t -d $<
endif

# Lex rules

LEX := lex
define lex_to_c
	@$(PRINT) Generating $@ from $< with $(LEX)
	$(RM) $@
	$(LEX) -o$@ $<
endef

%.ll.cc: %.ll
	$(lex_to_c)

%.ll.cpp: %.ll
	$(lex_to_c)

%.l.c: %.l
	$(lex_to_c)

# Sun RPC rules

RPCGEN := rpcgen
# make sure make does not remove the generated header file
.PRECIOUS: %_rpc.h

%_rpc.h: %.x
	@$(PRINT) Generating $@ from $< with $(RPCGEN)
	$(RM) $@
	$(RPCGEN) -h -o $@ $<

%_rpc_xdr.c: %_rpc.h
	@$(PRINT) Generating $@ from $< with $(RPCGEN)
	$(RM) $@
	$(RPCGEN) -c -o $@ $(SRCDIR)/$*.x
	sed -i -e 's/#include \"\.\.\/$*\.h/#include \"$*_rpc\.h/' $@

%_rpc_svc.c: %_rpc.h
	@$(PRINT) Generating $@ from $< with $(RPCGEN)
	$(RM) $@
	$(RPCGEN) -m -o $@ $(SRCDIR)/$*.x
	sed -i -e 's/#include \"\.\.\/$*\.h/#include \"$*_rpc\.h/' $@

%_rpc_clnt.c: %_rpc.h
	@$(PRINT) Generating $@ from $< with $(RPCGEN)
	$(RM) $@
	$(RPCGEN) -l -o $@ $(SRCDIR)/$*.x
	sed -i -e 's/#include \"\.\.\/$*\.h/#include \"$*_rpc\.h/' $@
