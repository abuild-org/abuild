#include <Interface.hh>

#include <assert.h>
#include <Util.hh>
#include <Error.hh>
#include <QTC.hh>
#include <FlagData.hh>

Interface::Interface(std::string const& item_name,
		     std::string const& item_platform,
		     Error& error, std::string const& local_dir) :
    error(error),
    item_name(item_name),
    item_platform(item_platform),
    target_type(TargetType::tt_all)
{
    setLocalDirectory(local_dir);
}

void
Interface::setLocalDirectory(std::string const& local_dir)
{
    this->local_directory = local_dir;
    assert(Util::isAbsolutePath(this->local_directory));
    Util::normalizePathSeparators(this->local_directory);
}

bool
Interface::importInterface(Interface const& other)
{
    bool status = true;

    for (std::map<std::string, Variable>::const_iterator iter =
	     other.symbol_table.begin();
	 iter != other.symbol_table.end(); ++iter)
    {
	Variable const& var = (*iter).second;
	if (var.scope == s_local)
	{
	    // Local variables, including their declarations, are not
	    // imported at all.
	    continue;
	}

	if (this->declareVariable(var.declare_location, var.target_type,
				  var.name, var.scope,
				  var.type, var.list_type))
	{
	    // Import reset history without the affect of the reset
	    // operation.  Although the reset is "local" (so its
	    // affect shouldn't be imported), it is useful for
	    // debugging purposes to see the reset history of a
	    // variable when reconstructing how that variable got its
	    // value.
	    for (std::list<Reset>::const_iterator riter =
		     var.reset_history.begin();
		 riter != var.reset_history.end(); ++riter)
	    {
		Reset const& reset = *riter;
		if (! this->resetVariable(
			reset.location, var.name,
			reset.item_name, reset.item_platform, false))
		{
		    status = false;
		}
	    }
	    for (std::list<Assignment>::const_iterator aiter =
		     var.assignment_history.begin();
		 aiter != var.assignment_history.end(); ++aiter)
	    {
		Assignment const& assignment = *aiter;
		if ((var.scope == s_recursive) ||
		    (assignment.item_name == other.item_name))
		{
		    if (! this->assignVariable(assignment.location,
					       var.name, assignment.value,
					       assignment.assignment_type,
					       assignment.flag,
					       assignment.item_name,
					       assignment.item_platform))
		    {
			status = false;
		    }
		}
	    }
	}
	else
	{
	    status = false;
	}
    }

    return status;
}

void
Interface::setTargetType(TargetType::target_type_e target_type)
{
    this->target_type = target_type;
}

bool
Interface::declareVariable(FileLocation const& location,
			   std::string const& variable_name,
			   scope_e scope, type_e type, list_e list_type)
{
    return declareVariable(location, this->target_type, variable_name,
			   scope, type, list_type);
}

bool
Interface::declareVariable(FileLocation const& location,
			   TargetType::target_type_e target_type,
			   std::string const& variable_name,
			   scope_e scope, type_e type, list_e list_type)
{
    bool status = true;

    if (this->symbol_table.count(variable_name))
    {
	Variable& var = this->symbol_table[variable_name];
	if (var.declare_location == location)
	{
	    // Okay -- this is a duplicate of an existing declaration,
	    // which is a normal case when the same interface file is
	    // loaded through more than one dependency path.  We don't
	    // care about duplicate declarations from multiple
	    // platform instances of an interface since the affect of
	    // a declaration, unlike that of an assignment, can never
	    // be influenced by the platform.
	    QTC::TC("abuild", "Interface repeat declaration");
	}
	else
	{
	    QTC::TC("abuild", "Interface ERR conflicting declaration");
	    status = false;
	    this->error.error(location, "variable " + variable_name +
			      " has already been declared");
	    this->error.error(var.declare_location,
			      "here is the previous declaration");
	}
    }
    else
    {
	// Add this variable to the symbol table
	this->symbol_table[variable_name] =
	    Variable(variable_name, location, target_type,
		     scope, type, list_type);
    }

    return status;
}

bool
Interface::assignVariable(FileLocation const& location,
			  std::string const& variable_name,
			  std::string const& value,
			  assign_e assignment_type)
{
    std::deque<std::string> values;
    values.push_back(value);
    return assignVariable(location, variable_name, values,
			  assignment_type, "",
			  this->item_name, this->item_platform);
}

