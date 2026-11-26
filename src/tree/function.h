#pragma once
#include<optional>
#include"node.h"
#include"block.h"

#include"vardecl.h"

#include "../visitor.h"

TREE_BEGIN
struct Function : public Node
{
    Function(const Token& tok);
    bool is_static = false;
    BlockPtr body;
    std::vector<VarDeclPtr> args;
    
    std::optional<std::pair<std::string,size_t>> ret_type = std::nullopt;
    
    bool alwaysReturn = false;  
    
    void pushArg(VarDeclPtr&&);
    
    void accept(Visitor*);

    virtual ~Function() = default;

};

using FunctionPtr = std::unique_ptr<Function>;

struct InlinedFunction : public Node
{
    BlockPtr body;
    std::vector<VarDeclPtr> args;
    bool is_static = false;
    InlinedFunction(const Token& tok) : Node(tok) {}
    InlinedFunction(const Token& tok,FunctionPtr&& function) : Node(tok){
        body = std::move(function->body);
        args = std::move(function->args);
        is_static = function->is_static;
    }
    
    void accept( Visitor* v ){
        v->visit(this);
    }
};

using InlinedPtr = std::unique_ptr<InlinedFunction>;

TREE_END