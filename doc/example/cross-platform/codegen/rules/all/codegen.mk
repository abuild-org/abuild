# Export this variable to the environment so we can access it from
# $(CODEGEN) using the CALCULATE environment variable.  We could also
# have passed it on the command line.
export CALCULATE

#XXXX-HELP
rules-help::
	@echo Set NUMBERS to the name of a file that contains a list of
	@echo numbers, one per line, to pass to the generator.  The file
	@echo generate.cc will be generated.

generate.cc: $(NUMBERS) $(CODEGEN)
	perl $(CODEGEN) $(SRCDIR)/$(NUMBERS) > $@