bool
Interface::assignVariable(FileLocation const& location,
			  std::string const& variable_name,
			  std::deque<std::string> const& values,
			  assign_e assignment_type,
			  std::string const& flag)
{
    return assignVariable(location, variable_name, values,
			  assignment_type, flag,
			  this->item_name, this->item_platform);
}


bool
Interface::assignVariable(FileLocation const& location,
			  std::string const& variable_name,
			  std::deque<std::string> const& ovalues,
			  assign_e assignment_type,
			  std::string const& flag,
			  std::string const& interface_item_name,
			  std::string const& interface_item_platform)
{
    bool status = true;
    Assignment const* old_assignment = 0;

    if (this->symbol_table.count(variable_name))
    {
	Variable& var = this->symbol_table[variable_name];

	for (std::list<Assignment>::iterator iter =
		 var.assignment_history.begin();
	     iter != var.assignment_history.end(); ++iter)
	{
	    if ((*iter).location == location)
	    {
		if ((*iter).item_platform == interface_item_platform)
		{
		    old_assignment = &(*iter);
		    break;
		}
		else
		{
		    QTC::TC("abuild", "Interface same location, not instance");
		}
	    }
	}

	// Filter out empty strings from values with the special case
	// exception of allowing a single-word scalar value to be the
	// empty string.
	std::deque<std::string> values;
	if ((var.list_type == l_scalar) && (ovalues.size() == 1))
	{
	    values = ovalues;
	}
	else
	{
	    for (std::deque<std::string>::const_iterator iter = ovalues.begin();
		 iter != ovalues.end(); ++iter)
	    {
		if ((*iter).empty())
		{
		    QTC::TC("abuild", "Interface skip empty string");
		}
		else
		{
		    values.push_back(*iter);
		}
	    }
	}

	if ((var.list_type == l_scalar) &&
	    (! (values.empty() || (values.size() == 1))))
	{
	    QTC::TC("abuild", "Interface ERR multiword scalar");
	    status = false;
	    error.error(location,
			"multiple words may not be assigned"
			" to scalar variable " + variable_name);
	}

	// Check and, if needed, adjust each value based on the type.
	for (std::deque<std::string>::iterator iter = values.begin();
	     iter != values.end(); ++iter)
	{
	    std::string& value = *iter;
	    if (var.type == t_boolean)
	    {
		// Accept "true", "1", "false", or "0", but internally
		// normalize to "1" and "0".  InterfaceParser knows
		// about this normalization.
		if ((value == "1") || (value == "0"))
		{
		    // okay
		}
		else if (value == "true")
		{
		    value = "1";
		}
		else if (value == "false")
		{
		    value = "0";
		}
		else
		{
		    QTC::TC("abuild", "Interface ERR bad boolean value");
		    status = false;
		    error.error(location, "value " + value + " is invalid for"
				" boolean variable " + variable_name);
		}
	    }
	    else if (var.type == t_filename)
	    {
		if (value.empty())
		{
		    QTC::TC("abuild", "Interface ERR empty filename");
		    status = false;
		    error.error(location, "the empty string may not be used"
				" as a value for filename variable " +
				variable_name);
		}
		else
		{
		    normalizeFilename(value);
		}
	    }
	}

	Assignment assignment(location, assignment_type, flag,
			      interface_item_name, interface_item_platform,
			      values);

	if (old_assignment)
	{
	    // If this assertion fails, it means we thought we were
	    // looking at an assignment statement that we had seen
	    // before, but for some reason, the value is different
	    // this time.  This used to be able to happen when two
	    // "instances" of the same interface, "instantiated" for
	    // different platforms, were imported into the same
	    // interface object.
	    assert(old_assignment->value == values);

	    // Okay -- this is a duplicate of an existing assignment.
	    QTC::TC("abuild", "Interface repeat assignment");
	    return true;
	}

	// For lists, assignment_history contains all list
	// assignments, and all values are used when retrieving the
	// value of the variable.  For scalars, assignment_history
	// contains zero or more fallback assignments followed by zero
	// or one normal assignments followed by zero or more override
	// assignments.  When we get the value of a variable, after
	// filtering for flags, we take the last item on the
	// assignment history.

	if (var.list_type == l_scalar)
	{
	    if (assignment_type == a_normal)
	    {
		Assignment const* previous_normal_assignment = 0;
		for (std::list<Assignment>::const_iterator aiter =
			 var.assignment_history.begin();
		     aiter != var.assignment_history.end(); ++aiter)
		{
		    if ((*aiter).assignment_type == a_normal)
		    {
			previous_normal_assignment = &(*aiter);
		    }
		    break;
		}

		if (previous_normal_assignment)
		{
		    bool same_location =
			previous_normal_assignment->location == location;
		    if (same_location &&
			(values == previous_normal_assignment->value))
		    {
			QTC::TC("abuild", "Interface duplicate assignment different platform same value");
			// Ignore duplicate normal assignment coming
			// from a different instance of the same
			// interface as long as the values are the
			// same.
		    }
		    else
		    {
			status = false;
			error.error(location, "variable " + variable_name +
				    " already has a value");
			if (same_location)
			{
			    QTC::TC("abuild", "Interface ERR variable assigned by other instance");
			    error.error(
				location,
				"conflicting assignment was made by this "
				"build item's instance on platform " +
				previous_normal_assignment->item_platform +
				"; see \"Interface Errors\" subsection of the "
				"\"Cross-Platform Dependencies\" section "
				"of the manual for an explanation and list "
				"of remedies");
			}
			else
			{
			    QTC::TC("abuild", "Interface ERR variable assigned elsewhere");
			    error.error(previous_normal_assignment->location,
					"here is the previous assignment");
			}
		    }
		}
	    }
	}
	else
	{
	    if (assignment_type != a_normal)
	    {
		status = false;
		QTC::TC("abuild", "Interface ERR bad list assignment type");
		error.error(location, "fallback and override assignments "
			    "are not valid for list variables");
	    }
	}

	// No default for switch statement so gcc will warn for
	// missing case tags
	if (var.list_type == l_scalar)
	{
	    QTC::TC("abuild", "Interface update scalar",
		    (assignment_type == a_fallback) ? 0
		    : (assignment_type == a_normal) ? 1
		    : (assignment_type == a_override) ? 2
		    : 999);	// can't happen
	    if (assignment_type == a_fallback)
	    {
		var.assignment_history.push_front(assignment);
	    }
	    else
	    {
		var.assignment_history.push_back(assignment);
	    }
	}
	else
	{
	    QTC::TC("abuild", "Interface update list");
	    var.assignment_history.push_back(assignment);
	}
    }
    else
    {
	QTC::TC("abuild", "Interface ERR assign unknown variable");
	status = false;
	this->error.error(location, "assigning to unknown variable " +
			  variable_name);
    }

    return status;
}

