#
# This file is loaded before any rules in this target type.  It should
# never be loaded manually by the user.
#

include $(call load_plugin,toolchains/$(CCXX_TOOLCHAIN))
