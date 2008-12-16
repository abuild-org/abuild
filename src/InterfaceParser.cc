#include <InterfaceParser.hh>

#include <boost/regex.hpp>
#include <boost/bind.hpp>
#include <QEXC.hh>
#include <QTC.hh>
#include <Util.hh>

std::map<std::string, int> InterfaceParser::function_num_arguments;
std::map<std::string,
	 bool (InterfaceParser::*)(
	     FileLocation const&,
	     std::vector<nt_Argument const*> const&,
	     bool&)> InterfaceParser::function_evaluators;

extern int interfacedebug;

void interfaceerror(InterfaceParser*, char *)
{
    // nothing -- error token is captured
}

int interfacelex(YYSTYPE* data, InterfaceParser* p)
{
    p->storeData(data);
    return p->getNextToken();
}

InterfaceParser::~InterfaceParser()
{
}

InterfaceParser::InterfaceParser(std::string const& item_name,
				 std::string const& instance_name,
				 std::string const& local_dir) :
    Parser(lexer, tok_EOF),
    lexer(this),
    parse_tree(0),
    allow_after_build(false)
{
    if (function_num_arguments.empty())
    {
	function_num_arguments["and"] = 2;
	function_num_arguments["or"] = 2;
	function_num_arguments["not"] = 1;
	function_num_arguments["equals"] = 2;
	function_num_arguments["matches"] = 2;
	function_num_arguments["contains"] = 2;
	function_num_arguments["containsmatch"] = 2;

	function_evaluators["and"] =
	    &InterfaceParser::evaluateFunctionAnd;
	function_evaluators["or"] =
	    &InterfaceParser::evaluateFunctionOr;
	function_evaluators["not"] =
	    &InterfaceParser::evaluateFunctionNot;
	function_evaluators["equals"] =
	    &InterfaceParser::evaluateFunctionEquals;
	function_evaluators["matches"] =
	    &InterfaceParser::evaluateFunctionMatches;
	function_evaluators["contains"] =
	    &InterfaceParser::evaluateFunctionContains;
	function_evaluators["containsmatch"] =
	    &InterfaceParser::evaluateFunctionContainsmatch;
    }

    this->_interface.reset(new Interface(item_name, instance_name,
					 error_handler, local_dir));
}

bool
InterfaceParser::parse(std::string const& filename,
		       bool allow_after_build)
{
    this->allow_after_build = allow_after_build;
    return this->Parser::parse(filename);
}

void
InterfaceParser::setSupportedFlags(std::set<std::string> const& supported_flags)
{
    this->supported_flags = supported_flags;
}

nt_Word*
InterfaceParser::createWord(FileLocation const& location)
{
    return saveNonTerminal(new nt_Word(location));
}

nt_Words*
InterfaceParser::createWords(FileLocation const& location)
{
    return saveNonTerminal(new nt_Words(location));
}

nt_AfterBuild*
InterfaceParser::createAfterBuild(nt_Words* w)
{
    return saveNonTerminal(new nt_AfterBuild(w));
}

nt_TargetType*
InterfaceParser::createTargetType(Token* identifier)
{
    return saveNonTerminal(new nt_TargetType(identifier));
}

nt_TypeSpec*
InterfaceParser::createTypeSpec(FileLocation const& location,
				Interface::type_e type)
{
    return saveNonTerminal(new nt_TypeSpec(location, type));
}

nt_Declaration*
InterfaceParser::createDeclaration(Token* identifier, nt_TypeSpec* typespec)
{
    return saveNonTerminal(new nt_Declaration(identifier, typespec));
}

nt_Function*
InterfaceParser::createFunction(Token* identifier, nt_Arguments* arguments)
{
    return saveNonTerminal(new nt_Function(identifier, arguments));
}

nt_Argument*
InterfaceParser::createArgument(nt_Function* function)
{
    return saveNonTerminal(new nt_Argument(function));
}

nt_Argument*
InterfaceParser::createArgument(nt_Words* words)
{
    return saveNonTerminal(new nt_Argument(words));
}

nt_Arguments*
InterfaceParser::createArguments(FileLocation const& location)
{
    return saveNonTerminal(new nt_Arguments(location));
}

nt_Conditional*
InterfaceParser::createConditional(Token* variable)
{
    return saveNonTerminal(new nt_Conditional(variable));
}

nt_Conditional*
InterfaceParser::createConditional(nt_Function* function)
{
    return saveNonTerminal(new nt_Conditional(function));
}

nt_Assignment*
InterfaceParser::createAssignment(Token* identifier, nt_Words* words)
{
    return saveNonTerminal(new nt_Assignment(identifier, words));
}

