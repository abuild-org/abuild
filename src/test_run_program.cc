#include <Util.hh>

#include <iostream>
#include <assert.h>

int main(int argc, char* argv[], char* envp[])
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

	// In some cases, LD_LIBRARY_PATH may be required to run this
	// program.  If LD_LIBRARY_PATH is present in the old
	// environment, we need to copy it into the new environment.
	std::string ld_library_path;
	if (Util::getEnv("LD_LIBRARY_PATH", &ld_library_path))
	{
	    env["LD_LIBRARY_PATH"] = ld_library_path;
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
	if (! ld_library_path.empty())
	{
	    // Unfortunately, we don't really get to test this with no
	    // environment at all....
	    env["LD_LIBRARY_PATH"] = ld_library_path;
	}
	status = Util::runProgram(progname, args, env, 0, "/");
	std::cout << "status: " << status << std::endl << std::endl;
    }

    return 0;
}
