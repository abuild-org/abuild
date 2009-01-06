#ifndef __INTERFACE_HH__
#define __INTERFACE_HH__

// This class implements the internal representation of build item
// interfaces as defined in Abuild.interface.  The InterfaceParser
// class interfaces with the flex and bison code to read
// Abuild.interface files.  This class doesn't know about the file
// formats but only about the underlying information.

#include <string>
#include <deque>
#include <vector>
#include <set>
#include <map>
#include <list>
#include <iostream>
#include <TargetType.hh>
#include <FileLocation.hh>

class Error;
class FlagData;

class Interface
{
  public:
    enum type_e
    {
	t_boolean,
	t_string,
	t_filename
    };

    enum list_e
    {
	l_scalar,
	l_append,
	l_prepend
    };

    enum assign_e
    {
	a_normal,
	a_override,
	a_fallback
    };

    class VariableInfo
    {
      public:
	TargetType::target_type_e target_type;
	type_e type;
	list_e list_type;
	bool initialized;
	std::deque<std::string> value;
    };

    // Constuct an empty Interface object.  local_directory is the
    // directory from which local paths are to be resolved.  Local
    // paths are resolved at the time at which they are added to an
    // Interface object, so this value has no impact on items imported
    // from other Interface objects.  The item_name and item_platform
    // of an interface object are attached to every assignment that
    // happens for that interface object.  Interface flags are
    // associated with item names to delimit their scope.  There is no
    // requirement or expectation that all Interface objects in the
    // system have unique item names since an item's interface may be
    // instantiated for multiple platforms.  The intended mode of
    // operation is that the item name of the interface is the name of
    // the build item responsible for its creation.  Ordinarily, when
    // the interface system encounters the same assignment statement
    // more than once for one Interface object, all but the first
    // occurrence are ignored.  This happens when the same build item
    // appears more than once in a build item's dependency chain.
    // However, there are some cases in which one item may have more
    // than one platform instance of another item's interface in its
    // dependency chain at a time.  (This can only happen with
    // platform-specific dependencies.)  In this case, we want to
    // evaluate the assignment once for each platform.
    Interface(std::string const& item_name,
	      std::string const& item_platform,
	      Error&, std::string const& local_directory);

    // Reset the local directory.  This has no impact on already
    // established values; it affects only future assignments.
    void setLocalDirectory(std::string const&);

    // Import the contents of another Interface object, merging it
    // with this one.  Returns true iff there are no errors.  Any
    // errors found are reported via the Error object of the receiving
    // iterator.
    bool importInterface(Interface const& other);

    // Set the target type to which subsequent declarations apply.
    void setTargetType(TargetType::target_type_e);

    // Variable manipulation methods all return true iff successful.
    // Any errors are also reported through the error object, a
    // reference to which is provided at the time of construction.
    // The FileLocation argument indicates where the declaration comes
    // from.  For internally set variables, use an empty FileLocation.

    // Declare a variable.  It is an error if the variable is already
    // declared.  See also private declareVariable.
    bool declareVariable(FileLocation const&,
			 std::string const& variable_name,
			 type_e type, list_e list_type);

    // Assign a value to a scalar variable.  Calls the deque form of
    // assignVariable with a single-element deque.
    bool assignVariable(FileLocation const&,
			std::string const& variable_name,
			std::string const& value,
			assign_e assignment_type);

    // Assign a value to a variable.  If the variable is a scalar, the
    // values deque must have exactly one element.  If the variable is
    // a list, values are appended or prepended to the list based on
    // how the list was declared.  The values must be consistent with
    // the variable type.  If assignment_type is a_normal and the
    // variable is a scalar, it is an error if the variable already
    // has a value.  List variables cannot be set with a_override or
    // a_fallback.  If flag is a non-empty string, this assignment
    // will be visible only when this flag associated with this
    // interface name in a call to getVariable().
    bool assignVariable(FileLocation const&,
			std::string const& variable_name,
			std::deque<std::string> const& values,
			assign_e assignment_type,
			std::string const& flag);

