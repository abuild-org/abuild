#include <nt_Block.hh>

#include <assert.h>
#include <nt_IfBlock.hh>
#include <nt_Reset.hh>
#include <nt_Assignment.hh>
#include <nt_Declaration.hh>
#include <nt_AfterBuild.hh>
#include <nt_TargetType.hh>

nt_Block::nt_Block(nt_IfBlock const* ifblock) :
    NonTerminal(ifblock->getLocation()),
    block_type(b_ifblock),
    ifblock(ifblock),
    reset(0),
    assignment(0),
    declaration(0),
    after_build(0),
    targettype(0)
{
}

nt_Block::nt_Block(nt_Reset const* reset) :
    NonTerminal(reset->getLocation()),
    block_type(b_reset),
    ifblock(0),
    reset(reset),
    assignment(0),
    declaration(0),
    after_build(0),
    targettype(0)

{
}

nt_Block::nt_Block(nt_Assignment const* assignment) :
    NonTerminal(assignment->getLocation()),
    block_type(b_assignment),
    ifblock(0),
    reset(0),
    assignment(assignment),
    declaration(0),
    after_build(0),
    targettype(0)

{
}

nt_Block::nt_Block(nt_Declaration const* declaration) :
    NonTerminal(declaration->getLocation()),
    block_type(b_declaration),
    ifblock(0),
    reset(0),
    assignment(0),
    declaration(declaration),
    after_build(0),
    targettype(0)

{
}

nt_Block::nt_Block(nt_AfterBuild const* after_build) :
    NonTerminal(after_build->getLocation()),
    block_type(b_after_build),
    ifblock(0),
    reset(0),
    assignment(0),
    declaration(0),
    after_build(after_build),
    targettype(0)

{
}

nt_Block::nt_Block(nt_TargetType const* targettype) :
    NonTerminal(targettype->getLocation()),
    block_type(b_targettype),
    ifblock(0),
    reset(0),
    assignment(0),
    declaration(0),
    after_build(0),
    targettype(targettype)

{
}

nt_Block::~nt_Block()
{
}

nt_Block::block_e
nt_Block::getBlockType() const
{
    return this->block_type;
}

nt_IfBlock const*
nt_Block::getIfBlock() const
{
    assert(this->ifblock != 0);
    return this->ifblock;
}

nt_Reset const*
nt_Block::getReset() const
{
    assert(this->reset != 0);
    return this->reset;
}

nt_Assignment const*
nt_Block::getAssignment() const
{
    assert(this->assignment != 0);
    return this->assignment;
}

nt_Declaration const*
nt_Block::getDeclaration() const
{
    assert(this->declaration != 0);
    return this->declaration;
}

nt_AfterBuild const*
nt_Block::getAfterBuild() const
{
    assert(this->after_build != 0);
    return this->after_build;
}

nt_TargetType const*
nt_Block::getTargetType() const
{
    assert(this->targettype != 0);
    return this->targettype;
}
