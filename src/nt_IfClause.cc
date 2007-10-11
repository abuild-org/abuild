#include <nt_IfClause.hh>

#include <nt_Conditional.hh>
#include <nt_Blocks.hh>

// conditional may be 0 even for an if or elseif.
nt_IfClause::nt_IfClause(nt_Conditional const* conditional,
			 nt_Blocks const* blocks,
			 bool conditional_expected) :
    NonTerminal(conditional
		? conditional->getLocation()
		: blocks->getLocation()),
    conditional_okay(conditional_expected == (conditional != 0)),
    conditional(conditional),
    blocks(blocks)
{
}

nt_IfClause::~nt_IfClause()
{
}

bool
nt_IfClause::getConditionalOkay() const
{
    return this->conditional_okay;
}

nt_Conditional const*
nt_IfClause::getConditional() const
{
    return this->conditional;
}

nt_Blocks const*
nt_IfClause::getBlocks() const
{
    return this->blocks;
}
