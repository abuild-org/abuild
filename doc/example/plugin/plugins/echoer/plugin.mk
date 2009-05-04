all:: echo ;

echo::
	@$(PRINT) This is a message from the echoer plugin.
	@$(PRINT) The value of ECHO_MESSAGE is $(ECHO_MESSAGE)
