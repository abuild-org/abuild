#ifndef __NT_BLOCKS_HH__
#define __NT_BLOCKS_HH__

#include <NonTerminal.hh>
#include <list>

class nt_Block;

class nt_Blocks: public NonTerminal
{
  public:
    nt_Blocks();
    virtual ~nt_Blocks();
    void addBlock(nt_Block const*);

    std::list<nt_Block const*> const& getBlocks() const;

  private:
    void maybeSetLocation(nt_Block const*);

    std::list<nt_Block const*> blocks;
};

#endif // __NT_BLOCKS_HH__
