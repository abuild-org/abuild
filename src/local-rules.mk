.PRECIOUS: interface.fl

# Generate .fl from .qfl file.  For some reason, using a pattern rule
# here doesn't work under cygwin, though it does work under Linux with
# make 3.81.
interface.fl: interface.qfl $(SRCDIR)/gen_flex interface.yy
	@$(PRINT) Generating $@ from $<
	perl $(SRCDIR)/gen_flex $< $@

clean-parser-cache:
	$(RM) $(SRCDIR)/parser-cache/interface.*