nt_Reset*
InterfaceParser::createReset(Token* identifier, bool negate)
{
    return saveNonTerminal(new nt_Reset(identifier, negate));
}

nt_Reset*
InterfaceParser::createReset(FileLocation const& location)
{
    return saveNonTerminal(new nt_Reset(location));
}

nt_Block*
InterfaceParser::createBlock(nt_IfBlock* ifblock)
{
    return saveNonTerminal(new nt_Block(ifblock));
}

nt_Block*
InterfaceParser::createBlock(nt_Assignment* assignment)
{
    return saveNonTerminal(new nt_Block(assignment));
}

nt_Block*
InterfaceParser::createBlock(nt_Reset* reset)
{
    return saveNonTerminal(new nt_Block(reset));
}

nt_Block*
InterfaceParser::createBlock(nt_Declaration* declaration)
{
    return saveNonTerminal(new nt_Block(declaration));
}

nt_Block*
InterfaceParser::createBlock(nt_AfterBuild* after_build)
{
    return saveNonTerminal(new nt_Block(after_build));
}

nt_Block*
InterfaceParser::createBlock(nt_TargetType* target_type)
{
    return saveNonTerminal(new nt_Block(target_type));
}

nt_Blocks*
InterfaceParser::createBlocks()
{
    return saveNonTerminal(new nt_Blocks);
}

nt_IfClause*
InterfaceParser::createIfClause(nt_Conditional* conditional, nt_Blocks* blocks,
				bool conditional_expected)
{
    return saveNonTerminal(new nt_IfClause(conditional, blocks,
					   conditional_expected));
}

nt_IfClauses*
InterfaceParser::createIfClauses(FileLocation const& location)
{
    return saveNonTerminal(new nt_IfClauses(location));
}

nt_IfBlock*
InterfaceParser::createIfBlock(nt_IfClause* ifclause,
			       nt_IfClauses* elseifs,
			       nt_IfClause* elseclause)
{
    return saveNonTerminal(new nt_IfBlock(ifclause, elseifs, elseclause));
}

void
InterfaceParser::parseFile()
{
    interfaceparse(this);
}

void
InterfaceParser::startFile(std::string const& filename)
{
    this->_interface->setLocalDirectory(
	Util::dirname(Util::canonicalizePath(filename)));
    this->after_builds.clear();
    if (this->debug_parser)
    {
	interfacedebug = 1;
    }
}

void
InterfaceParser::endFile(std::string const&)
{
    if (this->debug_parser)
    {
	interfacedebug = 0;
    }
    evaluateParseTree();
    this->parse_tree = 0;
}

void
InterfaceParser::storeData(YYSTYPE* data)
{
    this->yydata = data;
}

void
InterfaceParser::setToken(Token* t)
{
    this->yydata->token = t;
}

void
InterfaceParser::acceptParseTree(nt_Blocks* blocks)
{
    this->parse_tree = blocks;
}

bool
InterfaceParser::importInterface(Interface const& _interface)
{
    return this->_interface->importInterface(_interface);
}

boost::shared_ptr<Interface>
InterfaceParser::getInterface() const
{
    return this->_interface;
}

std::vector<std::string>
InterfaceParser::getAfterBuilds() const
{
    return this->after_builds;
}

void
InterfaceParser::evaluateParseTree()
{
    if (this->parse_tree)
    {
	evaluateBlocks(this->parse_tree, true);
    }
}

void
InterfaceParser::evaluateBlocks(nt_Blocks const* blocks, bool evaluating)
{
    std::list<nt_Block const*> const& blocklist = blocks->getBlocks();
    for (std::list<nt_Block const*>::const_iterator iter = blocklist.begin();
	 iter != blocklist.end(); ++iter)
    {
	evaluateBlock(*iter, evaluating);
    }
}

void
InterfaceParser::evaluateBlock(nt_Block const* block, bool evaluating)
{
    // omit default so gcc will warn for missing case tags
    switch (block->getBlockType())
    {
      case nt_Block::b_ifblock:
	evaluateIfBlock(block->getIfBlock(), evaluating);
	break;

      case nt_Block::b_reset:
	evaluateReset(block->getReset(), evaluating);
	break;

      case nt_Block::b_assignment:
	evaluateAssignment(block->getAssignment(), evaluating);
	break;

      case nt_Block::b_declaration:
	evaluateDeclaration(block->getDeclaration(), evaluating);
	break;

      case nt_Block::b_after_build:
	evaluateAfterBuild(block->getAfterBuild(), evaluating);
	break;

      case nt_Block::b_targettype:
	evaluateTargetType(block->getTargetType(), evaluating);
	break;
    }
}

