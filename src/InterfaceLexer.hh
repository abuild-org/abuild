#ifndef __INTERFACELEXER_HH__
#define __INTERFACELEXER_HH__

#ifdef _WIN32
# define YY_NO_UNISTD_H
#endif

#ifndef yyFlexLexerOnce
# ifdef yyFlexLexer
#  undef yyFlexLexer
# endif
# define yyFlexLexer yy_interfaceFlexLexer
# include <FlexLexer.h>
# undef yyFlexLexerOnce
#endif

class InterfaceParser;
class Token;

// --- workaround for bug in Fedora 7 flex

// To remove workaround, remove this block of code, add %option
// noyywrap back to interface.qfl, and remove InterfaceLexer.cc (which
// contains nothing but the implementation of yy_interfacewrap).

#ifndef yywrap
#define yywrap yy_interfacewrap
#endif

extern "C" int yy_interfacewrap();

// --- end workaround

class InterfaceLexer: public yy_interfaceFlexLexer
{
  public:
    InterfaceLexer(InterfaceParser* p) :
	parser(p)
    {
    }
    virtual ~InterfaceLexer()
    {
    }
    virtual int yylex();

  private:
    InterfaceParser* parser;
};

#endif // __INTERFACELEXER_HH__
