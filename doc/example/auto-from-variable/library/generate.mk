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
# compile
generate:: FileProvider_file.hh ;
