#include <nt_Words.hh>

nt_Words::nt_Words(FileLocation const& location) :
    NonTerminal(location)
{
}

nt_Words::~nt_Words()
{
}

void
nt_Words::append(nt_Word const* w)
{
    this->words.push_back(w);
}

std::list<nt_Word const*> const&
nt_Words::getWords() const
{
    return this->words;
}
