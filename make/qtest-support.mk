# Targets to support the qtest automated test framework

TC_SRCS ?=
export TC_SRCS

ifeq ($(wildcard $(SRCDIR)/qtest),$(SRCDIR)/qtest)

define run_qtest
	qtest-driver -datadir $(SRCDIR)/qtest -bindirs $(SRCDIR):. -covdir $(SRCDIR)
endef

check test:: all
	$(run_qtest)

test-only::
	$(run_qtest)

else

check test:: all ;
test-only:: ;

endif