bool
Interface::resetVariable(FileLocation const& location,
			 std::string const& variable_name)
{
    return resetVariable(location, variable_name,
			 this->item_name, this->item_platform, true);
}

bool
Interface::resetVariable(FileLocation const& location,
			 std::string const& variable_name,
			 std::string const& interface_item_name,
			 std::string const& interface_item_platform,
			 bool clear_assignment_history)
{
    bool status = true;

    if (this->symbol_table.count(variable_name))
    {
	Variable& var = this->symbol_table[variable_name];
	if (clear_assignment_history)
	{
	    var.assignment_history.clear();
	}
	bool found = false;
	for (std::list<Reset>::const_iterator iter = var.reset_history.begin();
	     iter != var.reset_history.end(); ++iter)
	{
	    Reset const& r = *iter;
	    if ((r.location == location) &&
		(r.item_name == interface_item_name) &&
		(r.item_platform == interface_item_platform))
	    {
		found = true;
		break;
	    }
	}
	if (! found)
	{
	    var.reset_history.push_back(
		Reset(location, interface_item_name, interface_item_platform));
	}
    }
    else
    {
	QTC::TC("abuild", "Interface ERR reset unknown variable");
	status = false;
	this->error.error(location, "resetting unknown variable " +
			  variable_name);
    }

    return status;
}

bool
Interface::getVariable(std::string const& variable_name,
		       VariableInfo& info) const
{
    FlagData empty;
    return getVariable(variable_name, empty, info);
}

