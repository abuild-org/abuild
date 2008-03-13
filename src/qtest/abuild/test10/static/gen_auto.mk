auto.cc: string-value gen_auto.mk
	$(RM) $@
	@$(PRINT) Generating $@ with value `cat $<`
	echo '#include "Static.hh"' > $@
	echo 'char const* const Static::str = "'`cat $(SRCDIR)/string-value`'";' >> $@
