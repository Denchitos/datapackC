#include<filesystem>
#include"node.h"


namespace fs = std::filesystem;

TREE_BEGIN

struct Import : public Node
{
    fs::path file_name;
    Import(const Token& tok) : Node(tok){}

    void accept(Visitor* vis)
    {
        vis->visit(this);
    }
};

TREE_END