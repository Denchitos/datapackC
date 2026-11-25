#include<vector>
#include"tree/node.h"

struct FileAST
{
    tree::NodePtr nmsp = nullptr;
    std::vector<tree::NodePtr> imports;
    std::vector<tree::NodePtr> functions;
    std::vector<tree::NodePtr> inlines;
    std::vector<tree::NodePtr> objectives;
    std::vector<tree::NodePtr> storages;
};