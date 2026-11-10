#pragma once
#include<optional>
#include"node.h"
#include"block.h"

#include"vardecl.h"

TREE_BEGIN
struct Function : public Node
{
    Function(const Token& tok);

    BlockPtr body;
    std::vector<VarDeclPtr> args;
    
    std::optional<std::string> ret_type = std::nullopt;

    void pushArg(VarDeclPtr&&);
    
    void accept(Visitor*);
};
using FunctionPtr = std::unique_ptr<Function>;
TREE_END