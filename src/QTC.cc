#include <QTC.hh>

#include <boost/thread/mutex.hpp>
#include <string>
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <Util.hh>
#ifdef _WIN32
#include <windows.h>
#endif

// With some compilers (gcc 4.1.0 on Fedora Core 5), declaring these
// statics inside QTC::TC causes problems when they are accessed by
// multiple threads, but declaring then static at the file level works
// fine.
static std::set<std::pair<std::string, int> > cache;
static boost::mutex qtc_mutex;

static bool tc_active(char const* const scope)
{
    std::string value;
    return (Util::getEnv("TC_SCOPE", &value) && (value == scope));
}

void QTC::TC(char const* const scope, char const* const ccase, int n)
{
    if (! tc_active(scope))
    {
	return;
    }

    std::string filename;
#ifdef _WIN32
# define TC_ENV "TC_WIN_FILENAME"
#else
# define TC_ENV "TC_FILENAME"
#endif
    if (! Util::getEnv(TC_ENV, &filename))
    {
	return;
    }
#undef TC_ENV

    boost::mutex::scoped_lock lock(qtc_mutex);
    if (cache.count(std::make_pair(ccase, n)))
    {
	return;
    }
    cache.insert(std::make_pair(ccase, n));

    FILE* tc = fopen(filename.c_str(), "ab");
    if (tc)
    {
	fprintf(tc, "%s %d\n", ccase, n);
	fclose(tc);
    }
}
