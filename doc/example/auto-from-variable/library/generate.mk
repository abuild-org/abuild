# Write the value to a temporary file and replace the real file if the
# value has changed or the real file doesn't exist.
DUMMY := $(shell echo > variable-value.tmp $(file-provider-filename))
DUMMY := $(shell diff >/dev/null 2>&1 variable-value.tmp variable-value || \
	   mv variable-value.tmp variable-value)

# Write the header file based on the variable value.  We can just use
# the variable directly here instead of catting the "variable-value"
# file since we know that the contents of the file always match the
# variable name.

abs_filename := $(abspath $(file-provider-filename))
# If this is cygwin supporting Windows, we need to convert this into a
# Windows path.  Convert \ to / as well to avoid quoting issues.
ifeq ($(ABUILD_PLATFORM_TOOLSET),nt5-cygwin)
 abs_filename := $(subst \,/,$(shell cygpath -w $(abs_filename)))
endif

FileProvider_file.hh: variable-value
	echo '#ifndef __FILEPROVIDER_FILE_HH__' > $@
	echo '#define __FILEPROVIDER_FILE_HH__' >> $@
	echo '#define FILE_LOCATION "$(abs_filename)"' >> $@
	echo '#endif' >> $@

# Make sure our automatically generated file gets generated before we
# compile FileProvider.cc.  Unfortunately, the only way to do this
# that will work reliably in a parallel build is to create an explicit
# dependency.  We use the LOBJ variable to get the object file suffix
# because FileProvider.cc is part of a library.  One way to avoid this
# issue entirely would be to automatically generate a source file
# instead of a header file, but as it is often more convenient to
# generate a header file, we illustrate how to do so in this example.
FileProvider.$(LOBJ): FileProvider_file.hh
