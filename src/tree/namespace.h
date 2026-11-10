#pragma once
#include "node.h"

#include<vector>

TREE_BEGIN

struct Namespace : public Node
{
    std::string namespace_ = "minecraft";
    std::vector<std::string> path;

    Namespace(const Token& tok);
    void accept(Visitor*) override;
};

TREE_END