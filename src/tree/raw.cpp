#include<vector>
#include "raw.h"
#include"../visitor.h"

TREE_BEGIN

Raw::Raw(const Token& tok): Node(tok)
{}

void Raw::pushWord(const std::string& s, bool space_before)
{
    raw.emplace_back(Type::Word,s,-1,false,space_before);
}

void Raw::pushVar(const std::string & s,char id,bool is_literal,bool space_before)
{
    raw.emplace_back(Type::Var,s,id,is_literal,space_before);
}

void Raw::accept(Visitor* v)
{
    v->visit(this);
}

TREE_END