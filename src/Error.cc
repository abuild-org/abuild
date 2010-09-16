
#include <Error.hh>

#include <sstream>
#include <FileLocation.hh>
#include <QEXC.hh>

bool Error::any_errors = false;
bool Error::deprecate_is_error = false;
boost::function<void (std::string const&)> Error::error_callback;

Error::Error(Logger::job_handle_t default_logger_job,
	     std::string const& default_prefix) :
    default_prefix(default_prefix),
    default_logger_job(default_logger_job),
    num_errors(0),
    logger(*(Logger::getInstance()))
{
}

void
Error::setDeprecationIsError(bool val)
{
    deprecate_is_error = val;
}

void
Error::setErrorCallback(boost::function<void (std::string const&)> cb)
{
    error_callback = cb;
}

void
Error::clearErrorCallback()
{
    error_callback = boost::function<void (std::string const&)>();
}

void
Error::logText(FileLocation const& location, std::string const& msg,
	       Logger::job_handle_t job)
{
    if (job == Logger::NO_JOB)
    {
	job = this->default_logger_job;
    }
    std::ostringstream fullmsg;
    if (location == FileLocation())
    {
	if (! this->default_prefix.empty())
	{
	    fullmsg << this->default_prefix << ": ";
	}
    }
    else
    {
	fullmsg << location << ": ";
    }
    fullmsg << msg;
    this->logger.logError(fullmsg.str(), job);
    if (error_callback)
    {
	error_callback(fullmsg.str());
    }
}

void
Error::error(FileLocation const& location, std::string const& msg,
	     Logger::job_handle_t job)
{
    any_errors = true;
    ++this->num_errors;
    logText(location, "ERROR: " + msg, job);
}

void
Error::deprecate(std::string const& version,
		 FileLocation const& location, std::string const& orig_message,
		 Logger::job_handle_t job)
{
    std::string message = "*** DEPRECATION WARNING *** (abuild version " +
	version + "): " + orig_message;
    if (deprecate_is_error)
    {
	error(location, message, job);
    }
    else
    {
	logText(location, message, job);
    }
}

int
Error::numErrors() const
{
    return this->num_errors;
}

bool
Error::anyErrors()
{
    return any_errors;
}
