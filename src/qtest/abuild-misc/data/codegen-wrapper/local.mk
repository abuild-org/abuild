COPY=cp

in2: in2.in
	@$(PRINT) generating $@ from $^
	$(CODEGEN_WRAPPER) --cache cache \
		--input $^ --output $@ \
		--command $(COPY) $< $@

out1: a/in1 in2
	@$(PRINT) generating $@ from $^
	$(CODEGEN_WRAPPER) --cache cache \
		--input $^ --output out1 b/out1 \
		--command perl ../codegen $^ $@

out2:
	@$(PRINT) generating $@
	$(CODEGEN_WRAPPER) --cache cache \
		--output $@ \
		--command sh -c "echo test > $@"

all:: out1 out2
