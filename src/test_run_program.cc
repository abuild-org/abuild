#include <Util.hh>

#include <boost/bind.hpp>
#include <iostream>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

class OutputHandler
{
  public:
    void handler(bool is_error, char const* buf, int len);

  private:
    void flush(bool is_error, std::string& line);

    std::string output_line;
    std::string error_line;
};

void
OutputHandler::handler(bool is_error, char const* buf, int len)
{
    std::string& line = (is_error ? this->error_line : this->output_line);
    if (len == 0)
    {
	if ((! line.empty()) && (*(line.rbegin()) != '\n'))
	{
	    line += "[no newline]\n";
	}
	if (! line.empty())
	{
	    flush(is_error, line);
	}
    }
    else
    {
	for (int i = 0; i < len; ++i)
	{
	    line.append(1, buf[i]);
	    if (buf[i] == '\n')
	    {
		flush(is_error, line);
	    }
	}
    }
}

void
OutputHandler::flush(bool is_error, std::string& line)
{
    std::ostream& out = (is_error ? std::cerr : std::cout);
    char const* prefix = (is_error ? "E " : "O ");
    out << prefix << line;
    out.flush();
    line.clear();
}

static int run(int argc, char* argv[], char* envp[])
{
    if ((argc == 2) && (strcmp(argv[1], "-win32") == 0))
    {
	std::string cwd = Util::getCurrentDirectory();
	std::string batfile = cwd + "/hello.bat";
	std::vector<std::string> args;
	args.push_back("hello");
	args.push_back("MOO");
	std::map<std::string, std::string> env;
	env["VAR"] = "potato";
	bool status = Util::runProgram(batfile, args, env, 0, ".");
	std::cout << "status: " << status << std::endl << std::endl;
    }
    else if ((argc > 2) && (strcmp(argv[1], "-handle-output") == 0))
    {
	std::string progname;
	std::vector<std::string> args;
	std::map<std::string, std::string> env;
	assert(Util::getProgramFullPath(argv[2], progname));
	for (int i = 2; i < argc; ++i)
	{
	    args.push_back(argv[i]);
	}
	OutputHandler oh;
	bool status = Util::runProgram(
	    progname, args, env, envp, ".",
	    boost::bind(&OutputHandler::handler, &oh, _1, _2, _3));
	std::cout << "status: " << status << std::endl;
    }
    else if (argc > 2)
    {
	std::cout << "working directory: "
		  << Util::getCurrentDirectory() << std::endl;
	std::cout << "args:" << std::endl;
	for (char **argp = argv; *argp; ++argp)
	{
	    std::cout << "  " << *argp << std::endl;
	}
	std::cout << "env:" << std::endl;
	for (char** env = envp; *env; ++env)
	{
	    std::cout << "  " << *env << std::endl;
	}
	std::cout << "done" << std::endl;
	return atoi(argv[1]);
    }
    else
    {
	// There are some environment variables that we can't do
	// without when running the program.  Make sure we always have
	// them.
	std::vector<std::string> save_environment_vars;
	save_environment_vars.push_back("LD_LIBRARY_PATH");
	save_environment_vars.push_back("SYSTEMROOT");

	std::map<std::string, std::string> save_environment;
	for (std::vector<std::string>::iterator iter =
		 save_environment_vars.begin();
	     iter != save_environment_vars.end(); ++iter)
	{
	    std::string val;
	    if (Util::getEnv(*iter, &val))
	    {
		save_environment[*iter] = val;
	    }
	}

	std::string progname;
	assert(Util::getProgramFullPath(argv[0], progname));

	std::vector<std::string> args;
	args.push_back("run-program");
	args.push_back(""); 	// exit status
	args.push_back("");	// environment type
	args.push_back("one \"two three");
	args.push_back("four");

	std::map<std::string, std::string> env;
	env["VAR"] = "potato";
	env["MOO"] = "quack";
	env["OINK"] = "spackle";

	for (std::map<std::string, std::string>::iterator iter =
		 save_environment.begin();
	     iter != save_environment.end(); ++iter)
	{
	    env[(*iter).first] = (*iter).second;
	}

	bool status = false;

	args[1] = "3";
	args[2] = "env = new";
	status = Util::runProgram(progname, args, env, 0, ".");
	std::cout << "status: " << status << std::endl << std::endl;

	args[1] = "0";
	args[2] = "env = both";
	status = Util::runProgram(progname, args, env, envp, "..");
	std::cout << "status: " << status << std::endl << std::endl;

	args[1] = "0";
	args[2] = "env = none";
	env.clear();
	// Unfortunately, we don't really get to test this with no
	// environment at all....
	for (std::map<std::string, std::string>::iterator iter =
		 save_environment.begin();
	     iter != save_environment.end(); ++iter)
	{
	    env[(*iter).first] = (*iter).second;
	}
	status = Util::runProgram(progname, args, env, 0, "/");
	std::cout << "status: " << status << std::endl << std::endl;
    }

    return 0;
}

int main(int argc, char* argv[], char* envp[])
{
    try
    {
	return run(argc, argv, envp);
    }
    catch (std::exception& e)
    {
	std::cerr << "exception: " << e.what() << std::endl;
    }
    return 2;
}
