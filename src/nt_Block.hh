#ifndef __NT_BLOCK_HH__
#define __NT_BLOCK_HH__

#include <NonTerminal.hh>

class nt_IfBlock;
class nt_Reset;
class nt_Assignment;
class nt_Declaration;
class nt_AfterBuild;
class nt_TargetType;

class nt_Block: public NonTerminal
{
  public:
    enum block_e
    {
	b_ifblock,
	b_reset,
	b_assignment,
	b_declaration,
	b_after_build,
	b_targettype
    };

    nt_Block(nt_IfBlock const*);
    nt_Block(nt_Reset const*);
    nt_Block(nt_Assignment const*);
    nt_Block(nt_Declaration const*);
    nt_Block(nt_AfterBuild const*);
    nt_Block(nt_TargetType const*);
    virtual ~nt_Block();

    block_e getBlockType() const;

    nt_IfBlock const* getIfBlock() const;
    nt_Reset const* getReset() const;
    nt_Assignment const* getAssignment() const;
    nt_Declaration const* getDeclaration() const;
    nt_AfterBuild const* getAfterBuild() const;
    nt_TargetType const* getTargetType() const;

  private:
    block_e block_type;

    // Only one of these will be defined
    nt_IfBlock const* ifblock;
    nt_Reset const* reset;
    nt_Assignment const* assignment;
    nt_Declaration const* declaration;
    nt_AfterBuild const* after_build;
    nt_TargetType const* targettype;
};

#endif // __NT_BLOCK_HH__
