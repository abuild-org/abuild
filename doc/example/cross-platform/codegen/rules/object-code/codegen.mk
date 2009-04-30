# Export this variable to the environment so we can access it from
# $(CODEGEN) using the CALCULATE environment variable.  We could also
# have passed it on the command line.
export CALCULATE

generate.cc: $(NUMBERS) $(CODEGEN)
	perl $(CODEGEN) $(SRCDIR)/$(NUMBERS) > $@