bool
Interface::getVariable(std::string const& variable_name,
		       FlagData const& flag_data, VariableInfo& info) const
{
    bool status = true;

    if (this->symbol_table.count(variable_name))
    {
	Variable const& var =
	    (*(this->symbol_table.find(variable_name))).second;
	info.target_type = var.target_type;
	info.type = var.type;
	info.scope = var.scope;
	info.list_type = var.list_type;
	info.value.clear();

	// Filter assignment history based on flag data.
	std::list<Assignment> assignment_history;
	for (std::list<Assignment>::const_iterator iter =
		 var.assignment_history.begin();
	     iter != var.assignment_history.end(); ++iter)
	{
	    Assignment const& assignment = *iter;
	    if (assignment.flag.empty() ||
		flag_data.isSet(
		    assignment.item_name, assignment.flag))
	    {
		assignment_history.push_back(assignment);
	    }
	    if (assignment_history.empty() &&
		(! var.assignment_history.empty()))
	    {
		QTC::TC("abuild", "Abuild flag made variable uninitialized");
	    }
	}

	if (assignment_history.empty())
	{
	    QTC::TC("abuild", "Interface uninitialized variable",
		    ((var.list_type == l_append) ? 0 :
		     (var.list_type == l_prepend) ? 1 :
		     2));
	    info.initialized = false;
	}
	else
	{
	    info.initialized = true;
	    // Omit default statement so gcc will warn for missing
	    // case tags.
	    switch (var.list_type)
	    {
	      case l_scalar:
		QTC::TC("abuild", "Interface scalar value");
		info.value = assignment_history.back().value;
		break;

	      case l_append:
		QTC::TC("abuild", "Interface appendlist value");
		// Append all words of all assignments in order of
		// assignment.
		for (std::list<Assignment>::const_iterator iter =
			 assignment_history.begin();
		     iter != assignment_history.end(); ++iter)
		{
		    Assignment const& assignment = (*iter);
		    for (std::deque<std::string>::const_iterator viter =
			     assignment.value.begin();
			 viter != assignment.value.end(); ++viter)
		    {
			info.value.push_back(*viter);
		    }
		}
		break;

	      case l_prepend:
		QTC::TC("abuild", "Interface prependlist value");
		// In order of assignment, prepend values of each
		// assignment to the result, but preserve the original
		// order of the words from each assignment.
		for (std::list<Assignment>::const_iterator iter =
			 assignment_history.begin();
		     iter != assignment_history.end(); ++iter)
		{
		    Assignment const& assignment = (*iter);
		    for (std::deque<std::string>::const_reverse_iterator viter =
			     assignment.value.rbegin();
			 viter != assignment.value.rend(); ++viter)
		    {
			info.value.push_front(*viter);
		    }
		}
		break;
	    }
	}
    }
    else
    {
	QTC::TC("abuild", "Interface ERR get unknown variable");
	status = false;
    }

    return status;
}

std::string
Interface::unparse_type(scope_e scope, type_e type, list_e list_type)
{
    std::string result;
    switch (scope)
    {
      case s_recursive:
	break;

      case s_nonrecursive:
	result += "non-recursive ";
	break;

      case s_local:
	result += "local ";
	break;
    }
    if (list_type != l_scalar)
    {
	result += "list ";
    }
    switch (type)
    {
      case t_string:
	result += "string";
	break;

      case t_boolean:
	result += "boolean";
	break;

      case t_filename:
	result += "filename";
	break;
    }
    switch (list_type)
    {
      case l_scalar:
	break;

      case l_append:
	result += " append";
	break;

      case l_prepend:
	result += " prepend";
	break;
    }

    return result;
}

std::string
Interface::unparse_assignment_type(assign_e assignment_type)
{
    std::string result;
    switch (assignment_type)
    {
      case Interface::a_normal:
	result += "normal";
	break;

      case Interface::a_override:
	result += "override";
	break;

      case Interface::a_fallback:
	result += "fallback";
	break;
    }
    return result;
}

