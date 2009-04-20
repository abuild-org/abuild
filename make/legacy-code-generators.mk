# Support for these code generators was added to abuild before plugins
# were implemented.  They should have been implemented as plugins.
# None of this code is exercised in abuild's test suite, though flex
# and bison support are used in its own build.

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

# Flex rules that are separate from lex rules

FLEX := flex
define flex_to_c
	@$(PRINT) Generating $@ from $< with $(FLEX)
	$(RM) $@
	$(FLEX) -o$@ $<
endef

%.fl.cc: %.fl
	$(flex_to_c)

%.fl.cpp: %.fl
	$(flex_to_c)

%.fl.c: %.l
	$(flex_to_c)

FlexLexer.%.cc: %.fl
	@$(PRINT) "Generating $@ from $<"
	$(FLEX) -Pyy_$(notdir $(basename $<)) -+ -s -o$@ $<

# Bison rules

BISON := bison
%.tab.cc %.tab.hh: %.yy
	@$(PRINT) "Generating $@ from $<"
	$(BISON) -p $(notdir $(basename $<)) -t -d $<

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
