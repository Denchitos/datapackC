#pragma once
#include<string>
#include<memory>
#include"../token.h"
class Visitor;

#define TREE_BEGIN namespace tree {
#define TREE_END }

#define NODE_CONSTRUCTOR(Name) tree::Name::Name(const Token& tok): Node(tok){}


TREE_BEGIN
    class Node
    {
        size_t col,row;
        std::string token;
    public:
        Node(const Token& tok);
        const std::string& getName() const {return token;}
        const size_t& getColumn() const {return col;}
        const size_t& getRow() const {return row;}
        virtual void accept(Visitor*);

        virtual ~Node(){};
    };
    using NodePtr = std::unique_ptr<Node>;
TREE_END