#pragma once
#include<vector>
#include"expr.h"

TREE_BEGIN

struct MemberAccess:public ExprNode
{
    std::vector<ExprNodePtr> members;
    MemberAccess(const Token& tok) : ExprNode(tok,ExprNode::Kind::MemberAccess) {}

    static bool kindOf(ExprNode* e)
    {
        return e->getType() == ExprNode::Kind::MemberAccess;
    }

    void accept(Visitor*);
};

TREE_END