std::map<std::string, Interface::VariableInfo>
Interface::getVariablesForTargetType(
    TargetType::target_type_e target_type,
    FlagData const& flag_data) const
{
    std::map<std::string, Interface::VariableInfo> result;

    std::set<std::string> names = getVariableNames();
    for (std::set<std::string>::iterator iter = names.begin();
	 iter != names.end(); ++iter)
    {
	std::string const& name = *iter;
	VariableInfo info;
	assert(getVariable(name, flag_data, info));
	if ((info.target_type == TargetType::tt_all) ||
	    (info.target_type == target_type))
	{
	    result[name] = info;
	}
    }

    return result;
}

std::set<std::string>
Interface::getVariableNames() const
{
    std::set<std::string> result;
    for (std::map<std::string, Variable>::const_iterator iter =
	     this->symbol_table.begin();
	 iter != this->symbol_table.end(); ++iter)
    {
	result.insert((*iter).first);
    }
    return result;
}

void
Interface::dump(std::ostream& out) const
{
    out << "<?xml version=\"1.0\"?>" << std::endl;
    out << "<interface version=\"1\" item-name=\""
	<< Util::XMLify(this->item_name, true) << "\" item-platform=\""
	<< Util::XMLify(this->item_platform, true) << "\">" << std::endl;
    for (std::map<std::string, Variable>::const_iterator iter =
	     this->symbol_table.begin();
	 iter != this->symbol_table.end(); ++iter)
    {
	std::string const& variable_name = (*iter).first;
	Variable const& var = (*iter).second;

	out << " <variable name=\"" << Util::XMLify(variable_name, true)
	    << "\" type=\"" << unparse_type(var.scope, var.type, var.list_type)
	    << "\" target-type=\"" << TargetType::getName(var.target_type)
	    << "\" declaration-location=\""
	    << Util::XMLify(var.declare_location, true)
	    << "\">" << std::endl;
	if (! var.reset_history.empty())
	{
	    QTC::TC("abuild", "Interface dump reset history");
	    out << "  <reset-history>" << std::endl;
	    for (std::list<Reset>::const_iterator iter =
		     var.reset_history.begin();
		 iter != var.reset_history.end(); ++iter)
	    {
		Reset const& r = *iter;
		out << "   <reset item-name=\""
		    << Util::XMLify(r.item_name, true)
		    << "\" item-platform=\""
		    << Util::XMLify(r.item_platform, true)
		    << "\" location=\""
		    << Util::XMLify(r.location, true) << "/>" << std::endl;
	    }
	    out << "  </reset-history>" << std::endl;
	}
	if (var.assignment_history.empty())
	{
	    QTC::TC("abuild", "Interface dump unassigned");
	}
	else
	{
	    out << "  <assignment-history>" << std::endl;
	    for (std::list<Assignment>::const_iterator iter =
		     var.assignment_history.begin();
		 iter != var.assignment_history.end(); ++iter)
	    {
		Assignment const& assignment = *iter;
		out << "   <assignment assignment-type=\""
		    << unparse_assignment_type(assignment.assignment_type)
		    << "\" item-name=\""
		    << Util::XMLify(assignment.item_name, true)
		    << "\" item-platform=\""
		    << Util::XMLify(assignment.item_platform, true)
		    << "\" location=\""
		    << Util::XMLify(assignment.location, true)
		    << "\"";
		if (! assignment.flag.empty())
		{
		    QTC::TC("abuild", "Interface dump flag");
		    out << " flag=\""
			<< Util::XMLify(assignment.flag, true)
			<< "\"";
		}
		out << ">" << std::endl;
		for (std::deque<std::string>::const_iterator viter =
			 assignment.value.begin();
		     viter != assignment.value.end(); ++viter)
		{
		    out << "    <value value=\""
			<< Util::XMLify(*viter, true) << "\"/>" << std::endl;
		}
		out << "   </assignment>" << std::endl;
	    }
	    out << "  </assignment-history>" << std::endl;
	}
	out << " </variable>" << std::endl;
    }
    out << "</interface>" << std::endl;
}

void
Interface::normalizeFilename(std::string& filename)
{
    Util::normalizePathSeparators(filename);
    assert(! filename.empty());
    if (! Util::isAbsolutePath(filename))
    {
	if (filename == ".")
	{
	    filename = this->local_directory;
	}
	else
	{
	    filename = this->local_directory + "/" + filename;
	}
    }
    filename = Util::canonicalizePath(filename);
}
