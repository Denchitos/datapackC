#pragma once
#include "node.h"
#include "function.h"
#include "../visitor.h"
TREE_BEGIN

struct StorageDecl : public Node
{
    using field = std::pair<std::string,std::string>;

    std::string name;
    std::vector<VarDeclPtr> fields;
    std::vector<FunctionPtr> methods;

    StorageDecl(const Token& tok) : Node(tok){}

    void pushMethod(FunctionPtr&& method)
    {
        methods.emplace_back(std::move(method));
    }

    void pushField(VarDeclPtr&& var_decl)
    {
        fields.emplace_back(std::move(var_decl));
    }

    void accept(Visitor* v) override 
    {
        v->visit(this);
    };
};

TREE_END