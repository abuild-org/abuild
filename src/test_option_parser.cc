#include <OptionParser.hh>

#include <Util.hh>
#include <boost/bind.hpp>
#include <exception>
#include <string>
#include <iostream>
#include <cstdlib>

class OPTest
{
  public:
    OPTest(char** argv);
    void run();
    std::string const& getWhoami() const;

  private:
    void usageError(std::string const& msg);
    void doPositional(std::string const& arg);
    void doSomething();
    void doFlag1(int);
    void doNumber(int);
    void doPotatoName(std::string const&);
    void doPotatoType(boost::smatch const&);
    void doHelp(std::vector<std::string> const&);
    void doWords(std::vector<std::string> const&);

    std::string whoami;
    char** args;
    OptionParser op;
};

OPTest::OPTest(char** argv) :
    whoami(Util::removeExe(Util::basename(argv[0]))),
    args(argv + 1),
    op(boost::bind(&OPTest::usageError, this, _1),
       boost::bind(&OPTest::doPositional, this, _1))
{
    size_t pos = whoami.find_last_of('/');
    if (pos != std::string::npos)
    {
	whoami = whoami.substr(pos + 1);
    }

    op.registerNoArg(
	"do-something",
	boost::bind(&OPTest::doSomething, this));
    op.registerOptionalNumericArg(
	"flag1", 12,
	boost::bind(&OPTest::doFlag1, this, _1));
    op.registerSynonym("f", "flag1");
    op.registerNumericArg(
	"number",
	boost::bind(&OPTest::doNumber, this, _1));
    op.registerStringArg(
	"potato-name",
	boost::bind(&OPTest::doPotatoName, this, _1));
    op.registerRegexArg(
	"potato-type", "(?:(half|quarter)-)?((?i)baked|mashed|roasted)",
	boost::bind(&OPTest::doPotatoType, this, _1));
    op.registerListArg(
	"help", "-?-end-help", true,
	boost::bind(&OPTest::doHelp, this, _1));
    op.registerListArg(
	"words", "-.*", false,
	boost::bind(&OPTest::doWords, this, _1));
}

std::string const&
OPTest::getWhoami() const
{
    return this->whoami;
}

void
OPTest::run()
{
    this->op.parseOptions(this->args);
}

void
OPTest::usageError(std::string const& msg)
{
    std::cerr << this->whoami << ": ERROR: " << msg << std::endl;
}

void
OPTest::doPositional(std::string const& arg)
{
    std::cout << "positional option: " << arg << std::endl;
}

void
OPTest::doSomething()
{
    std::cout << "doing something" << std::endl;
}

void
OPTest::doFlag1(int val)
{
    std::cout << "flag 1, argument = " << val << std::endl;
}

void
OPTest::doNumber(int val)
{
    std::cout << "number, argument = " << val << std::endl;
}

void
OPTest::doPotatoName(std::string const& val)
{
    std::cout << "potato name: " << val << std::endl;
}

void
OPTest::doPotatoType(boost::smatch const& m)
{
    std::string degree = "fully";
    if (m[1].matched)
    {
	degree = m[1].str();
    }
    std::cout << "potato type: " << degree << " " << m[2].str() << std::endl;
}

void
OPTest::doHelp(std::vector<std::string> const& args)
{
    std::cout << "help args:" << std::endl;
    for (std::vector<std::string>::const_iterator iter = args.begin();
	 iter != args.end(); ++iter)
    {
	std::cout << "  \"" << *iter << "\"" << std::endl;
    }
    std::cout << "end of help args" << std::endl;
}

void
OPTest::doWords(std::vector<std::string> const& args)
{
    std::cout << "words:" << std::endl;
    for (std::vector<std::string>::const_iterator iter = args.begin();
	 iter != args.end(); ++iter)
    {
	std::cout << "  \"" << *iter << "\"" << std::endl;
    }
    std::cout << "end of words" << std::endl;
}

int main(int argc, char* argv[])
{
    OPTest ot(argv);
    try
    {
	ot.run();
    }
    catch (std::exception& e)
    {
	std::cerr << ot.getWhoami() << ": exception: " << e.what() << std::endl;
	std::exit(2);
    }
    return 0;
}
