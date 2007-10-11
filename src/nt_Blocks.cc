#include <nt_Blocks.hh>
#include <nt_Block.hh>

nt_Blocks::nt_Blocks()
{
}

nt_Blocks::~nt_Blocks()
{
}

void
nt_Blocks::addBlock(nt_Block const* b)
{
    maybeSetLocation(b);
    this->blocks.push_back(b);
}

std::list<nt_Block const*> const&
nt_Blocks::getBlocks() const
{
    return this->blocks;
}

void
nt_Blocks::maybeSetLocation(nt_Block const* b)
{
    if (this->blocks.empty())
    {
	this->location = b->getLocation();
    }
}
