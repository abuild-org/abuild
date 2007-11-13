ifeq ($(ABUILD_TARGET_TYPE), object-code)
 ifeq ($(SAW_AUTO_PROVIDER), 0)
 $(error This item is supposed to depend on auto-provider, but it does not)
 endif
endif
