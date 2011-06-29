# Targets to support the qtest automated test framework

ifeq ($(wildcard $(SRCDIR)/qtest),$(SRCDIR)/qtest)

TC_SRCS ?=
export TC_SRCS

define run_qtest
	qtest-driver -datadir $(SRCDIR)/qtest -bindirs $(SRCDIR):. -covdir $(SRCDIR)
endef

# Note: although abuild.mk declares check and test to depend on all
# and test-only not to depend on all, we still have to set up our own
# check, test, and test-only targets in this way so that these
# instances of the targets will exhibit the correct behavior.

check test:: all
	$(run_qtest)

test-only::
	$(run_qtest)

endif