void
InterfaceParser::evaluateIfBlock(
    nt_IfBlock const* ifblock, bool evaluating)
{
    // An if block consists of multiple if clauses, a maximum of one
    // of which will evaluate to true.  Whether we are evaluating or
    // not, we check each condition to make sure that it is
    // syntactically correct and that any function calls are valid
    // functions called with the correct number of arguments.  If we
    // are evaluating, we additionally expand all the arguments of all
    // our conditions to make sure they are fully valid.  If errors
    // are encountered expanding any of the conditions, then all the
    // conditions are considered false.  (Otherwise, we might skip the
    // intended true condition and generate spurious errors from
    // evaluating a condition that was supposed to have been false.)
    // Additionally, if we were called with evaluating set to false
    // (because we are nested inside a false condition), then all the
    // conditions are considered to be false.  Once we have evaluated
    // each condition, we call evaluateBlock recursively passing false
    // for evaluating to all but the first condition that evaluated
    // true.  This approach ensures that as much validation as
    // possible is performed even for conditions that would not be
    // evaluated because they are somewhere inside a false
    // conditional.

    std::vector<nt_IfClause const*> const& clauses = ifblock->getClauses();
    unsigned int nclauses = clauses.size();
    std::vector<bool> evaluate_clause(nclauses);
    bool any_errors = false;
    for (unsigned int i = 0; i < nclauses; ++i)
    {
	evaluate_clause[i] = false;
	if (clauses[i]->getConditionalOkay())
	{
	    // The parser was successfully able to parse the conditional.
	    bool truth_value = false;
	    if (evaluateConditional(
		    clauses[i]->getConditional(), evaluating, truth_value))
	    {
		// The condition was semantically valid -- evaluate
		// this part of the if block if the condition
		// evaluated to true.  If evaluating is false,
		// evaluateConditional never sets truth_value to true.
		evaluate_clause[i] = truth_value;
	    }
	    else
	    {
		// evaluateConditional has reported an error.
		QTC::TC("abuild", "InterfaceParser erroneous conditional");
		any_errors = true;
	    }
	}
	else
	{
	    // The parser would have generated an error message in
	    // this case, and getConditional() would return NULL.
	    QTC::TC("abuild", "InterfaceParser found invalid clause");
	    any_errors = true;
	}
    }
    if (any_errors)
    {
	// If any of the clauses were invalid, don't evaluate any of
	// their code.
	for (unsigned int i = 0; i < nclauses; ++i)
	{
	    evaluate_clause[i] = false;
	}
    }
    else
    {
	// Set all but the first true condition to false.
	bool any_true = false;
	for (unsigned int i = 0; i < nclauses; ++i)
	{
	    if (any_true)
	    {
		evaluate_clause[i] = false;
	    }
	    else if (evaluate_clause[i])
	    {
		any_true = true;
	    }
	}
    }

    // Now recursively call evaluateBlocks with evaluation turned off
    // for any false conditions.
    for (unsigned int i = 0; i < nclauses; ++i)
    {
	evaluateBlocks(clauses[i]->getBlocks(), evaluate_clause[i]);
    }
}

void
InterfaceParser::evaluateReset(
    nt_Reset const* reset, bool evaluating)
{
    if (evaluating)
    {
	std::string const& variable = reset->getIdentifier();
	if (reset->isNegate())
	{
	    assert(! variable.empty());
	    Interface::VariableInfo info;
	    if (this->_interface->getVariable(variable, info))
	    {
		QTC::TC("abuild", "InterfaceParser no-reset");
		this->protected_from_reset.insert(variable);
	    }
	    else
	    {
		QTC::TC("abuild", "InterfaceParser ERR no-reset invalid");
		error(reset->getLocation(),
		      "unknown variable " + variable);
	    }
	}
	else
	{
	    std::set<std::string> to_reset;
	    if (variable.empty())
	    {
		QTC::TC("abuild", "InterfaceParser reset all");
		to_reset = this->_interface->getVariableNames();
	    }
	    else
	    {
		QTC::TC("abuild", "InterfaceParser reset variable");
		to_reset.insert(variable);
	    }

	    for (std::set<std::string>::iterator iter = to_reset.begin();
		 iter != to_reset.end(); ++iter)
	    {
		if (this->protected_from_reset.count(*iter) == 0)
		{
		    this->_interface->resetVariable(
			reset->getLocation(), *iter);
		}
	    }
	    this->protected_from_reset.clear();
	}
    }
}

