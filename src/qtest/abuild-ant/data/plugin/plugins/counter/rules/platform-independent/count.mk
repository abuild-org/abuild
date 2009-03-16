
all:: count

# Make sure the user has asked for things to count.
ifeq ($(words $(TO_COUNT)), 0)
$(error plugin.counter: TO_COUNT is empty)
endif

# Use echo `wc` to normalize whitespace
count:
	for i in $(TO_COUNT); do echo `wc -l $$i`; done
