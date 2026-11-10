#pragma once
#include"expr.h"

TREE_BEGIN



struct RawSelector : public ExprNode
{
    RawSelector(const Token& tok) : ExprNode(tok,ExprNode::Kind::RawSelector){}
    const inline static std::string nearest = "@n";
    const inline static std::string random = "@r";
    const inline static std::string executor = "@s";
    const inline static std::string all = "@a";
    const inline static std::string entities = "@e"; 

    static bool kindOf(ExprNode* e)
    {
        return e->getType() == ExprNode::Kind::RawSelector;
    }

    void accept(Visitor*) override;

    std::string type = executor;
    std::string params;
};

TREE_END