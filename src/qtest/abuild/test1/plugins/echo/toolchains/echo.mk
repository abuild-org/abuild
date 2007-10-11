.LIBPATTERNS = shlib-% lib-%
OBJ = o
LOBJ = lo

PREPROCESS_c = echo PREPROCESS_c
PREPROCESS_cxx = echo PREPROCESS_cxx
COMPILE_c = echo COMPILE_c
COMPILE_cxx = echo COMPILE_cxx
LINK_c = echo LINK_c
LINK_cxx = echo LINK_cxx

DFLAGS = -dflags-
OFLAGS = -oflags-
WFLAGS = -wflags-

ifeq ($(ABUILD_PLATFORM_OPTION), debug)
OFLAGS =
endif
ifeq ($(ABUILD_PLATFORM_OPTION), release)
DFLAGS =
endif

CCXX_GEN_DEPS = echo CCXX_GEN_DEPS

define libname
lib-$(1)
endef

define binname
bin-$(1)
endef

define shlibname
shlib-$(1)$(if $(2),.$(2)$(if $(3),.$(3)$(if $(4),.$(4))))
endef

define include_flags
$(foreach I,$(1),-include- $(I))
endef

define make_obj
	@$(PRINT) make-obj compiler: $(1)
	@$(PRINT) make-obj pic: $(2)
	@$(PRINT) make-obj flags: $(3)
	@$(PRINT) make-obj src: $(4)
	@$(PRINT) make-obj obj: $(5)
	cp $(4) $(5)
	@echo contents of $(5):
	cat $(5)
	@echo end contents of $(5)
endef

define make_lib
	@$(PRINT) make-lib objects: $(1)
	@$(PRINT) make-lib lib: $(call libname,$(2))
	echo '** lib $(2) **' > $(call libname,$(2))
	cat $(1) >> $(call libname,$(2))
	echo '** end of lib $(2) **' >> $(call libname,$(2))
	@echo contents of $(call libname,$(2)):
	cat $(call libname,$(2))
	@echo end contents of $(call libname,$(2))
endef

define make_bin
	@$(PRINT) make-bin: linker: $(1)
	@$(PRINT) make-bin: compiler-flags: $(2)
	@$(PRINT) make-bin: link-flags: $(3)
	@$(PRINT) make-bin: objects: $(4)
	@$(PRINT) make-bin: lib-dirs: $(5)
	@$(PRINT) make-bin: libs: $(6)
	@$(PRINT) make-bin: bin: $(call binname,$(7))
	echo '** bin $(7) **' > $(call binname,$(7))
	cat $(4) `$(abDIR_plugin.echo)/find_libs $(foreach libdir,$(5),-L$(libdir)) $(foreach lib,$(6),-l$(lib))` >> $(call binname,$(7))
	echo '** end of bin $(7) **' >> $(call binname,$(7))
	@echo contents of $(call binname,$(7)):
	cat $(call binname,$(7))
	@echo end contents of $(call binname,$(7))
endef

define make_shlib
	@$(PRINT) make-shlib: linker: $(1)
	@$(PRINT) make-shlib: compiler-flags: $(2)
	@$(PRINT) make-shlib: link-flags: $(3)
	@$(PRINT) make-shlib: objects: $(4)
	@$(PRINT) make-shlib: lib-dirs: $(5)
	@$(PRINT) make-shlib: libs: $(6)
	@$(PRINT) make-shlib: shlib: $(call shlibname,$(7),$(8),$(9),$(10))
	$(RM) $(call shlibname,$(7)) $(call shlibname,$(7)).*
	echo '** shlib $(7) **' > $(call shlibname,$(7),$(8),$(9),$(10))
	cat $(4) `$(abDIR_plugin.echo)/find_libs $(foreach libdir,$(5),-L$(libdir)) $(foreach lib,$(6),-l$(lib))` >> $(call shlibname,$(7),$(8),$(9),$(10))
	echo '** end of shlib $(7) **' >> $(call shlibname,$(7),$(8),$(9),$(10))
	@echo contents of $(call shlibname,$(7),$(8),$(9),$(10)):
	cat $(call shlibname,$(7),$(8),$(9),$(10))
	@echo end contents of $(call shlibname,$(7),$(8),$(9),$(10))
	if [ x"$(8)" != x ]; then \
	    cp $(call shlibname,$(7),$(8),$(9),$(10)) $(call shlibname,$(7)); \
	fi
	if [ x"$(9)" != x ]; then \
	    cp $(call shlibname,$(7),$(8),$(9),$(10)) $(call shlibname,$(7),$(8)); \
	fi
endef
