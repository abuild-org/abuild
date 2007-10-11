# Targets to support the qtest automated test framework

ifeq ($(wildcard $(SRCDIR)/qtest),$(SRCDIR)/qtest)

check test:: all
	qtest-driver -datadir $(SRCDIR)/qtest -bindirs $(SRCDIR):. -covdir $(SRCDIR)

clean::
	$(RM) test.log *.cov_out

else

check test:: all ;

endif
