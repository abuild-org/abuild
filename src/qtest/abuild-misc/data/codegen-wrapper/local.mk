out1: a/in1 in2
	$(CODEGEN_WRAPPER) --cache cache \
		--input a/in1 in2 \
		--output out1 b/out1 \
		--command perl ../codegen $^ $@

out2:
	$(CODEGEN_WRAPPER) --cache cache \
		--output out2 \
		--command sh -c "echo test > $@"

all:: out1 out2
