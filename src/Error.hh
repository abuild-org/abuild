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

    // Register a callback to be called with the any error message
    // text that is generated from the error function.
    static void setErrorCallback(boost::function<void (std::string const&)>);
    static void clearErrorCallback();

    int numErrors() const;
    static bool anyErrors();

  private:
    // Returns the string corresponding to a particular error message
    std::string getText(FileLocation const&, std::string const&);

    static bool any_errors;
    static boost::function<void (std::string const&)> error_callback;

    std::string default_prefix;
    int num_errors;
    Logger& logger;
};

#endif // __ERROR_HH__