    // Reset a variable's assignment history and value.  This is a
    // local operation.  When this interface is imported, the reset
    // command will not be seen by the importer.  Instead, the effect
    // of the import will be seen.  For example, if interface Q has
    // variables A and B, interface R has variables C and D, and
    // interface S has E and F, if R imports Q and then resets B and
    // D, and then S imports Q and R, S will still see B, but it will
    // not get D.
    bool resetVariable(FileLocation const&,
		       std::string const& variable_name);

    // Get the value of a variable and its type information.  The
    // value of an uninitialized scalar is an empty deque.
    // Initialized scalars always have exactly one element in the
    // deque.  Returns true iff the variable is known.
    bool getVariable(std::string const& variable_name,
		     VariableInfo& info) const;
    bool getVariable(std::string const& variable_name,
		     FlagData const&, VariableInfo& info) const;

    // Describe the type of a variable -- for debugging
    static std::string unparse_type(type_e type, list_e list_type);
    static std::string unparse_assignment_type(assign_e assignment_type);

    // Return a map of variable name to value for each variable
    // declared with target type all or with the given target type.
    // Uninitialized variables are returned as well as initialized
    // variables.  Uninitialized variables have the empty deque as
    // their value.
    std::map<std::string, VariableInfo> getVariablesForTargetType(
	TargetType::target_type_e, FlagData const&) const;

    // Return a set of the names of all known variables.
    std::set<std::string> getVariableNames() const;

    // Dump full debugging data to a given output stream
    void dump(std::ostream& out) const;

    // Normalize path separator characters in a filename.  If the file
    // is local, prepend the local directory.
    void normalizeFilename(std::string& filename);

  private:
    Interface(Interface const&);
    Interface& operator=(Interface const&);

    bool assignVariable(FileLocation const&,
			std::string const& variable_name,
			std::deque<std::string> const& values,
			assign_e assignment_type,
			std::string const& flag,
			std::string const& interface_item_name,
			std::string const& interface_item_platform);

    class Assignment
    {
      public:
	Assignment(FileLocation const& location,
		   assign_e assignment_type,
		   std::string const& flag,
		   std::string const& item_name,
		   std::string const& item_platform,
		   std::deque<std::string> const& value) :
	    location(location),
	    assignment_type(assignment_type),
	    flag(flag),
	    item_name(item_name),
	    item_platform(item_platform),
	    value(value)
	{
	}

	FileLocation location;
	assign_e assignment_type;
	std::string flag;
	std::string item_name;
	std::string item_platform;
	std::deque<std::string> value;
    };

    class Reset
    {
      public:
	Reset(FileLocation const& location,
	      std::string const& item_name,
	      std::string const& item_platform) :
	    location(location),
	    item_name(item_name),
	    item_platform(item_platform)
	{
	}

	FileLocation location;
	std::string item_name;
	std::string item_platform;
    };

    class Variable
    {
      public:
	// Provide default constructor so we can store this in a map.
	// We'll just leave it uninitialized.  The variable name will
	// be the empty string, so we'll be able to tell if it ever
	// matters.
	Variable()
	{
	}

	Variable(std::string const& name,
		 FileLocation const& declare_location,
		 TargetType::target_type_e target_type,
		 type_e type,
		 list_e list_type) :
	    name(name),
	    declare_location(declare_location),
	    target_type(target_type),
	    type(type),
	    list_type(list_type)
	{
	}

	std::string name;
	FileLocation declare_location;
	TargetType::target_type_e target_type;
	type_e type;
	list_e list_type;
	std::list<Reset> reset_history;
	// See comments in assignVariable for how assignment_history
	// is used.
	std::list<Assignment> assignment_history;
    };

    // The real declareVariable -- also takes a target type
    bool declareVariable(FileLocation const&,
			 TargetType::target_type_e target_type,
			 std::string const& variable_name,
			 type_e type, list_e list_type);

    Error& error;
    std::string item_name;
    std::string item_platform;
    std::map<std::string, Variable> symbol_table;
    std::string local_directory;
    TargetType::target_type_e target_type;
};

#endif // __INTERFACE_HH__
