all:: $(REQ_XML).html

%.html: %.xml requirements.dtd requirements.xsl
	@$(PRINT) Validating $<
	xmllint --valid --noout $<
	perl $(SRCDIR)/check-ids.pl $<
	@$(PRINT) Generating $@ from $<
	xsltproc $(SRCDIR)/requirements.xsl $< > $@

clean::
	$(RM) *.html
