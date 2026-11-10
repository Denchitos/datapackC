#pragma once
#include "expr.h"
TREE_BEGIN

struct Return : public Node
{
    ExprNodePtr expr = nullptr;
    Return(const Token& tok) : Node(tok){}

    void accept(Visitor* v);
};

TREE_END