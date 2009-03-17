#ifndef __ERROR_HH__
#define __ERROR_HH__

#include <string>
#include <boost/function.hpp>

class FileLocation;
class Logger;

class Error
{
  public:
    Error(std::string const& default_prefix = "");

    // Writes an error message using the logger
    void error(FileLocation const&, std::string const&);

    // Writes a deprecation error or warning
    void deprecate(std::string const& version,
		   FileLocation const&, std::string const&);

    // Set globally whether or not deprecation messages are considered
    // errors.  By default, they are warnings.
    static void setDeprecationIsError(bool);

    // Register a callback to be called with the any error message
    // text that is generated from the error function.
    static void setErrorCallback(boost::function<void (std::string const&)>);
    static void clearErrorCallback();

    int numErrors() const;
    static bool anyErrors();

  private:
    void logText(FileLocation const&, std::string const&);

    static bool any_errors;
    static bool deprecate_is_error;
    static boost::function<void (std::string const&)> error_callback;

    std::string default_prefix;
    int num_errors;
    Logger& logger;
};

#endif // __ERROR_HH__
