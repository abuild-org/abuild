// Help system

#include <Abuild.hh>
#include <QTC.hh>
#include <QEXC.hh>
#include <Util.hh>
#include <Logger.hh>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <assert.h>

bool
Abuild::generalHelp()
{
    // Return true if we have provided help.
    if (this->help_topic.empty())
    {
	return false;
    }

    if (this->help_topic == h_RULES)
    {
	if ((this->rules_help_topic == hr_HELP) ||
	    (this->rules_help_topic == hr_LIST) ||
	    (this->rules_help_topic.find(hr_RULE) == 0) ||
	    (this->rules_help_topic.find(hr_TOOLCHAIN) == 0))
	{
	    return false;
	}
    }

    boost::regex text_re("(.*)\\.txt");
    boost::regex description_re("## description: (.*?)\\s*");
    boost::smatch match;

    std::string helpdir = this->abuild_top + "/help";
    std::vector<std::string> entries = Util::getDirEntries(helpdir);
    std::map<std::string, std::string, Util::StringCaseLess> topics;
    for (std::vector<std::string>::iterator iter = entries.begin();
	 iter != entries.end(); ++iter)
    {
	if (boost::regex_match(*iter, match, text_re))
	{
	    std::string topic = match.str(1);
	    std::string filename = helpdir + "/" + *iter;
	    std::ifstream in(filename.c_str());
	    if (! in.is_open())
	    {
		throw QEXC::System("unable to open file " + filename, errno);
	    }
	    std::string firstline;
	    std::getline(in, firstline);
	    in.close();

	    std::string description;
	    if (boost::regex_match(firstline, match, description_re))
	    {
		description = match.str(1);
	    }

	    topics[topic] = description;
	}
    }
    if (topics.count(this->help_topic))
    {
	readHelpFile(helpdir + "/" + this->help_topic + ".txt");
	return true;
    }

    boost::function<void(std::string const&)> h =
	boost::bind(&Logger::logInfo, &(this->logger), _1, Logger::NO_JOB);

    if (this->help_topic != h_HELP)
    {
	h("");
	if (this->help_topic == h_RULES)
	{
	    QTC::TC("abuild", "Abuild-help ERR bad rules help topic");
	    h("Invalid rules help topic \"" + this->rules_help_topic + "\"");
	}
	else
	{
	    QTC::TC("abuild", "Abuild-help ERR bad help topic");
	    h("Invalid help topic \"" + this->help_topic + "\"");
	}
	// fall through to description of help system
    }

    // Describe the help system

    topics[h_HELP] = "the help system (this topic)";
    topics[h_RULES] = "rule-specific help (see below)";

    h("");
    h("To request help on a specific topic, run \"" +
      this->whoami + " --help topic\".");
    h("Help is available on the following topics:");
    h("");
    for (std::map<std::string, std::string, Util::StringCaseLess>::iterator
	     iter = topics.begin();
	 iter != topics.end(); ++iter)
    {
	std::string line = "  " + (*iter).first;
	if (! (*iter).second.empty())
	{
	    line += ": " + (*iter).second;
	}
	h(line);
    }
    showRulesHelpMessage();
    return true;
}

void
Abuild::showRulesHelpMessage()
{
    boost::function<void(std::string const&)> h =
	boost::bind(&Logger::logInfo, &(this->logger), _1, Logger::NO_JOB);

    h("");
    h("Help is available on built-in and user-supplied rules.  To request help");
    h("on rules, run \"" + this->whoami + " --help " + h_RULES + " topic" + "\".");
    h("The following rules topics are available:");
    h("");
    h("  help: show help specific to --help rules");
    h("  list: show all items for which rule help is available");
    h("  rule:rulename: show help on rule \"rulename\"");
    h("  toolchain:toolchainname: show help on toolchain \"toolchainname\"");
    h("");
}

void
Abuild::readHelpFile(std::string const& filename)
{
    std::list<std::string> lines = Util::readLinesFromFile(filename);
    for (std::list<std::string>::iterator iter = lines.begin();
	 iter != lines.end(); ++iter)
    {
	if ((*iter).find("#") == 0)
	{
	    continue;
	}
	this->logger.logInfo(*iter);
    }
}

