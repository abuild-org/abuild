#ifndef __NT_AUTOFILE_HH__
#define __NT_AUTOFILE_HH__

#include <NonTerminal.hh>
#include <FileLocation.hh>
#include <string>

class nt_Word;

class nt_AfterBuild: public NonTerminal
{
  public:
    nt_AfterBuild(nt_Word const*);
    virtual ~nt_AfterBuild();

    nt_Word const* getArgument() const;

  private:
    nt_Word const* argument;
};

#endif // __NT_AUTOFILE_HH__
