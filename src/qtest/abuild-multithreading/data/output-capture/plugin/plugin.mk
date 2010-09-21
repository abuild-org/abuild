# Use printf here since Solaris lacks echo -n
all::
	@printf "$(ABUILD_ITEM_NAME) out1 "
	@printf 1>&2 "$(ABUILD_ITEM_NAME) err1 "
	@sleep 1
	@echo 1>&2 $(ABUILD_ITEM_NAME) err2
	@sleep 1
	@echo $(ABUILD_ITEM_NAME) out2
