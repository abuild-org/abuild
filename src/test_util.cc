#include <Util.hh>
#include <set>
#include <sstream>
#include <iostream>

// This program tests those parts of Util that are not involved in
// canonicalizePath, which are tested in test_canonicalize_path.

static void test_str(std::string const& s1, std::string const& s2)
{
    std::cout << s1 << " cmp " << s2 << " = "
	      << Util::strCaseCmp(s1, s2) << std::endl;
}


static void test_glob_to_regex(char const* glob)
{
    try
    {
	std::string regex = Util::globToRegex(glob);
	std::cout << glob << " -> " << regex << std::endl;
    }
    catch (std::exception& e)
    {
	std::cout << "glob " << glob << " threw exception: " << e.what()
		  << std::endl;
    }
}

static void test_strip_trailing_slash(std::string const& str)
{
    std::string buf = str;
    Util::stripTrailingSlash(buf);
    std::cout << "\"" << str << "\" -> \"" << buf << "\"" << std::endl;
}

int main()
{
    std::cout << Util::intToString(5) << " "
	      << Util::intToString(-12) << std::endl;

    std::cout << Util::dirname("/") << std::endl
	      << Util::dirname("a") << std::endl
	      << Util::dirname("a/b/c") << std::endl
	      << Util::dirname("a\\b\\c") << std::endl
	      << Util::dirname("\\a") << std::endl;
#ifdef _WIN32
    std::cout << Util::dirname("C:\\") << std::endl;
#else
    std::cout << "C:/" << std::endl;
#endif

    //
    // NOTE: absToRel tests are duplicated in abuild-groovy-basic test
    // suite for the groovy version of this function.
    //
    std::cout << Util::absToRel(
	"/one/two/three",
	"/one/two/four/five") << std::endl;
    std::cout << Util::absToRel(
	"/one/two/four/five",
	"/one/two/three") << std::endl;
    std::cout << Util::absToRel(
	"/one/two/three",
	"/one/two/three/four") << std::endl;
    std::cout << Util::absToRel(
	"/one/two/three/four",
	"/one/two/three") << std::endl;
    std::cout << Util::absToRel(
	"/one/two/three",
	"/one/two/three") << std::endl;

#ifdef _WIN32
    // case-insensitive and multiple drive letter tests
    std::cout << Util::absToRel(
	"/ONE/two/THREE",
	"/one/TWO/Three") << std::endl;
    std::cout << Util::absToRel(
	"C:/A/B/C/D/E/F",
	"c:/a/b/c/q/r/s") << std::endl;
    std::cout << Util::absToRel(
	"Q:/w/w/w",
	"R:/x/x/x") << std::endl;
#else
    std::cout << "." << std::endl;
    std::cout << "../../../D/E/F" << std::endl;
    std::cout << "Q:/w/w/w" << std::endl;
#endif

    std::list<std::string> split = Util::splitBySpace(
	"  \t one two\tthree  \t  four  five  ");
    for (std::list<std::string>::iterator iter = split.begin();
	 iter != split.end(); ++iter)
    {
	std::cout << "-" << *iter << "-" << std::endl;
    }
    split = Util::splitBySpace(" ");
    std::cout << "split " << (split.empty() ? "empty" : "not empty")
	      << std::endl;

    std::cout << Util::getProgramOutput("echo test getProgramOutput")
	      << std::endl;

    // Here's a silly test of readLinesFromFile using a string
    std::string str("one\r\ntwo\nthree");
    std::istringstream in(str);
    std::list<std::string> lines = Util::readLinesFromFile(in);
    for (std::list<std::string>::iterator iter = lines.begin();
	 iter != lines.end(); ++iter)
    {
	std::cout << *iter << std::endl;
    }

    // XMLify
    std::cout << Util::XMLify("one'two\"three&<four>\nfive!", false)
	      << std::endl
	      << Util::XMLify("one'two\"three&<four>\r\nfive!", true)
	      << std::endl;

    // case-insensitive string comparison
    test_str("Asdf", "asdF");
    test_str("qwer", "QWERTY");
    test_str("123x", "123X");
    test_str("ORANGE", "apple");
    test_str("APPLE", "orange");

    std::set<std::string, Util::StringCaseLess> strings;
    strings.insert("apple");
    strings.insert("ORANGE");
    for (std::set<std::string, Util::StringCaseLess>::iterator iter =
	     strings.begin();
	 iter != strings.end(); ++iter)
    {
	std::cout << *iter << std::endl;
    }
    if (strings.count("orange"))
    {
	std::cout << "orange found" << std::endl;
    }

    // glob to regex
    test_glob_to_regex("*");
    test_glob_to_regex("*.*");
    test_glob_to_regex("a.b.c");
    test_glob_to_regex("a.{b,c}.d");
    test_glob_to_regex("a[bcdef]g");
    test_glob_to_regex("a.{b,[bcdef],g}h?j*");
    test_glob_to_regex("a^b[^c]^d");
    test_glob_to_regex("a[\\[\\]],{()+$\\{,\\}}\\\\");
    test_glob_to_regex("a{,b}{,,b}c{d,}{d,,}e{f,,g}{f,,,g}h{,,i,,,j,,}k");

    test_glob_to_regex("");
    test_glob_to_regex("a\\");
    test_glob_to_regex("a[");
    test_glob_to_regex("a{");
    test_glob_to_regex("a{{");

    // trailing slash
    test_strip_trailing_slash("");
    test_strip_trailing_slash("a/b");
    test_strip_trailing_slash("a/b/");

    return 0;
}
