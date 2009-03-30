#ifndef __KEYVAL_HH__
#define __KEYVAL_HH__

#include <string>
#include <set>
#include <map>
#include <vector>
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

    std::string getPreferredEOL() const;

    // Rewrite the data to newfile.  For each element of key_changes,
    // replace any occurrences of map keys with the associated value.
    // For each element of replacements, replace the key and value
    // together with the value, appending whatever end of line
    // character is on the original.  For keys in deletions, omit
    // entirely.  For keys in additions, add the key and value,
    // terminating with whatever line terminator was used on the first
    // line of the original file.
    void writeFile(
	char const* newfile,
	std::map<std::string, std::string> const& key_changes,
	std::map<std::string, std::string> const& replacements,
	std::set<std::string> const& deletions,
	std::vector<std::pair<std::string, std::string> > const& additions)
	const;

    // Get a list of all keys.
    std::set<std::string> getKeys() const;

    // Get a list of all keys that appeared explicitly.
    std::set<std::string> getExplicitKeys() const;

    // Get a specific key.  It is an error to ask for a key that is
    // not defined.
    std::string const& getVal(std::string const& key) const;

    // Get a specific key.  If the key is not found, return the
    // default value specified.
    std::string const& getVal(std::string const& key,
			      std::string const& fallback_value) const;

  private:
    class OrigData
    {
      public:
	// a key/value entry, possibly spanning multiple lines and
	// included embedded comments
	std::string before;
	std::string key;
	std::string after;
    };

    Error error;
    std::string filename;
    std::string preferred_eol;
    std::set<std::string> keys;
    std::set<std::string> explicit_keys;
    std::map<std::string, std::string> defaults;
    std::map<std::string, std::string> data;
    std::vector<OrigData> orig_data;
};

#endif // __KEYVAL_HH__