void
Abuild::rulesHelp(BuildForest& forest)
{
    if (this->rules_help_topic == hr_HELP)
    {
	showRulesHelpMessage();
	return;
    }

    // Note that this function may be called on a build forest with
    // dependency/integrity errors.

    BuildItem_map& builditems = forest.getBuildItems();
    std::string const& this_name = this->this_config->getName();
    BuildItem_ptr this_builditem;
    if ((! this_name.empty()) && builditems.count(this_name))
    {
        this_builditem = builditems[this_name];
    }
    QTC::TC("abuild", "Abuild-help rules help with/without build item",
	    this_builditem.get() ? 0 : 1);

    // Create a table of available topics.

    static int const tt_toolchain = 0; // tt = topic type
    static int const tt_rule = 1;
    std::vector<HelpTopic_map> topics(2);

    appendToolchainHelpTopics(topics[tt_toolchain], "", "",
			      this->abuild_top + "/make");
    appendRuleHelpTopics(topics[tt_rule], "", "", this->abuild_top);

    for (BuildItem_map::iterator iter = builditems.begin();
	 iter != builditems.end(); ++iter)
    {
	BuildItem& item = *((*iter).second);
	std::string const& item_name = (*iter).first;
	std::string tree_name = item.getTreeName();
	std::string const& item_dir = item.getAbsolutePath();

	appendToolchainHelpTopics(
	    topics[tt_toolchain], item_name, tree_name, item_dir);
	appendRuleHelpTopics(
	    topics[tt_rule], item_name, tree_name, item_dir);
    }

    // Special case: remove data for built-in "_base" rules.
    topics[tt_rule].erase("_base");

    std::string toolchain;
    std::string rule;
    if (this->rules_help_topic.find(hr_TOOLCHAIN) == 0)
    {
	toolchain = this->rules_help_topic.substr(hr_TOOLCHAIN.length());
    }
    else if (this->rules_help_topic.find(hr_RULE) == 0)
    {
	rule = this->rules_help_topic.substr(hr_RULE.length());
    }

    boost::function<void(std::string const&)> h =
	boost::bind(&Logger::logInfo, &(this->logger), _1, Logger::NO_JOB);

    if (showHelpFiles(topics[tt_toolchain], "toolchain", toolchain) ||
	showHelpFiles(topics[tt_rule], "rule", rule))
    {
	return;
    }

    h("");
    h("Run \"" + this->whoami + " --help " + h_RULES + " " +
      hr_RULE + "rulename\" for help on a specific rule.");
    h("Run \"" + this->whoami + " --help " + h_RULES + " " +
      hr_TOOLCHAIN + "rulename\" for help on a specific toolchain.");
    h("");

    h("When an rule is shown to be available, it means that it is provided by");
    h("a build item that is in your dependency chain or is a plugin that is");
    h("active for your build item.  When a toolchain is shown to be");
    h("available, it means it is provided by a plugin that is active for your");
    h("build item.  Built-in toolchains and rules are always available.  In");
    h("order to make use of a rule or toolchain, it must also work for your");
    h("platform and be available for your item's target type.");

    std::set<std::string> references;
    std::set<std::string> plugins;
    if (this_builditem.get())
    {
	references = this_builditem->getReferences();
	std::list<std::string> const& plist = this_builditem->getPlugins();
	plugins.insert(plist.begin(), plist.end());
    }

    listHelpTopics(topics[tt_toolchain], "toolchains", plugins);
    listHelpTopics(topics[tt_rule], "rules", references);
}

void
Abuild::appendToolchainHelpTopics(HelpTopic_map& topics,
				  std::string const& item_name,
				  std::string const& tree_name,
				  std::string const& dir)
{
    std::list<std::string> dirs;
    appendToolchainPaths(dirs, dir);
    for (std::list<std::string>::iterator iter = dirs.begin();
	 iter != dirs.end(); ++iter)
    {
	appendHelpTopics(topics, item_name, tree_name,
			 TargetType::tt_object_code, *iter);
    }
}

void
Abuild::appendRuleHelpTopics(HelpTopic_map& topics,
			     std::string const& item_name,
			     std::string const& tree_name,
			     std::string const& dir)
{
    std::list<std::string> dirs;
    appendRulePaths(dirs, dir, TargetType::tt_all);
    for (std::list<std::string>::iterator iter = dirs.begin();
	 iter != dirs.end(); ++iter)
    {
	appendHelpTopics(topics, item_name, tree_name,
			 TargetType::getID(Util::basename(*iter)), *iter);
    }
}

