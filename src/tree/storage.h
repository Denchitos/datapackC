#pragma once
#include "node.h"
#include "function.h"

#include "../visitor.h"

#include "../file_ast.h"

TREE_BEGIN

struct StorageDecl : public Node
{
    using field = std::pair<std::string,std::string>;

    std::string name;

    std::unique_ptr<AST> ast;

    StorageDecl(const Token& tok) : Node(tok){}

    void accept(Visitor* v) override 
    {
        v->visit(this);
    };
};

TREE_END