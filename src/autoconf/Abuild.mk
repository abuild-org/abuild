AUTOFILES := autoconf.interface
CONFIGURE_ARGS :=
ifeq ($(ABUILD_PLATFORM_COMPILER),msvc)
  CONFIGURE_ARGS += --with-msvc
endif
ifdef BOOST_TOP
  CONFIGURE_ARGS += --with-boost=$(BOOST_TOP)
endif
RULES := autoconf
