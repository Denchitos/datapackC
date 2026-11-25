#include<vector>
#include "raw.h"
#include"../visitor.h"

TREE_BEGIN

void Raw::accept(Visitor* v)
{
    v->visit(this);
}

TREE_END