#include <Error.hh>

#include <sstream>
#include <Logger.hh>
#include <FileLocation.hh>
#include <QEXC.hh>

bool Error::any_errors = false;
boost::function<void (std::string const&)> Error::error_callback;

Error::Error(std::string const& default_prefix) :
    default_prefix(default_prefix),
    num_errors(0),
    logger(*(Logger::getInstance()))
{
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

std::string
Error::getText(FileLocation const& location, std::string const& msg)
{
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
	fullmsg << location;
    }
    fullmsg << msg;
    return fullmsg.str();
}

void
Error::error(FileLocation const& location, std::string const& msg)
{
    any_errors = true;
    ++this->num_errors;
    std::string const& text = getText(location, "ERROR: " + msg);
    this->logger.logError(text);
    if (error_callback)
    {
	error_callback(text);
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
