#include "ExternalData.hh"
#include <Util.hh>
#include <QTC.hh>
#include <assert.h>

ExternalData::ExternalData() :
    read_only(true)
{
    // empty constructor std::map
}

ExternalData::ExternalData(std::string const& declared_path,
			   bool read_only) :
    declared_path(declared_path),
    read_only(read_only)
{
}

std::string const&
ExternalData::getDeclaredPath() const
{
    return this->declared_path;
}

std::string const&
ExternalData::getAbsolutePath() const
{
    // The absolute path is computed later; make sure no one accesses
    // it before it is computed.
    assert(! this->absolute_path.empty());
    return this->absolute_path;
}

bool
ExternalData::isReadOnly() const
{
    return this->read_only;
}

void
ExternalData::setAbsolutePath(std::string const& absolute_path)
{
    this->absolute_path = absolute_path;
}