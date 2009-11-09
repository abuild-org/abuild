#include <nt_Word.hh>
#include <Token.hh>

nt_Word::nt_Word()
{
}

nt_Word::~nt_Word()
{
}

void
nt_Word::appendWord(nt_Word const* w)
{
    maybeSetLocation(w->tokens.front().first);
    this->tokens.insert(this->tokens.end(),
			w->tokens.begin(), w->tokens.end());
}

void
nt_Word::appendString(Token const* t)
{
    maybeSetLocation(t);
    this->tokens.push_back(std::make_pair(t, w_string));
}

void
nt_Word::appendVariable(Token const* t)
{
    maybeSetLocation(t);
    this->tokens.push_back(std::make_pair(t, w_variable));
}

void
nt_Word::appendEnvironment(Token const* t)
{
    maybeSetLocation(t);
    this->tokens.push_back(std::make_pair(t, w_environment));
}

void
nt_Word::maybeSetLocation(Token const* t)
{
    if (this->tokens.empty())
    {
	this->location = t->getLocation();
    }
}

std::list<nt_Word::word_t> const&
nt_Word::getTokens() const
{
    return this->tokens;
}
