#pragma once
#include<vector>
#include"node.h"
TREE_BEGIN

struct If : public Node
{
    If(const Token& tok);
    std::vector<std::pair<NodePtr,NodePtr>> elif;
    NodePtr else_ = nullptr;
    bool alwaysReturn = false;  
    virtual void accept(Visitor*);
};

TREE_END