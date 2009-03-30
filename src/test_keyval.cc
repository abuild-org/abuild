#include <KeyVal.hh>

#include <Logger.hh>
#include <QEXC.hh>
#include <Util.hh>

Logger* logger = 0;

static void
dump_keyval(KeyVal const& kv)
{
    std::set<std::string> const& keys = kv.getKeys();
    for (std::set<std::string>::const_iterator iter = keys.begin();
	 iter != keys.end(); ++iter)
    {
	logger->logInfo(*iter + ": " + kv.getVal(*iter));
    }
    // Exercise fallback code
    if (kv.getVal("does-not-exist", "fallback") != "fallback")
    {
	throw QEXC::General("fallback didn't work");
    }
}

int main(int argc, char* argv[])
{
    logger = Logger::getInstance();

    char* filename = argv[1];

    std::set<std::string> keys;
    std::map<std::string, std::string> defaults;
    keys.insert("qoww");
    keys.insert("chicken");
    keys.insert("duck");
    keys.insert("pig");
    defaults["potato"] = "salad";
    std::map<std::string, std::string> substitutions;
    substitutions["chicken"] = "adventurous";
    std::map<std::string, std::string> replacements;
    replacements["pig"] = "small: whatever smalls say"; // opposite of pig
    std::set<std::string> omissions;
    std::vector<std::pair<std::string, std::string> > additions;

    if (std::string(filename) == "test9.in")
    {
	keys.insert("sheep");
	omissions.insert("sheep");
	additions.push_back(
	    std::make_pair(std::string("h-monster"),
			   std::string("grunt and rub face on carpet")));
    }

    try
    {
	logger->logInfo("no defaults");
	KeyVal kv1(filename, keys, std::map<std::string, std::string>());
	if (kv1.readFile())
	{
	    dump_keyval(kv1);
	}

	logger->logInfo("defaults");
	KeyVal kv2(filename, keys, defaults);
	if (kv2.readFile())
	{
	    dump_keyval(kv2);
	}

	std::string ek1 = Util::join(" ", kv1.getExplicitKeys());
	std::string ek2 = Util::join(" ", kv2.getExplicitKeys());
	if (ek1 == ek2)
	{
	    logger->logInfo("explicit keys: " + ek1);
	}
	else
	{
	    logger->logError("explicit keys: no defaults: " + ek1 +
			     "; defaults: " + ek2);
	}

	std::string newfile = std::string(filename) + ".wr1";
	kv2.writeFile(newfile.c_str(),
		      std::map<std::string, std::string>(),
		      std::map<std::string, std::string>(),
		      std::set<std::string>(),
		      std::vector<std::pair<std::string, std::string> >());
	newfile = std::string(filename) + ".wr2";
	kv2.writeFile(newfile.c_str(), substitutions,
		      replacements, omissions, additions);
    }
    catch (std::exception& e)
    {
	std::string msg = e.what();
	// Clean up error message for test suite
	size_t p = msg.rfind(':');
	if (p != std::string::npos)
	{
	    msg = msg.substr(0, p);
	    msg += ": <system message>";
	}
	logger->logError(msg);
    }

    Logger::stopLogger();

    return 0;
}
