MAIN_DOC := manual

# Generate example-list as a side effect of initializing DUMMY.  This
# forces it to be run serially.  This script only updates example-list
# if its contents have changed from the last run.  The example-list
# file contains the names and modification times of all files in
# doc/example and all example output files in the test suite.
DUMMY := $(shell perl $(SRCDIR)/gen-example-list.pl)

FIGURE_SRCS := $(wildcard $(SRCDIR)/figures/*.dia)
FIGURES := $(foreach F,$(FIGURE_SRCS),$(patsubst %.dia,%.png,$(notdir $(F))))
EXTRA_DEPS := example-list $(SRCDIR)/../abuild_data.dtd $(FIGURES)
EXTRA_FILES := stylesheet.css
LOCAL_RULES := docbook.mk figures.mk extrafiles.mk
