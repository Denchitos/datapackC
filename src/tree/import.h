#include<filesystem>
#include"node.h"


namespace fs = std::filesystem;

TREE_BEGIN

struct Import : public Node
{
    fs::path file_name;
    
    bool bad_import = false;

    Import(const Token& tok) : Node(tok){}

    void accept(Visitor* vis)
    {
        if ( !bad_import ) vis->visit(this);
    }
};

TREE_END