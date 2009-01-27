# Ensure that we can use -llib dependencies properly
.LIBPATTERNS := lib%.so lib%.a
$(foreach PAT,$(.LIBPATTERNS),$(eval vpath $(PAT) $(LIBDIRS)))

all:: $(TARGET)

QOBJS := $(call x_to_y,q,r,SRCS)
$(QOBJS): %.r: %.q
	@$(PRINT) Pretending to compile $< as Q
	mkdir -p $(dir $@)
	$(RM) $@
	cp -f $< $@
	echo 'q -> r' >> $@

$(TARGET): $(QOBJS)
	@$(PRINT) Prentending to link $@
	$(RM) $(TARGET)
	cat $(QOBJS) > $@
	echo 'link' >> $@

clean::
	$(RM) $(TARGET) $(QOBJS)
