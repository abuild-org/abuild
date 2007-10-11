
.PRECIOUS: %.fl

# Generate .fl from .qfl file

%.fl: %.qfl $(SRCDIR)/gen_flex
	@$(PRINT) Generating $@ from $<
	perl $(SRCDIR)/gen_flex $< $@

# Save automatically generated C/C++ code

save-auto: $(SAVE_AUTO)
	@$(RM) -r ../auto-srcs
	mkdir ../auto-srcs
	@$(PRINT) Saving automatically generated source files
	cp -p $(SAVE_AUTO) ../auto-srcs
