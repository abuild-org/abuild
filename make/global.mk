# Global setup required for all abuild invocations

RM := rm -f

# Note: do not add .SECONDARY: with no targets.  Doing so causes make
# to treat all targets as secondary.  One of the side effects is that
# make will no longer consider a non-existent dependency as a reason
# to rebuild something, which defeats the way we do automatic
# dependencies.

# We are quiet by default, but use of ABUILD_VERBOSE and ABUILD_SILENT
# can control this to some extent.  Export these if defined so that
# helper scripts can also use them.

ifdef ABUILD_VERBOSE
export ABUILD_VERBOSE
else
.SILENT:
endif

# Use @$(PRINT) for diagnostics.  That way we can make abuild even
# more silent than it is by default.
ifdef ABUILD_SILENT
export ABUILD_SILENT
PRINT := @:
else
PRINT := echo
endif

# dep suffix
DEP := dep

# Cancel automatic remaking of included makefile fragments.  Note that
# gnu make makes extra attempts to rebuild included makefiles (which
# are always .mk files for us) even if they exist when included.  See
# the gnu make documentation for details.
%.mk: ;
%.$(DEP): ;

# Define a FORCE target for cases in which this techinque is required
# rather than .PHONY to force something to build.  It is also used by
# gen_deps.

FORCE: ;

# Support for codegen-wrapper
CODEGEN_WRAPPER = perl $(ABUILD_TOP)/util/codegen-wrapper --src $(SRCDIR)

# Define some useful utility functions and common variables

ABUILD_SUPPORT_1_0 ?= 0

# It's hard to get a space in make for substitution, so we define one
_space := $(subst a,,a a)
_comma := ,

GEN_DEPS := $(abBIN)/gen_deps

# Usage: $(call starts_with s1,s2) -- returns 1 if s2 starts with s1
# and the empty string otherwise
define starts_with
$(if $(filter $(1)%,$(2)),1)
endef

# Usage: $(call starts_with_any s1,s2) -- returns a non-empty string
# if s2 starts with any of the strings in s1 and the empty string
# otherwise
define starts_with_any
$(subst $(_space),,$(foreach S,$(1),$(call starts_with,$(S),$(2))))
endef

# usage: $(call strip_srcdir,val) -- strip $(SRCDIR) from the
# beginning of val
define strip_srcdir
$(subst $(SRCDIR)/,,$(1))
endef

# usage: $(call x_to_y,x,y,VAR) -- return %.y for each %.x in $(VAR)
define x_to_y
$(patsubst %.$(1),%.$(2),$(filter %.$(1),$($(3))))
endef

# Usage: $(call undefined_vars,VAR VAR...) -- return a list of
# undefined variables from passed in arguments.
define undefined_vars
$(subst =undefined,,$(filter %=undefined,$(foreach V,$(1),$(V)=$(origin $(V)))))
endef

# Usage: $(call value_if_defined,VAR,fallback-value) -- returns $(VAR)
# if VAR is defined or fallback-value otherwise.
define value_if_defined
$(if $(filter undefined,$(origin $(1))),$(2),$($(1)))
endef

# Usage: $(call general_to_specific,abc.def.ghi)
# returns abc abc.def abc.def.ghi
define general_to_specific
$(if $(findstring .,$1),$(call general_to_specific,$(basename $1)) $1,$1)
endef

# Usage: $(call undefined_items,source_variable)
# Returns a list of undefined items in in $(source_variable).
# source_variable is the NAME of the user-provided variable that is
# being checked.
define undefined_items
$(subst abDIR_,,$(call undefined_vars,$(foreach V,$($(1)),abDIR_$(V))))
endef

# Usage: $(call load_toolchain,toolchain)
# Searches each toolchain path, returning the first match.
define load_toolchain
$(firstword $(wildcard $(foreach D,$(ABUILD_TOOLCHAIN_PATHS),\
                         $(D)/$(1).mk)) --not-found--/$(1).mk)
endef

# Usage: $(call load_rule,rule)
# Searches each rule path, returning the first match.
define load_rule
$(firstword $(wildcard $(foreach D,$(ABUILD_RULE_PATHS),\
                         $(D)/$(1).mk)) --not-found--/$(1).mk)
endef

# Usage: $(call deprecate,version,message)
define _deprecation_warning
 _DUMMY := $(warning *** DEPRECATION WARNING *** (abuild version $(1)): $(2))
endef
# Prints a deprecation warning and also an error if in error mode.  It
# doesn't seem to be possible to nest an ifdef inside a define, so we
# have to duplicate the deprecation warning statement...
ifdef ABUILD_DEPRECATE_IS_ERROR
 define deprecate
  $(_deprecation_warning)
  _DUMMY := $(error deprecation error mode; failing)
 endef
else
 define deprecate
  $(_deprecation_warning)
 endef
endif