void
InterfaceParser::evaluateAssignment(
    nt_Assignment const* assignment, bool evaluating)
{
    // We can validate flags whether or not we are evaluating since
    // they are static from the perspective of the interface file.

    Token const* flag_token = assignment->getFlag();
    std::string flag;
    if (flag_token)
    {
	flag = flag_token->getValue();
	if (this->supported_flags.count(flag) == 0)
	{
	    QTC::TC("abuild", "InterfaceParser ERR bad flag");
	    error(flag_token->getLocation(),
		  "flag " + flag + " is not supported by this build item");
	}
    }

    if (evaluating)
    {
	nt_Words const* words = assignment->getWords();
	std::deque<std::string> value = evaluateWords(words);
	this->_interface->assignVariable(
	    assignment->getLocation(),
	    assignment->getIdentifier(),
	    value,
	    assignment->getAssignmentType(),
	    flag);
    }
}

void
InterfaceParser::evaluateDeclaration(
    nt_Declaration const* declaration, bool evaluating)
{
    if (evaluating)
    {
	this->_interface->declareVariable(
	    declaration->getLocation(),
	    declaration->getVariableName(),
	    declaration->getType(),
	    declaration->getListType());
    }
}

void
InterfaceParser::evaluateAfterBuild(
    nt_AfterBuild const* after_build, bool evaluating)
{
    if (! this->allow_after_build)
    {
	QTC::TC("abuild", "InterfaceParser ERR disallowed after-build");
	error(after_build->getLocation(),
	      "after-build files are not permitted in this interface file");
    }
    if (evaluating)
    {
	nt_Words const* words = after_build->getArgument();
	std::string filename;
	nt_Argument argument(words);
	if (checkFilenameArgument(&argument, filename))
	{
	    this->after_builds.push_back(filename);
	    this->_interface->normalizeFilename(this->after_builds.back());
	}
    }
}

void
InterfaceParser::evaluateTargetType(
    nt_TargetType const* target_type, bool evaluating)
{
    // Whether we're evaluating or not, if the argument to target_type
    // is a literal, we can verify that it is a valid target type.  If
    // we're evaluating, we can also check it if it's a variable.

    Token const* token = target_type->getToken();
    std::string value = token->getValue();
    assert(! value.empty());
    bool check = true;
    if (value[0] == '$')
    {
	if (evaluating)
	{
	    QTC::TC("abuild", "InterfaceParser checking target_type variable");
	    // check will be set to true and value will be initialized
	    // iff this is a string scalar variable.
	    withVariable(
		token, boost::bind(
		    &InterfaceParser::checkStringOrFilenameVariable,
		    this, token->getLocation(), true,
		    boost::ref(check),
		    boost::ref(value),
		    _1, _2));
	}
	else
	{
	    // We can't check this any further.
	    QTC::TC("abuild", "InterfaceParser not checking target_type var");
	    check = false;
	}
    }

    if (check && (! TargetType::isValid(value)))
    {
	QTC::TC("abuild", "InterfaceParser ERR invalid target_type");
	if (value.empty())
	{
	    QTC::TC("abuild", "InterfaceParser empty target_type value");
	    value = "the empty string";
	}
	error(target_type->getLocation(),
	      value + " is not a valid target type");
    }
    else if (evaluating)
    {
	this->_interface->setTargetType(TargetType::getID(value));
    }
}

std::deque<std::string>
InterfaceParser::evaluateWords(nt_Words const* words)
{
    std::deque<std::string> result;
    std::list<nt_Word const*> const& wordlist = words->getWords();
    for (std::list<nt_Word const*>::const_iterator iter = wordlist.begin();
	 iter != wordlist.end(); ++iter)
    {
	evaluateWord(*iter, result);
    }
    return result;
}

void
InterfaceParser::evaluateWord(nt_Word const* word,
			      std::deque<std::string>& result)
{
    std::list<nt_Word::word_t> const& tokens = word->getTokens();
    result.push_back("");
    for (std::list<nt_Word::word_t>:: const_iterator iter = tokens.begin();
	 iter != tokens.end(); ++iter)
    {
	Token const* token = (*iter).first;
	switch ((*iter).second)
	{
	  case nt_Word::w_variable:
	    withVariable(token,
			 boost::bind(&InterfaceParser::appendVariableValue,
				     this, boost::ref(result),
				     _1, _2));
	    break;

	  case nt_Word::w_environment:
	    result.back() += evaluateEnvironment(token);
	    break;

	  case nt_Word::w_string:
	    result.back() += evaluateToken(token);
	    break;
	}
    }
}

