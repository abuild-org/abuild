MAIN_DOC := manual
MAIN_DOC_OUTPUT_PREFIX := abuild-

FIGURE_SRCS := $(wildcard $(SRCDIR)/figures/*.dia)
FIGURES := $(foreach F,$(FIGURE_SRCS),$(patsubst %.dia,%.png,$(notdir $(F))))
EXTRA_DEPS := $(FIGURES) stylesheet.css
LOCAL_RULES := docbook.mk figures.mk extrafiles.mk
