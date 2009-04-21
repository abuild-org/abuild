#ifndef __INTERFACEPARSER_HH__
#define __INTERFACEPARSER_HH__

#include <Parser.hh>

#include <string>
#include <vector>
#include <deque>
#include <set>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/regex.hpp>

#include <nt_all.hh>
#include "interface.tab.hh"

class InterfaceParser;
class FlexCaller;

extern "C"
{
    void interfaceerror(InterfaceParser*, char *);
    int interfacelex(YYSTYPE*, InterfaceParser*);
    int interfaceparse(InterfaceParser*);
}
extern FlexCaller& interfaceGetFlexCaller();

// A single InterfaceParser object may be used to parse multiple
// interface files.  When used in this way, it is as if the contents
// of the files are concatenated into a single interface file, except
// that the directory from which relative paths are resolved can
// differ for each file.  The intended mode of operation is that a
// single InterfaceParser object will be used to parse a single build
// item's Abuild.interface and any after-build files it loads, but
// different InterfaceParser objects will be used to create Interface
// objects for different build items.  Certain documented behaviors of
// abuild, such as the semantics around resetting variables and making
// things conditional based on flags, depend upon this pattern of use.

class InterfaceParser: public Parser
{
  public:
    // The item_name and item_platform parameters are passed through
    // to the Interface created by this parser.
    InterfaceParser(std::string const& item_name,
		    std::string const& item_platform,
		    std::string const& local_dir);
    virtual ~InterfaceParser();

    // Parse a file.  Returns true if there were no errors.
    bool parse(std::string const& filename, bool allow_after_build);

    // Tell the InterfaceParser what flags is is allowed to accept
    void setSupportedFlags(std::set<std::string> const& supported_flags);

    void storeData(YYSTYPE* data);
    virtual void setToken(Token* t);
    void acceptParseTree(nt_Blocks*);

    // Imports the given interface into our internal interface
    bool importInterface(Interface const&);

    // Return the internal interface object
    boost::shared_ptr<Interface> getInterface() const;

    // Retrieve a copy of the list of after-build files from the most
    // recent call to parse().  The after-build files list is reset
    // with each call.
    std::vector<std::string> getAfterBuilds() const;

    // These methods create non-terminals and save them for automatic
    // deletion.
    nt_Word* createWord(FileLocation const&);
    nt_Words* createWords(FileLocation const&);
    nt_AfterBuild* createAfterBuild(nt_Words*);
    nt_TargetType* createTargetType(Token*);
    nt_TypeSpec* createTypeSpec(FileLocation const&, Interface::type_e);
    nt_Declaration* createDeclaration(Token* identifier, nt_TypeSpec* typsepec);
    nt_Function* createFunction(Token* identifier, nt_Arguments* arguments);
    nt_Argument* createArgument(nt_Function*);
    nt_Argument* createArgument(nt_Words*);
    nt_Arguments* createArguments(FileLocation const&);
    nt_Conditional* createConditional(Token* variable);
    nt_Conditional* createConditional(nt_Function*);
    nt_Assignment* createAssignment(Token* identifier, nt_Words*);
    nt_Reset* createReset(Token* identifier, bool negate);
    nt_Reset* createReset(FileLocation const&);
    nt_Block* createBlock(nt_IfBlock*);
    nt_Block* createBlock(nt_Reset*);
    nt_Block* createBlock(nt_Assignment*);
    nt_Block* createBlock(nt_Declaration*);
    nt_Block* createBlock(nt_AfterBuild*);
    nt_Block* createBlock(nt_TargetType*);
    nt_Blocks* createBlocks();
    nt_IfClause* createIfClause(nt_Conditional*, nt_Blocks*, bool);
    nt_IfClauses* createIfClauses(FileLocation const&);
    nt_IfBlock* createIfBlock(nt_IfClause*, nt_IfClauses*, nt_IfClause*);

  protected:
    virtual void parseFile();
    virtual void startFile(std::string const&);
    virtual void endFile(std::string const&);

