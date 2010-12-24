// Miscellaneous methods: logging, usage, helper functions shared by
// multiple phases, etc.

#include <Abuild.hh>
#include <QTC.hh>
#include <QEXC.hh>
#include <Util.hh>
#include <Logger.hh>
#include <FileLocation.hh>
#include <BackingConfig.hh>
#include <cstdlib>
#include <cstdio>
#include <assert.h>

bool
Abuild::isBuildItemWritable(BuildItem const& item)
{
    if (item.getBackingDepth() > 0)
    {
	return false;
    }
    std::string path = item.getAbsolutePath();
    while (true)
    {
	if (this->ro_paths.count(path))
	{
	    return false;
	}
	if (this->rw_paths.count(path))
	{
	    return true;
	}
	std::string next = Util::dirname(path);
	if (path == next)
	{
	    return this->default_writable;
	}
	path = next;
    }

    // can't get here
    assert(false);
    return false;
}

void
Abuild::exitIfErrors()
{
    if (Error::anyErrors())
    {
	fatal("errors detected; exiting");
    }
}

void
Abuild::info(std::string const& msg, Logger::job_handle_t job)
{
    if (! this->silent)
    {
	this->logger.logInfo(this->whoami + ": " + msg, job);
    }
}

void
Abuild::notice(std::string const& msg, Logger::job_handle_t job)
{
    this->logger.logInfo(this->whoami + ": " + msg, job);
}

void
Abuild::incrementVerboseIndent()
{
    this->verbose_indent += " ";
}

void
Abuild::decrementVerboseIndent()
{
    this->verbose_indent.erase(this->verbose_indent.length() - 1);
}

void
Abuild::verbose(std::string const& msg, Logger::job_handle_t job)
{
    if (this->verbose_mode)
    {
	this->logger.logInfo(this->whoami + ": (verbose) " +
			     this->verbose_indent + msg, job);
    }
}

void
Abuild::monitorOutput(std::string const& msg)
{
    if (this->monitored)
    {
	this->logger.logInfo("abuild-monitor: " + msg);
    }
}

void
Abuild::monitorErrorCallback(std::string const& msg)
{
    monitorOutput("error " + msg);
}

void
Abuild::error(std::string const& msg, Logger::job_handle_t job)
{
    error(FileLocation(), msg, job);
}

void
Abuild::error(FileLocation const& location, std::string const& msg,
	      Logger::job_handle_t job)
{
    this->error_handler.error(location, msg, job);
}

void
Abuild::deprecate(std::string const& version, std::string const& msg,
		  Logger::job_handle_t job)
{
    deprecate(version, FileLocation(), msg, job);
}

void
Abuild::deprecate(std::string const& version,
		  FileLocation const& location, std::string const& msg,
		  Logger::job_handle_t job)
{
    this->error_handler.deprecate(version, location, msg, job);
}

void
Abuild::suggestUpgrade()
{
    assert(this->compat_level.allow_1_0());
    if (this->suggest_upgrade)
    {
	this->logger.logInfo("");
	this->logger.logInfo("******************** " + this->whoami +
			     " ********************");
	this->logger.logInfo("WARNING: Build items/trees with"
			     " deprecated 1.0 features were found.");
	this->logger.logInfo("Consider upgrading your build trees,"
			     " which you can do automatically by");
	this->logger.logInfo("running");
	this->logger.logInfo("");
	this->logger.logInfo("  " + this->whoami + " --upgrade-trees");
	this->logger.logInfo("");
	this->logger.logInfo("from the appropriate location.");
	this->logger.logInfo("");
	this->logger.logInfo("For details, please see \"Upgrading Build"
			     " Trees from 1.0 to 1.1\" in");
	this->logger.logInfo("the user's manual");
	this->logger.logInfo("******************** " + this->whoami +
			     " ********************");
	this->logger.logInfo("");
    }
    else if (! this->deprecated_backing_files.empty())
    {
	// Only complain specifically about deprecated backing files
	// if there aren't other upgrade suggestions.
	QTC::TC("abuild", "Abuild backing deprecation warning");
	for (std::set<std::string>::iterator iter =
		 this->deprecated_backing_files.begin();
	     iter != this->deprecated_backing_files.end(); ++iter)
	{
	    deprecate("1.1", FileLocation(*iter, 0, 0),
		      "this is a 1.0-style " + BackingConfig::FILE_BACKING +
		      " file in an otherwise updated area");
	}
    }
}

void
Abuild::fatal(std::string const& msg)
{
    // General exception caught in main().
    throw QEXC::General(this->whoami + ": ERROR: " + msg);
}

void
Abuild::usage(std::string const& msg)
{
    error(msg);
    fatal("run \"" + this->whoami + " --help\" or \"" +
	  this->whoami + " --help usage\" for help");
}
