_UNDEFINED := $(call undefined_vars,\
		INPUT)
ifneq ($(words $(_UNDEFINED)),0)
$(error The following variables are undefined: $(_UNDEFINED))
endif

all:: $(foreach I,$(INPUT),$(I).rpt)

define rpt_command
	perl $(abDIR_repeater)/repeater.pl -i $< -o $@
endef

$(INPUT:%=%.rpt): %.rpt: %
	@$(PRINT) Generating $@ from $< with repeater
ifdef REPEATER_CACHE
	$(CODEGEN_WRAPPER) --cache $(REPEATER_CACHE) \
	    --input $< --output $@ --command $(rpt_command)
else
	$(rpt_command)
endif
