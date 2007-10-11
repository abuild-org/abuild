#ifndef __NT_WORDS_HH__
#define __NT_WORDS_HH__

#include <NonTerminal.hh>
#include <list>

class nt_Word;

class nt_Words: public NonTerminal
{
  public:
    nt_Words(FileLocation const&);
    virtual ~nt_Words();
    void append(nt_Word const* w);

    std::list<nt_Word const*> const& getWords() const;

  private:
    std::list<nt_Word const*> words;
};

#endif // __NT_WORDS_HH__
