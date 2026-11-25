#pragma once
#include"expr.h"
#include<variant>
TREE_BEGIN

struct ExprNode;

struct Raw : public Node
{
    struct Emplace
    {
        bool is_literal = false;
    };

    struct Var : public Emplace
    {
        std::unique_ptr<ExprNode> expr = nullptr;
        char id = -1;
    };

    struct Func : public Emplace
    {
        std::string function;
        std::string var_name;
    };

    struct Word
    {
        size_t r,c;
        std::string tok;
    };

    struct Literal : public Word {};

    using Variant = std::variant<Var,Word,Literal>;

    struct Item
    {
        Variant item;
        bool space_before;
    };
    Raw(const Token& t) : Node(t) {}
    std::vector<Item> raw;

    void accept(Visitor*) override;
};

using RawPtr = std::unique_ptr<Raw>;

TREE_END