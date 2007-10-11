#ifndef __NT_AUTOFILE_HH__
#define __NT_AUTOFILE_HH__

#include <NonTerminal.hh>
#include <FileLocation.hh>
#include <string>

class nt_Words;

class nt_AfterBuild: public NonTerminal
{
  public:
    nt_AfterBuild(nt_Words const*);
    virtual ~nt_AfterBuild();

    nt_Words const* getArgument() const;

  private:
    nt_Words const* argument;
};

#endif // __NT_AUTOFILE_HH__
