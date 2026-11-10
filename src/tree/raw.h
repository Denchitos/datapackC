#pragma once
#include"node.h"

TREE_BEGIN

struct Raw : public Node
{
    enum Type
    {
        Word,
        Var,
    };

    struct Item
    {
        Type type;
        std::string str;
        char id;
        bool is_literal;
        bool space_before;
    };

    Raw(const Token& t);
    std::vector<Item> raw;

    void pushWord(const std::string& s,bool space_before);
    void pushVar(const std::string & s,char id,bool is_literal,bool space_before);

    void accept(Visitor*) override;
};

using RawPtr = std::unique_ptr<Raw>;

TREE_END