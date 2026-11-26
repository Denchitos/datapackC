#pragma once
#include<vector>
#include"tree/node.h"

struct AST
{
    tree::NodePtr nmsp = nullptr;
    std::vector<tree::NodePtr> imports;
    std::vector<tree::NodePtr> functions;
    std::vector<tree::NodePtr> inlines;
    std::vector<tree::NodePtr> objectives;
    std::vector<tree::NodePtr> storages;
    std::vector<tree::NodePtr> variables;
};