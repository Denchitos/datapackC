#pragma once
#include "node.h"
#include "../visitor.h"
TREE_BEGIN

struct StorageDecl : public Node
{
    using field = std::pair<std::string,std::string>;

    std::string name;
    std::vector<field> fields;

    StorageDecl(const Token& tok) : Node(tok){}

    void pushField(const std::string& name,const std::string& type)
    {
        fields.emplace_back(name,type);
    }

    void accept(Visitor* v) override 
    {
        v->visit(this);
    };
};

TREE_END