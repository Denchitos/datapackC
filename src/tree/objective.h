#include "node.h"

TREE_BEGIN

struct Objective : public Node
{
    std::string name;

    Objective(const Token& tok);
    void accept(Visitor*);
};

TREE_END