ifdef FOP_HOME
FOP = $(FOP_HOME)/fop
else
FOP = fop
endif

_UNDEFINED := $(call undefined_vars, DOCBOOK_XSL DOCBOOK_DTD)
ifneq ($(words $(_UNDEFINED)),0)
$(error The following variables are undefined: $(_UNDEFINED); see README)
endif

all:: $(MAIN_DOC).html $(MAIN_DOC).pdf $(EXTRA_FILES)

validate: $(MAIN_DOC).xml
	@$(PRINT) Validating $<
	xmllint --noout --dtdvalid $(DOCBOOK_DTD) ../$(MAIN_DOC).xml
	touch validate

%.pdf: %.fo
	@$(PRINT) Generating $@ from $<
	$(FOP) $< -pdf $@

%.html: %-processed.xml html.xsl
	@$(PRINT) Generating $@ from $<
	xsltproc --output $@ html.xsl $<

.PRECIOUS: %.fo
%.fo: %-processed.xml print.xsl
	@$(PRINT) Generating $@ from $<
	xsltproc --output $@ print.xsl $<

.PRECIOUS: %.xsl
%.xsl: %.xsl.in
	sed -e 's%--STYLESHEETS--%$(DOCBOOK_XSL)%' < $< > $@

.PRECIOUS: %-processed.xml
%-processed.xml: %.xml $(SRCDIR)/process-manual.pl $(EXTRA_DEPS) validate
	@$(PRINT) Processing $<
	perl $(SRCDIR)/process-manual.pl $< $@