std::string
InterfaceParser::evaluateToken(Token const* token)
{
    std::string value = token->getValue();
    if ((value.length() == 2) && (value[0] == '\\'))
    {
	// Unquote quoted characters
	value = value.substr(1);
    }
    return value;
}

std::string
InterfaceParser::evaluateEnvironment(Token const* token)
{
    boost::regex env_re("\\$\\(ENV:(.*?)\\)");
    std::string variable = getFirstMatch(token, env_re);
    std::string value;

    if (Util::getEnv(variable, &value))
    {
	QTC::TC("abuild", "InterfaceParser environment variable");
    }
    else
    {
	QTC::TC("abuild", "InterfaceParser ERR invalid environment variable");
	error(token->getLocation(),
	      "unknown environment variable " + variable);
    }


    return value;
}

bool
InterfaceParser::evaluateConditional(nt_Conditional const* conditional,
				     bool evaluating,
				     bool& truth_value)
{
    // If we are evaluating, then do full validation on this
    // conditional.  Otherwise, we will consider the condition to be
    // false no matter what we discover (to prevent any conditions
    // that nested inside false conditionals from being evaluated),
    // and we will only check to make sure that any functions we
    // encounter, including those hidden recursively in arguments, are
    // valid functions with the correct number of arguments.

    bool valid = true;
    truth_value = false;
    if (conditional)
    {
	// Exactly one of variable and function will be non-null.
	Token const* variable = conditional->getVariable();
	nt_Function const* function = conditional->getFunction();
	valid = evaluateBooleanOrFunction(
	    variable, function, evaluating, truth_value);
    }
    else
    {
	// A null conditional that was considered valid by the parser
	// (and was therefore passed to this method) can only occur on
	// an else clause.  We automatically consider it to be true.
	truth_value = true;
    }

    if (! evaluating)
    {
	// Whatever we found, consider this false if we're nested
	// inside a false conditional.
	truth_value = false;
    }

    return valid;
}

bool
InterfaceParser::evaluateBooleanOrFunction(
    Token const* variable, nt_Function const* function,
    bool evaluating, bool& truth_value)
{
    bool valid = false;
    if (variable)
    {
	if (evaluateBooleanVariable(variable, evaluating, truth_value))
	{
	    valid = true;
	}
    }
    else
    {
	assert(function != 0);
	if (evaluateFunction(function, evaluating, truth_value))
	{
	    valid = true;
	}
    }
    return valid;
}

bool
InterfaceParser::evaluateBooleanVariable(Token const* token, bool evaluating,
					 bool& truth_value)
{
    // If we're not evaluating, assume the variable is a boolean value
    // whose value is false.  The value doesn't matter, and assuming
    // validity prevents spurious error messages as described in
    // evaluateIfBlock.
    bool valid = true;
    truth_value = false;

    if (evaluating)
    {
	// Get the type and value of the variable.
	withVariable(
	    token, boost::bind(&InterfaceParser::checkBooleanVariable,
			       this, token->getLocation(),
			       boost::ref(valid),
			       boost::ref(truth_value),
			       _1, _2));
    }
    return valid;
}

bool
InterfaceParser::evaluateFunction(nt_Function const* function, bool evaluating,
				  bool& truth_value)
{
    // If we're not evaluating, check only the function's name and
    // number of arguments.  Then recursively check all of its
    // arguments.  If we are evaluating, then actually evaluate the
    // function and get its return value.

    bool valid = true;
    truth_value = false;

    std::string function_name = getFunctionName(function->getFunction());
    FileLocation const& location = function->getLocation();
    std::vector<nt_Argument const*> const& arguments =
	function->getArguments();

    if (function_num_arguments.count(function_name))
    {
	valid = checkNumArguments(
	    location, function_name, arguments,
	    function_num_arguments[function_name]);
    }
    else
    {
	valid = false;
	QTC::TC("abuild", "InterfaceParser ERR invalid function");
	error(function->getLocation(),
	      "unknown function " + function_name);
    }

    if (valid && evaluating)
    {
	valid = (this->*(function_evaluators[function_name]))(
	    location, arguments, truth_value);
    }
    else
    {
	// If we're not evaluating or this is invalid, at least check
	// nested functions to the extent that we can.
	valid = checkNestedFunctions(arguments) && valid;
    }

    return valid;
}

std::string
InterfaceParser::getVariableName(Token const* token)
{
    boost::regex variable_re("\\$\\((.*?)\\)");
    return getFirstMatch(token, variable_re);
}

