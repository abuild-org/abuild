.PRECIOUS: interface.fl

# Generate .fl from .qfl file
interface.fl: interface.qfl $(SRCDIR)/gen_flex interface.yy
	@$(PRINT) Generating $@ from $<
	perl $(SRCDIR)/gen_flex $< $@

clean-parser-cache:
	$(RM) $(SRCDIR)/parser-cache/interface.*
