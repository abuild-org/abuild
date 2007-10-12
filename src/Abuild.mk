ifdef BOOST_INCLUDES
INCLUDES += $(BOOST_INCLUDES)
endif
ifdef BOOST_LIBDIRS
LIBDIRS += $(BOOST_LIBDIRS)
endif
BOOST_LIB_SUFFIX ?=
# MSVC doesn't require explicitly naming of boost libraries
ifneq ($(CCXX_TOOLCHAIN), msvc)
LIBS += boost_thread$(BOOST_LIB_SUFFIX) boost_regex$(BOOST_LIB_SUFFIX)
endif
ifneq ($(ABUILD_PLATFORM_OS),windows)
LIBS += pthread
endif

ifdef ABUILD_STATIC
# Support static linking for the bootstrap build
XLINKFLAGS += -static
endif

ifeq ($(CCXX_TOOLCHAIN), gcc)
WFLAGS += -Werror
endif

ifeq ($(CCXX_TOOLCHAIN), msvc)
# Disable a warning in InterfaceParser about using "this" in a constructor
WFLAGS_InterfaceParser.cc = -wd4355
endif

WFLAGS_interface.tab.cc :=
WFLAGS_FlexLexer.interface.cc :=

TEST_PROGS := \
	test_util \
	test_keyval \
	test_canonicalize_path \
	test_get_program_fullpath \
	test_run_program \
	test_logger \
	test_thread_safe_queue \
	test_worker_pool \
	test_dependency_graph \
	test_dependency_evaluator \
	test_dependency_runner \
	test_interface \
	test_interface_parser

TARGETS_bin := abuild $(TEST_PROGS)
TARGETS_lib := abuild

$(foreach P,$(TEST_PROGS),$(eval SRCS_bin_$(P) := $(P).cc))

# Sources that are not specific to abuild
GENERAL_SRCS := \
	QTC.cc \
	QEXC.cc \
	Util.cc \
	Logger.cc \
	Error.cc \
	KeyVal.cc \
	FileLocation.cc \
	DependencyEvaluator.cc \
	DependencyGraph.cc \
	DependencyRunner.cc

# Automatically generated scanners and parsers that we may sometimes save
AUTO_SRCS := \
	interface.tab.cc \
	FlexLexer.interface.cc

# Files to be saved by save_auto in local-rules.mk
SAVE_AUTO := $(AUTO_SRCS) interface.tab.hh /usr/include/FlexLexer.h

# All sources including abuild-specific sources

# The "abuild library" isn't released -- it's just a convenient way of
# getting all the object files together for use by the various test
# programs.  We put files that are not used by any test suites (other
# than abuild's own) in with the executable instead of the library
# simply to avoid having to relink all the test programs whenever
# they change.
SRCS_lib_abuild := \
	$(GENERAL_SRCS) \
	$(AUTO_SRCS) \
	InterfaceLexer.cc \
	InterfaceParser.cc \
	FlagData.cc \
	Parser.cc \
	Token.cc \
	TokenFactory.cc \
	NonTerminal.cc \
	nt_Word.cc \
	nt_Words.cc \
	nt_AfterBuild.cc \
	nt_TargetType.cc \
	nt_TypeSpec.cc \
	nt_Declaration.cc \
	nt_Function.cc \
	nt_Argument.cc \
	nt_Arguments.cc \
	nt_Conditional.cc \
	nt_Assignment.cc \
	nt_Reset.cc \
	nt_Blocks.cc \
	nt_Block.cc \
	nt_IfClause.cc \
	nt_IfClauses.cc \
	nt_IfBlock.cc \
	Interface.cc \
	TargetType.cc

SRCS_bin_abuild := \
	PlatformData.cc \
	TraitData.cc \
	PlatformSelector.cc \
	ItemConfig.cc \
	BuildTree.cc \
	BuildItem.cc \
	Abuild.cc \
	abuild_main.cc

# If not defined or empty, coverage cases will not be invoked in the
# make code.
export QTC_MK_DIR := $(shell qtest-driver --print-path)/QTC/make

export TC_SRCS := \
	$(wildcard $(SRCDIR)/*.cc) \
	$(wildcard $(SRCDIR)/../make/*.mk) \
	$(wildcard $(SRCDIR)/../make/*/*.mk) \
	$(wildcard $(SRCDIR)/../make/*/*/*.mk)

RULES := ccxx
LOCAL_RULES := local-rules.mk