std::string
InterfaceParser::getFunctionName(Token const* token)
{
    boost::regex function_re("(.*?)[ \t]*\\(");
    return getFirstMatch(token, function_re);
}

std::string
InterfaceParser::getFirstMatch(Token const* token, boost::regex& expression)
{
    // Return the first submatch from matching the token value with
    // the given expression.  This method must be called only when a
    // match is certain, as would be the case with tokens constructed
    // by flex using a compatible expression.
    std::string const& value = token->getValue();
    boost::smatch match;
    assert(boost::regex_match(value, match, expression));
    return match[1].str();
}

bool
InterfaceParser::withVariable(
    Token const* token,
    boost::function<void(std::string const&, Interface::VariableInfo const&)> f)
{
    bool result = false;
    std::string variable_name = getVariableName(token);
    Interface::VariableInfo info;
    if (this->_interface->getVariable(variable_name, info))
    {
	if (info.initialized)
	{
	    result = true;
	    f(variable_name, info);
	}
	else
	{
	    QTC::TC("abuild", "InterfaceParser ERR uninitialized var");
	    error(token->getLocation(), "variable " + variable_name +
		  " is not initialized");
	}
    }
    else
    {
	QTC::TC("abuild", "InterfaceParser ERR unknown variable");
	error(token->getLocation(), "unknown variable " + variable_name);
    }
    return result;
}

void
InterfaceParser::appendVariableValue(std::deque<std::string>& result,
				     std::string const&,
				     Interface::VariableInfo const& info)
{
    // Append the first word of the result, if any to the current
    // word.  Then create additional words for additional words in the
    // value.  This results in reasonable semantics for both single-
    // and multi-word variable expansions.

    std::deque<std::string> value = info.value;
    if (! value.empty())
    {
	if (result.empty())
	{
	    result.push_back("");
	}
	result.back() += value.front();
	value.pop_front();
    }
    while (! value.empty())
    {
	result.push_back(value.front());
	value.pop_front();
    }
}

void
InterfaceParser::checkBooleanVariable(FileLocation const& location,
				      bool& valid,
				      bool& truth_value,
				      std::string const& variable_name,
				      Interface::VariableInfo const& info)
{
    valid = false;
    if ((info.list_type == Interface::l_scalar) &&
	(info.type == Interface::t_boolean))
    {
	valid = true;
	// booleans are normalized internally to "0" and "1" by
	// Interface.
	truth_value = (info.value.front() == "1");
    }
    else
    {
	QTC::TC("abuild", "InterfaceParser ERR non-boolean variable");
	error(location,
	      "variable " + variable_name + " is not a boolean scalar");
    }
}

void
InterfaceParser::checkStringOrFilenameVariable(
    FileLocation const& location, bool string,
    bool& valid, std::string& string_value,
    std::string const& variable_name,
    Interface::VariableInfo const& info)
{
    valid = false;
    if ((info.list_type == Interface::l_scalar) &&
	(info.type == (string ? Interface::t_string : Interface::t_filename)))
    {
	valid = true;
	string_value = info.value.front();
    }
    else
    {
	QTC::TC("abuild", "InterfaceParser ERR non-string/filename variable",
		string ? 0 : 1);
	error(location,
	      "variable " + variable_name + " is not a " +
	      (string ? "string" : "filename") + " scalar");
    }
}

bool
InterfaceParser::checkNumArguments(
    FileLocation const& location, std::string const& function_name,
    std::vector<nt_Argument const*> arguments,
    unsigned int desired_num_arguments)
{
    bool valid = false;
    if (arguments.size() == desired_num_arguments)
    {
	valid = true;
    }
    else
    {
	QTC::TC("abuild", "InterfaceParser ERR wrong number of arguments");
	error(location, "function \"" + function_name + "\" takes " +
	      Util::intToString(desired_num_arguments) +
	      " argument" + (desired_num_arguments == 1 ? "" : "s"));
    }

    return valid;
}

bool
InterfaceParser::evaluateFunctionAnd(
    FileLocation const& location,
    std::vector<nt_Argument const*> const& arguments,
    bool& truth_value)
{
    bool valid = false;
    bool arg1_value = false;
    bool arg2_value = false;
    // Rather than putting the check calls directly into the
    // conditional, assign each check call to a variable and compare
    // the variables so that the second call gets made even if the
    // first one returns false.
    bool arg1_valid = checkBooleanArgument(arguments[0], arg1_value);
    bool arg2_valid = checkBooleanArgument(arguments[1], arg2_value);
    if (arg1_valid && arg2_valid)
    {
	valid = true;
	truth_value = arg1_value && arg2_value;
    }
    return valid;
}

