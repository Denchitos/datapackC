#pragma once
#include<vector>
#include"node.h"
namespace tree
{
    struct Block : public Node
    {
        std::vector<NodePtr> content;
        
        Block(const Token& tok);
        void push(NodePtr&& node);
        
        void accept(Visitor*);
    };
    using BlockPtr = std::unique_ptr<Block>;

}