void
Abuild::appendHelpTopics(HelpTopic_map& topics,
			 std::string const& item_name,
			 std::string const& tree_name,
			 TargetType::target_type_e target_type,
			 std::string const& dir)
{
    boost::regex module_re("(.*?)\\.(mk|groovy)");
    boost::regex helpfile_re("(.*?)-help\\.txt");
    boost::smatch match;

    std::set<std::string> modules;
    std::set<std::string> helpfiles;

    std::vector<std::string> entries = Util::getDirEntries(dir);
    for (std::vector<std::string>::iterator iter = entries.begin();
	 iter != entries.end(); ++iter)
    {
	std::string const& entry = *iter;
	std::string path = dir + "/" + entry;
	if (boost::regex_match(entry, match, module_re))
	{
	    // Don't worry if we see the same module more than once
	    // (both make and groovy).  In that case, the modules
	    // share a help file anyway.
	    modules.insert(match.str(1));
	}
	else if (boost::regex_match(entry, match, helpfile_re))
	{
	    helpfiles.insert(match.str(1));
	}
    }
    for (std::set<std::string>::iterator iter = modules.begin();
	 iter != modules.end(); ++iter)
    {
	std::string const& module = *iter;
	if (helpfiles.count(module))
	{
	    QTC::TC("abuild", "Abuild-help helpfile found");
	    helpfiles.erase(module);
	    topics[module].push_back(
		HelpTopic(
		    item_name, tree_name, target_type,
		    dir + "/" + module + "-help.txt"));
	}
	else
	{
	    QTC::TC("abuild", "Abuild-help module without helpfile found");
	    topics[module].push_back(
		HelpTopic(item_name, tree_name, target_type, ""));
	}
    }
    for (std::set<std::string>::iterator iter = helpfiles.begin();
	 iter != helpfiles.end(); ++iter)
    {
	QTC::TC("abuild", "Abuild-help module stray helpfile found");
	notice("WARNING: help file \"" +
	       dir + "/" + *iter + "-help.txt\""
	       " does not correspond to any implementation file");
    }
}

bool
Abuild::showHelpFiles(HelpTopic_map& topics,
		      std::string const& module_type,
		      std::string const& module_name)
{
    bool displayed = false;

    if (! module_name.empty())
    {
	// Set displayed even when we just given an "unknown" message.
	displayed = true;

	boost::function<void(std::string const&)> h =
	    boost::bind(&Logger::logInfo, &(this->logger), _1, Logger::NO_JOB);

	if (topics.count(module_name))
	{
	    h("");
	    h("The following help is available for " + module_type +
	      " \"" + module_name + "\":");
	    HelpTopic_vec& htv = topics[module_name];
	    for (HelpTopic_vec::iterator iter = htv.begin();
		 iter != htv.end(); ++iter)
	    {
		HelpTopic& ht = *iter;
		h("");
		if (ht.item_name.empty())
		{
		    h("built in " + module_type);
		}
		else
		{
		    h("provided by build item " + ht.item_name +
		      " (from tree " + ht.tree_name + ")");
		}
		h("applies to target type " +
		  TargetType::getName(ht.target_type));
		QTC::TC("abuild", "Abuild-help show item help file",
			ht.filename.empty() ? 1 : 0);
		if (ht.filename.empty())
		{
		    if (! ht.item_name.empty())
		    {
			QTC::TC("abuild", "Abuild-help no help for item");
		    }
		    h("No help file has been provided for this item.");
		}
		else
		{
		    h("Help text:");
		    h("----------");
		    readHelpFile(ht.filename);
		    h("----------");
		}
	    }
	}
	else
	{
	    QTC::TC("abuild", "Abuild-help help for unknown module");
	    h("");
	    h(module_type + " \"" + module_name + "\" is not known");
	    h("");
	    h("Run \"" + this->whoami + " --help " + h_RULES + " " + hr_LIST + "\" for a list of available rule topics.");
	    h("");
	}
    }

    return displayed;
}

void
Abuild::listHelpTopics(HelpTopic_map& topics, std::string const& description,
		       std::set<std::string>& references)
{
    boost::function<void(std::string const&)> h =
	boost::bind(&Logger::logInfo, &(this->logger), _1, Logger::NO_JOB);

    h("");
    h("The following " + description + " are available:");
    for (HelpTopic_map::iterator i1 = topics.begin();
	 i1 != topics.end(); ++i1)
    {
	std::string const& module = (*i1).first;
	h("");
	h("  " + module);
	HelpTopic_vec& htv = (*i1).second;
	for (HelpTopic_vec::iterator i2 = htv.begin();
	     i2 != htv.end(); ++i2)
	{
	    HelpTopic& ht = (*i2);
	    if (ht.item_name.empty())
	    {
		h("  * built in");
	    }
	    else
	    {
		h("  * provided by build item " + ht.item_name +
		  " (from tree " + ht.tree_name +
		  "); available: " +
		  (references.count(ht.item_name) ? "yes" : "no"));
	    }
	    h("    applies to target type " +
	      TargetType::getName(ht.target_type));
	    h(std::string("    help provided: ") +
	      (ht.filename.empty() ? "no" : "yes"));
	}
    }
}
