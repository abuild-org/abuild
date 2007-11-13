all:: auto.h

auto.h:
	@$(PRINT) Generating $@
	echo '#define AUTO_VALUE 818' > $@