bool
InterfaceParser::evaluateFunctionOr(
    FileLocation const& location,
    std::vector<nt_Argument const*> const& arguments,
    bool& truth_value)
{
    bool valid = false;
    bool arg1_value = false;
    bool arg2_value = false;
    bool arg1_valid = checkBooleanArgument(arguments[0], arg1_value);
    bool arg2_valid = checkBooleanArgument(arguments[1], arg2_value);
    if (arg1_valid && arg2_valid)
    {
	valid = true;
	truth_value = arg1_value || arg2_value;
    }
    return valid;
}

bool
InterfaceParser::evaluateFunctionNot(
    FileLocation const& location,
    std::vector<nt_Argument const*> const& arguments,
    bool& truth_value)
{
    bool valid = false;
    bool arg_value = false;
    if (checkBooleanArgument(arguments[0], arg_value))
    {
	valid = true;
	truth_value = ! arg_value;
    }
    return valid;
}

bool
InterfaceParser::evaluateFunctionEquals(
    FileLocation const& location,
    std::vector<nt_Argument const*> const& arguments,
    bool& truth_value)
{
    bool valid = false;
    std::string arg1;
    std::string arg2;
    bool arg1_valid = checkStringArgument(arguments[0], arg1);
    bool arg2_valid = checkStringArgument(arguments[1], arg2);
    if (arg1_valid && arg2_valid)
    {
	valid = true;
	truth_value = (arg1 == arg2);
    }

    return valid;
}

bool
InterfaceParser::evaluateFunctionMatches(
    FileLocation const& location,
    std::vector<nt_Argument const*> const& arguments,
    bool& truth_value)
{
    bool valid = false;
    std::string str;
    std::string regex;
    bool arg1_valid = checkStringArgument(arguments[0], str);
    bool arg2_valid = checkStringArgument(arguments[1], regex);
    if (arg1_valid && arg2_valid)
    {
	std::deque<std::string> words;
	words.push_back(str);
	valid = containsMatch(arguments[1]->getLocation(),
			      words, regex, truth_value);
    }

    return valid;
}

bool
InterfaceParser::evaluateFunctionContains(
    FileLocation const& location,
    std::vector<nt_Argument const*> const& arguments,
    bool& truth_value)
{
    bool valid = false;
    nt_Words const* list = 0;
    std::string str;
    bool arg1_valid = checkWordsArgument(arguments[0], list);
    bool arg2_valid = checkStringArgument(arguments[1], str);
    if (arg1_valid && arg2_valid)
    {
	valid = true;

	// Search the results even if there are errors in evaluating
	// the words.
	truth_value = false;
	std::deque<std::string> words = evaluateWords(list);
	for (std::deque<std::string>::iterator iter = words.begin();
	     iter != words.end(); ++iter)
	{
	    if (str == (*iter))
	    {
		truth_value = true;
		break;
	    }
	}
    }

    return valid;
}

bool
InterfaceParser::evaluateFunctionContainsmatch(
    FileLocation const& location,
    std::vector<nt_Argument const*> const& arguments,
    bool& truth_value)
{
    bool valid = false;
    nt_Words const* list = 0;
    std::string regex;
    bool arg1_valid = checkWordsArgument(arguments[0], list);
    bool arg2_valid = checkStringArgument(arguments[1], regex);
    if (arg1_valid && arg2_valid)
    {
	// Search the results even if there are errors in evaluating
	// the words.
	truth_value = false;
	std::deque<std::string> words = evaluateWords(list);
	valid = containsMatch(arguments[1]->getLocation(),
			      words, regex, truth_value);
    }

    return valid;
}

bool
InterfaceParser::containsMatch(FileLocation const& location,
			       std::deque<std::string> const& words,
			       std::string const& regex,
			       bool& truth_value)
{
    bool valid = true;
    truth_value = false;
    try
    {
	boost::regex expression(regex);
	boost::smatch match;
	for (std::deque<std::string>::const_iterator iter = words.begin();
	     iter != words.end(); ++iter)
	{
	    if (boost::regex_match(*iter, match, expression))
	    {
		truth_value = true;
		break;
	    }
	}
    }
    catch (boost::bad_expression)
    {
	valid = false;
	QTC::TC("abuild", "InterfaceParser ERR bad regular expression");
	error(location, "invalid regular expression " + regex);
    }

    return valid;
}

