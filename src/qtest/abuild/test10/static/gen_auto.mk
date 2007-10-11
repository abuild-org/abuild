auto.cc: string-value gen_auto.mk
	$(RM) $@
	@$(PRINT) Generating $@ with value `cat $<`
	echo '#include "Static.hh"' > $@
	echo -n 'char const* const Static::str = "' >> $@
	echo -n `cat $(SRCDIR)/string-value` >> $@
	echo '";' >> $@
