# Assign a default target before doing anything else so that we don't
# end up with the default being whatever target happens to be defined
# first.  Abuild never invokes make without explicitly providing a
# target.

.PHONY: default
default: ;
	@$(PRINT) abuild invoked make with no target or default target
	false

# If QTC_MK_DIR is set, we're running abuild's test suite, so we
# should include makefile coverage support.  Otherwise, set QTC.TC to
# empty so that calls to it will expand to nothing and have negligible
# impact on build times.
ifdef QTC_MK_DIR
  include $(QTC_MK_DIR)/QTC.mk
else
  QTC.TC=
endif

# Set convenience variables for accessing other abuild files
abMK := $(ABUILD_TOP)/make
abBIN := $(ABUILD_TOP)/private/bin

# Global setup, utility functions
include $(abMK)/global.mk

# Include abuild's dynamically computed information.  This is
# generated by abuild before it invokes gmake.
include .ab-dynamic.mk

# Provide a "clean" target that gets run from inside an output
# directory.
clean::
	@: $(call QTC.TC,abuild,abuild.mk platform-specific clean,0)
	$(RM) .ab-dynamic.mk

# Determine the compiler toolchain, if applicable
ifeq ($(ABUILD_TARGET_TYPE), object-code)
CCXX_TOOLCHAIN = $(ABUILD_PLATFORM_COMPILER)
endif

# Include the current directory's build rules, remembering that the
# source directory is actually this directory's parent.
VPATH := ..
SRCDIR := ..
include $(SRCDIR)/Abuild.mk

RULES ?=
LOCAL_RULES ?=
BUILD_ITEM_RULES ?=

ifdef BUILD_ITEM_RULES
  $(call deprecate,1.1,BUILD_ITEM_RULES is deprecated; use named rules from the rules directory instead)
  _UNDEFINED := $(call undefined_items,BUILD_ITEM_RULES)
  ifneq ($(words $(_UNDEFINED)),0)
    $(error These build items from BUILD_ITEM_RULES in are unknown: $(_UNDEFINED))
  endif
endif

ifeq ($(words $(RULES) $(BUILD_ITEM_RULES) $(LOCAL_RULES)), 0)
  $(error No rules defined.)
endif

ifeq ($(filter _base,$(RULES)), _base)
  $(error The _base rule may not be specified explicitly.)
endif

# Support for the automated test framework is loaded for all types
# of rules.
include $(abMK)/qtest-support.mk

# Documentation support -- users are expected to provide documentation
# functionality in a plugin.
doc:: all ;

# Load any plugin files
include $(wildcard $(foreach P,$(ABUILD_PLUGINS),$(P)/plugin.mk))

# Include base rule and user-specified built-in rules
include $(foreach RULE,_base $(RULES),$(call load_rule,$(RULE)))

# Include any build-item-specific rules
ifneq ($(words $(BUILD_ITEM_RULES)),0)
  _qtx_dummy := $(call QTC.TC,abuild,abuild.mk BUILD_ITEM_RULES,0)
  include $(foreach BI,$(BUILD_ITEM_RULES),$(abDIR_$(BI))/Rules.mk)
endif

# Finally, include any local rules
ifneq ($(words $(LOCAL_RULES)),0)
  _qtx_dummy := $(call QTC.TC,abuild,abuild.mk LOCAL_RULES,0)
  include $(foreach R,$(LOCAL_RULES),$(SRCDIR)/$(R))
endif
