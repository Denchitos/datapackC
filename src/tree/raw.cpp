#include<vector>
#include "raw.h"
#include"../visitor.h"

TREE_BEGIN

Raw::Raw(const Token& tok): Node(tok)
{}

void Raw::accept(Visitor* v)
{
    v->visit(this);
}

TREE_END