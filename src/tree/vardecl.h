#pragma once

#include"expr.h"
TREE_BEGIN

struct VarDecl : public Node
{
    std::string type = "auto";
    bool is_link = false;
    std::string var_name ;

    size_t list_depth = 0;
    
    std::unique_ptr<ExprNode> expr = nullptr;
    bool mut = false;

    VarDecl(const Token&);
    void setVarType(const std::string& type);
    void setVarName(const std::string&);

    const std::string& getVarType();
    const std::string& getVarName();

    void setMut(bool);
    bool isMut();

    void accept(Visitor*);
};

struct OutScopeVar : public Node
{
    std::unique_ptr<VarDecl> var_decl;

    OutScopeVar(const Token& tok) : Node(tok) {}
    void accept(Visitor*);
    bool is_static = false;
};

using VarDeclPtr = std::unique_ptr<VarDecl>;

TREE_END