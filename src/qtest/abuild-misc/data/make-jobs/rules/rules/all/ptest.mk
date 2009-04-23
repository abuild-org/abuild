all:: a b

a:
	$(RM) a
	sleep 1
	touch a

b:
	if [ -f a ]; then echo a exists; else echo a does not exist; fi
