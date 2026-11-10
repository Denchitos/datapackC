#pragma once
#include "expr.h"
#include<vector>
TREE_BEGIN

struct Call : public ExprNode
{

    Call(const Token& tok) : ExprNode(tok,ExprNode::Kind::Call) {}

    std::vector<ExprNodePtr> call_args;

    static bool kindOf(ExprNode* en)
    {
        return en->getType() == ExprNode::Kind::Call;
    }

    void push( ExprNodePtr&& arg )
    {
        call_args.emplace_back(std::move(arg));
    }

    size_t size() const
    {
        return call_args.size();
    }

    void accept(Visitor* v) override;
};

TREE_END