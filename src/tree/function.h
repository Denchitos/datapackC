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

    BlockPtr body;
    std::vector<VarDeclPtr> args;
    
    std::optional<std::pair<std::string,size_t>> ret_type = std::nullopt;

    void pushArg(VarDeclPtr&&);
    
    void accept(Visitor*);

    virtual ~Function() = default;
};

struct InlinedFunction : public Function
{
    InlinedFunction(const Token& tok) : Function(tok) {}
    
    
    void accept( Visitor* v )
    {
        v->visit(this);
    }
};

using FunctionPtr = std::unique_ptr<Function>;
TREE_END