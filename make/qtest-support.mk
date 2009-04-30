# Targets to support the qtest automated test framework

ifeq ($(wildcard $(SRCDIR)/qtest),$(SRCDIR)/qtest)

TC_SRCS ?=
export TC_SRCS

check test test-only:: all
	qtest-driver -datadir $(SRCDIR)/qtest -bindirs $(SRCDIR):. -covdir $(SRCDIR)

endif
