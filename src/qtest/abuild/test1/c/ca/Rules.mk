all:: moo.quack ;

moo.quack:
	@$(PRINT) Creating moo.quack
	echo oink > moo.quack

clean::
	$(RM) moo.quack
