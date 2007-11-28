
# Dia's conversion to png isn't very good.  It creates a 20 pixels/cm
# image with poor antialiasing.  We get better results converting to
# eps and then using ImageMagick to go from there to png.  We might
# want to generate different images for HTML and PDF (fo), but for
# now, we'll just leave it at this.  The > in the resize parameter
# means to scale down to this size of the image dimensions are greater
# than this but not to scale up if they are smaller.

.PRECIOUS: %.png
%.png: ../figures/%.dia
	dia -e $*.eps $<
	@$(PRINT) Generating $@ from $*.eps
	convert $*.eps -antialias -resize '432x432>' $@
