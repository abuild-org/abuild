#ifndef __UTIL_HH__
#define __UTIL_HH__

// This file declares several miscellaneous functions to provide
// simple but useful functionality.  It is intended to be the only
// file in the system that has to care about operating system
// differences, particularly between UNIX and Windows systems.

#include <list>
#include <string>
#include <vector>
#include <set>
#include <map>

namespace Util
{
    std::string intToString(int num);

    // Returns true iff stdout is a tty.
    bool stdoutIsTty();

    // Returns true iff the variable was defined.  If value is
    // non-NULL, it is initialized to the value of the variable, if
    // any.
    bool getEnv(std::string const& var, std::string* value = 0);

    // Normalize path separators in file name
    void normalizePathSeparators(std::string& filename);

    // Perform non-case-sensitive string comparison.  Returns -1 if s1
    // < s2, -1 if s1 > s2, or 0 otherwise.
    int strCaseCmp(std::string const& s1, std::string const& s2);

    // Strip a trailing slash from the end of a string
    void stripTrailingSlash(std::string& str);

    // A comparator class suitable for use as the comparator template
    // argument for map or set to get case-insensitive string
    // comparison
    class StringCaseLess
    {
      public:
	bool operator()(std::string const& a, std::string const& b) const
	{
	    return (Util::strCaseCmp(a, b) < 0);
	}
    };

    // Split a string into components using the given separator
    std::list<std::string> split(char sep, std::string input);

    // Split a string into whitespace-delimited sections
    std::list<std::string> splitBySpace(std::string const& input);

    // Join components into a string using the given separator and
    // iterators.
    template <typename Iterator>
    std::string join(std::string const& sep,
		     Iterator const& begin, Iterator end)
    {
	std::string result;
	bool first = true;
	for (Iterator iter = begin; iter != end; ++iter)
	{
	    if (first)
	    {
		first = false;
	    }
	    else
	    {
		result += sep;
	    }
	    result += *iter;
	}
	return result;
    }

    // Join components into a string using the given separator and
    // container.
    template <typename Container>
    std::string join(std::string const& sep, Container const& items)
    {
	return join(sep, items.begin(), items.end());
    }


    // Append to the list each element of the set that is not already
    // in the list.
    void appendNonMembers(std::list<std::string>& list,
			  std::set<std::string> const& set);

    // Return a list of lines from the given file.  All newline
    // terminators are removed unless otherwise specified.
    std::list<std::string> readLinesFromFile(
	std::string const& filename, bool strip_newlines = true);
    std::list<std::string> readLinesFromFile(
	std::istream& in, bool strip_newlines = true);

    std::string getCurrentDirectory();
    void setCurrentDirectory(std::string const&);

    bool isAbsolutePath(std::string const& path);

    // Return the canonical form of a path.  The canonical path is an
    // absolute path that traverses no symbolic links and has no
    // components equal to "." or "..".  path must be non-empty, but
    // it does not need to exist.
    std::string canonicalizePath(std::string path);

    // Indicate whether "dir" is under "topdir".  If topdir is empty,
    // the current directory is used.
    bool isDirUnder(std::string const& dir, std::string const& topdir = "");

    // Return the directory name of the given path.  Just trucates
    // everything after the last / or \ character, or if there are
    // none, returns ".".
    std::string dirname(std::string path);

    // Return the non-directory name of the given path.  Just returns
    // everything after the last / or \ character, or if there are
    // none, returns the original argument.
    std::string basename(std::string path);

    // In Windows, return the extension on a filename as a
    // three-character lower-case string if the string ends with
    // . followed by three characters or the empty string otherwise.
    // On UNIX, return the empty string.
    std::string getExtension(std::string const& path);

    // If in Windows, add .bat, .com, or .exe to the end of a string
    // if it doesn't already have it and if the result is a file that
    // exists.  Otherwise, do nothing.
    void appendExe(std::string& progname);

    // If in Windows, return a string equal to the original string
    // with any .exe, .bat, or .com suffix removed.  In UNIX, simply
    // return a copy of the original string.
    std::string removeExe(std::string const& progname);

    // Look in the path for an executable with the given name.  On
    // Windows, this automatically appends .exe internally if needed,
    // and it never returns results that end with .exe.  On UNIX, it
    // only checks executable files.  Returns each occurrence found in
    // path order.
    std::list<std::string> findProgramInPath(std::string progname);

    // Return our best guess at the canonical path of the given
    // program name, which is expected to come from argv[0].  If
    // progname is absolute, canonicalize.  Otherwise, if progname
    // contains any path separators, find it relative to the current
    // directory.  Otherwise, try to find it in the path.  This should
    // give the right answer for any program that was invoked from the
    // shell (UNIX/Windows command line or windows explorer) but will
    // not give the right answer if argv[0] was explicitly set to
    // something different from the actual path of the program as is
    // possible when calling variants of exec.  Returns false if we
    // try to find the program in the path and can't find any valid
    // candidates.  On Windows, any .exe suffix is stripped.
    bool getProgramFullPath(std::string progname, std::string& progpath);

    // Does file exist?
    bool fileExists(std::string const& filename);

    // Is file a plain file?
    bool isFile(std::string const& filename);

    // Is file a directory?
    bool isDirectory(std::string const& filename);

    // Is file a symbolic link?
    bool isSymlink(std::string const& filename);
    bool osSupportsSymlinks();

    // Returns a relative path for abs_path that reaches it from dir.
    // If dir is the empty string, return a relative path from the
    // current directory.  Both abs_path and dir must be canonical
    // paths with forward slash (/) as the path separator.  For
    // efficiency, this is not checked.  In Windows, if abs_path is on
    // a different drive from dir, abs_path is returned as is, which
    // means that it will not actually be a relative path.
    std::string absToRel(std::string const& abs_path,
			 std::string dir = "");

    // Run a command that is supposed to always succeed.  Throws
    // QEXC::General if the program does not exit normally.
    // Otherwise, returns the program output as a string with any
    // trailing carriage return and newline stripped.  Useful for
    // running programs like uname.
    std::string getProgramOutput(std::string cmd);

    // Run a program given by progname with the given args and
    // environment and with its current directory set to dir.  If
    // old_env is passed in, it is the existing environment, which
    // will be preserved.  If old_env is NULL, the existing
    // environment will not be preserved.  Returns true iff the
    // program exited normally.
    bool runProgram(std::string const& progname,
		    std::vector<std::string> const& args,
		    std::map<std::string, std::string> const& environment,
		    char* old_env[],
		    std::string const& dir);

    // Return the list of entries in a given directory.  This method
    // includes . and .. and does not prepend the path name.  It
    // throws an exception on any error condition.  The path must be a
    // directory.
    std::vector<std::string> getDirEntries(std::string const& path);

    // Recursively remove the given path, which may be a file or a
    // directory.  It is not an error if the file doesn't exist.
    // Throws an exception if any other type of error occurs.
    void removeFileRecursively(std::string const& path);

    // Create a directory with default permissions.  Throws an
    // exception on an error.  Intermediate directories must exist.
    // Does nothing if the directory already exists.
    void makeDirectory(std::string const& path);

    // Quote a string such that it is safe to include it in XML text.
    // If the "attribute" argument is true, also make it safe to
    // include in an XML attribute.
    std::string XMLify(std::string const& str, bool attribute);

    // Return a regular expression (as a string) that matches the same
    // strings matched by the given file glob.  Throws an exception if
    // the glob is invalid.
    std::string globToRegex(std::string const& glob);

    // Return the Java path separator appropriate for this platform
    std::string pathSeparator();
};

#endif // __UTIL_HH__
