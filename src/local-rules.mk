.PRECIOUS: %.fl

# Generate .fl from .qfl file
%.fl: %.qfl $(SRCDIR)/gen_flex
	@$(PRINT) Generating $@ from $<
	perl $(SRCDIR)/gen_flex $< $@

clean-parser-cache:
	$(RM) $(SRCDIR)/parser-cache/interface.*
