all::
	@echo -n $(ABUILD_ITEM_NAME) 'out1 '
	@echo -n 1>&2 $(ABUILD_ITEM_NAME) 'err1 '
	@sleep 1
	@echo 1>&2 $(ABUILD_ITEM_NAME) err2
	@sleep 1
	@echo $(ABUILD_ITEM_NAME) out2