  private:
    // Parse Tree evaluation functions
    void evaluateParseTree();
    void evaluateBlocks(nt_Blocks const*, bool evaluating);
    void evaluateBlock(nt_Block const*, bool evaluating);
    void evaluateIfBlock(nt_IfBlock const*, bool evaluating);
    void evaluateReset(nt_Reset const*, bool evaluating);
    void evaluateAssignment(nt_Assignment const*, bool evaluating);
    void evaluateDeclaration(nt_Declaration const*, bool evaluating);
    void evaluateAfterBuild(nt_AfterBuild const*, bool evaluating);
    void evaluateTargetType(nt_TargetType const*, bool evaluating);
    std::deque<std::string> evaluateWords(nt_Words const*);
    void evaluateWord(nt_Word const*, std::deque<std::string>& result);
    std::string evaluateToken(Token const*);
    std::string evaluateEnvironment(Token const*);
    bool evaluateConditional(nt_Conditional const*, bool evaluating,
			     bool& istrue);
    bool evaluateBooleanOrFunction(
	Token const* variable, nt_Function const* function,
	bool evaluating, bool& truth_value);
    bool evaluateBooleanVariable(Token const* token, bool evaluating,
				 bool& variable_true);
    bool evaluateFunction(nt_Function const* function, bool evaluating,
			  bool& function_true);
    std::string getVariableName(Token const*);
    std::string getFunctionName(Token const*);
    std::string getFirstMatch(Token const*, boost::regex&);
    bool evaluateFunctionAnd(
	FileLocation const&, std::vector<nt_Argument const*> const&,
	bool& function_true);
    bool evaluateFunctionOr(
	FileLocation const&, std::vector<nt_Argument const*> const&,
	bool& function_true);
    bool evaluateFunctionNot(
	FileLocation const&, std::vector<nt_Argument const*> const&,
	bool& function_true);
    bool evaluateFunctionEquals(
	FileLocation const&, std::vector<nt_Argument const*> const&,
	bool& function_true);
    bool evaluateFunctionMatches(
	FileLocation const&, std::vector<nt_Argument const*> const&,
	bool& function_true);
    bool evaluateFunctionContains(
	FileLocation const&, std::vector<nt_Argument const*> const&,
	bool& function_true);
    bool evaluateFunctionContainsmatch(
	FileLocation const&, std::vector<nt_Argument const*> const&,
	bool& function_true);
    bool containsMatch(FileLocation const&,
		       std::deque<std::string> const& words,
		       std::string const& regex,
		       bool& truth_value);

    bool checkBooleanArgument(nt_Argument const*, bool& truth_value);
    bool checkStringArgument(nt_Argument const*, std::string& value);
    bool checkFilenameArgument(nt_Argument const*, std::string& value);
    bool checkStringOrFilenameArgument(
	nt_Argument const*, bool string, std::string& value);
    bool checkWordsArgument(nt_Argument const*, nt_Words const*& value);
    bool checkNestedFunctions(std::vector<nt_Argument const*> const&);
    bool checkNumArguments(
	FileLocation const& location, std::string const& function_name,
	std::vector<nt_Argument const*> arguments,
	unsigned int desired_num_arguments);

    // Perform the requested action with the given variable and return
    // true if the variable is declared and initialized.  Otherwise,
    // issue the appropriate error message and return false.
    bool withVariable(Token const*,
		      boost::function<void (std::string const&,
					    Interface::VariableInfo const&)>);
    // Functions to be used with withVariable
    void appendVariableValue(std::deque<std::string>& results,
			     std::string const&,
			     Interface::VariableInfo const& info);
    void checkBooleanVariable(FileLocation const&,
			      bool& valid,
			      bool& variable_true,
			      std::string const&,
			      Interface::VariableInfo const& info);
    void checkStringOrFilenameVariable(FileLocation const&, bool string,
				       bool& valid,
				       std::string& string_value,
				       std::string const&,
				       Interface::VariableInfo const& info);

    static std::map<std::string, int> function_num_arguments;
    static std::map<std::string,
		    bool (InterfaceParser::*)(
			FileLocation const&,
			std::vector<nt_Argument const*> const&,
			bool& /* function_true */)> function_evaluators;

    YYSTYPE* yydata;
    nt_Blocks* parse_tree;
    bool allow_after_build;
    // <windows.h> #define's interface to struct sometimes.
    boost::shared_ptr<Interface> _interface;
    std::set<std::string> supported_flags;
    std::vector<std::string> after_builds;
    std::set<std::string> protected_from_reset;
};

#endif // __INTERFACEPARSER_HH__
