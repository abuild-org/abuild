WOBJS := $(call x_to_y,w,x,SRCS)

all:: $(WOBJS)

$(WOBJS): %.x: %.w
	@$(PRINT) Pretending to compile $< as W
	mkdir -p $(dir $@)
	$(RM) $@
	cp -f $< $@
	echo 'w -> x' >> $@

clean::
	$(RM) $(WOBJS)