bool
InterfaceParser::checkBooleanArgument(nt_Argument const* argument,
				      bool& truth_value)
{
    // Make sure this argument is either a boolean variable or a
    // function call.  If okay, get the result.

    bool valid = false;

    // Exactly one of these will be non-NULL.
    nt_Function const* function = argument->getFunction();
    nt_Words const* words = argument->getWords();

    Token const* variable = 0;

    if (words)
    {
	assert(function == 0);
	// Determine whether this is a single variable.
	std::list<nt_Word const*> wordlist = words->getWords();
	if (wordlist.size() == 1)
	{
	    std::list<nt_Word::word_t> const& tokens =
		wordlist.front()->getTokens();
	    if (tokens.size() == 1)
	    {
		nt_Word::word_t const& word = tokens.front();
		if (word.second == nt_Word::w_variable)
		{
		    // This is a variable
		    variable = word.first;
		}
	    }
	}
    }

    if (variable || function)
    {
	valid = evaluateBooleanOrFunction(
	    variable, function, true, truth_value);
    }
    else
    {
	QTC::TC("abuild", "InterfaceParser ERR invalid boolean argument");
	error(argument->getLocation(),
	      "this argument must be a function call or a boolean variable");
    }

    return valid;
}

bool
InterfaceParser::checkStringArgument(nt_Argument const* argument,
				     std::string& value)
{
    return checkStringOrFilenameArgument(argument, true, value);
}

bool
InterfaceParser::checkFilenameArgument(nt_Argument const* argument,
				       std::string& value)
{
    return checkStringOrFilenameArgument(argument, false, value);
}

bool
InterfaceParser::checkStringOrFilenameArgument(
    nt_Argument const* argument, bool string, std::string& value)
{
    // Make sure this argument is not a function and that the result
    // expands to a single word.

    bool valid = false;
    if (argument->getFunction())
    {
	QTC::TC("abuild", "InterfaceParser ERR function for str/filename arg",
		string ? 0 : 1);
    }
    else
    {
	nt_Words const* words = argument->getWords();
	// Make sure this is a single word and that any variables
	// inside of it are string scalars.
	std::list<nt_Word const*> wordlist = words->getWords();
	if (wordlist.size() == 1)
	{
	    valid = true;
	    std::string tmp_value;
	    std::list<nt_Word::word_t> const& tokens =
		wordlist.front()->getTokens();
	    for (std::list<nt_Word::word_t>::const_iterator iter =
		     tokens.begin();
		 iter != tokens.end(); ++iter)
	    {
		Token const* token = (*iter).first;
		switch ((*iter).second)
		{
		  case nt_Word::w_variable:
		    {
			bool var_valid = false;
			std::string var_value;
			withVariable(
			    token,
			    boost::bind(
				&InterfaceParser::checkStringOrFilenameVariable,
				this, token->getLocation(), string,
				boost::ref(var_valid),
				boost::ref(var_value),
				_1, _2));
			QTC::TC("abuild", "InterfaceParser expand string/filename",
				((var_valid ? 0 : 1) +
				 (string ? 0 : 2)));
			valid = valid && var_valid;
			value += var_value;
		    }
		    break;

		  case nt_Word::w_environment:
		    value += evaluateEnvironment(token);
		    break;

		  case nt_Word::w_string:
		    value += evaluateToken(token);
		    break;
		}
	    }
	}
    }

    if (! valid)
    {
	error(argument->getLocation(),
	      std::string("this argument must be a ") +
	      (string ? "string" : "filename"));
    }

    return valid;
}

bool
InterfaceParser::checkWordsArgument(nt_Argument const* argument,
				    nt_Words const*& value)
{
    bool valid = true;
    value = argument->getWords();
    if (value == 0)
    {
	valid = false;
	QTC::TC("abuild", "InterfaceParser ERR function as words arg");
	error(argument->getLocation(),
	      "this argument may not be a function");
	nt_Function const* function = argument->getFunction();
	assert(function != 0);
	bool truth_value = false;
	evaluateFunction(function, true, truth_value);
    }
    return valid;
}

bool
InterfaceParser::checkNestedFunctions(
    std::vector<nt_Argument const*> const& arguments)
{
    // If any arguments are functions, check them recursively.  This
    // only gets called when we're not evaluating.

    bool valid = true;

    for (std::vector<nt_Argument const*>::const_iterator iter =
	     arguments.begin();
	 iter != arguments.end(); ++iter)
    {
	nt_Function const* function = (*iter)->getFunction();
	if (function)
	{
	    QTC::TC("abuild", "InterfaceParser check nested function");
	    bool truth_value;
	    valid = evaluateFunction(function, false, truth_value) && valid;
	}
    }

    return valid;
}
