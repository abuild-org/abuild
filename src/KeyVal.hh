#ifndef __KEYVAL_HH__
#define __KEYVAL_HH__

#include <string>
#include <set>
#include <map>
#include <Error.hh>

// This class loads a file whose lines are of the form
//
//  key: value
//
// Blank lines and lines that begin with # are ignored.  Lines that
// end with \ are joined with the next line.  It is an error if the
// last line of the file ends iwth \.
//
// When constructing a KeyVal object, pass a set of strings
// representing mandatory keys, and a map of strings representing keys
// with default values.  The strings in "keys" and "defaults" must be
// disjoint.  The resulting object will contain a value for every
// specified key.

class KeyVal
{
  public:
    // Constructs an empty KeyVal.  The getter methods are not valid
    // until readFile() has been called.
    KeyVal(char const* filename,
	   std::set<std::string> const& keys,
	   std::map<std::string, std::string> const& defaults);

    // Reads the file.  Throws QEXC::System if the file can't be
    // opened.  Otherwise, returns true if there were no errors.
    bool readFile();

    // Get a list of all keys.
    std::set<std::string> getKeys() const;

    // Get a specific key.  It is an error to ask for a key that is
    // not defined.
    std::string const& getVal(std::string const& key) const;

    // Get a specific key.  If the key is not found, return the
    // default value specified.
    std::string const& getVal(std::string const& key,
			      std::string const& fallback_value) const;

  private:
    Error error;
    std::string filename;
    std::set<std::string> keys;
    std::map<std::string, std::string> defaults;
    std::map<std::string, std::string> data;
};

#endif // __KEYVAL_HH__
