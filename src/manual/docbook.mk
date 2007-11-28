ifdef FOP_HOME
FOP = $(FOP_HOME)/fop
else
FOP = fop
endif

_UNDEFINED := $(call undefined_vars, DOCBOOK_XSL DOCBOOK_DTD)
ifneq ($(words $(_UNDEFINED)),0)
$(error The following variables are undefined: $(_UNDEFINED); see README)
endif

all:: html pdf

.PHONY: html
html: html.stamp

html.stamp: $(MAIN_DOC)-processed.xml html.xsl chunk.xsl
	@$(PRINT) Generating HTML documents from $<
	$(RM) -r html
	mkdir html
	cp -p *.png *.css html
	(cd html; xsltproc --output $(MAIN_DOC_OUTPUT_PREFIX)$(MAIN_DOC).html \
		../html.xsl ../$<)
	(cd html; xsltproc ../chunk.xsl ../$<)
	touch html.stamp

pdf: $(MAIN_DOC_OUTPUT_PREFIX)$(MAIN_DOC).pdf

validate: $(MAIN_DOC).xml
	@$(PRINT) Validating $<
	xmllint --noout --dtdvalid $(DOCBOOK_DTD) ../$(MAIN_DOC).xml
	touch validate

$(MAIN_DOC_OUTPUT_PREFIX)%.pdf: %.fo
	@$(PRINT) Generating $@ from $<
	$(FOP) $< -pdf $@

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
