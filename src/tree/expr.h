#pragma once
#include "node.h"

#include<vector>
TREE_BEGIN
struct ExprNode : public Node
{
public:
    enum class Kind
    {
        Variable,
        BinaryOperator,
        NumberLiteral,
        BoolLiteral,
        TextLiteral,
        FloatLiteral,
        RawSelector,
        MemberAccess,
        Call,
        InitList,
        IndexAccess
    };
    ExprNode(const Token& tok,Kind kind) : Node(tok), kind(kind) {}

    Kind getType() const
    {
        return kind;
    }
private:
    Kind kind;
};

using ExprNodePtr = std::unique_ptr<ExprNode>;

struct BinaryOperator : public ExprNode
{
    BinaryOperator(const Token& tok) : ExprNode(tok,ExprNode::Kind::BinaryOperator){}
    void accept(Visitor*);

    static bool kindOf(ExprNode* en)
    {
        return en->getType() == ExprNode::Kind::BinaryOperator;
    }

    std::unique_ptr<ExprNode> left;
    std::unique_ptr<ExprNode> right;
};

struct Variable : public ExprNode
{
    Variable(const Token& tok) : ExprNode(tok,ExprNode::Kind::Variable){}
    void accept(Visitor*);
    static bool kindOf(ExprNode* en)
    {
        return en->getType() == ExprNode::Kind::Variable;
    }
};

struct NumberLiteral : public ExprNode
{
    NumberLiteral(const Token& tok) : ExprNode(tok,ExprNode::Kind::NumberLiteral){}
    long long toInteger();
    double toDouble();
    void accept(Visitor*);
    
    static bool kindOf(ExprNode* en)
    {
        return en->getType() == ExprNode::Kind::NumberLiteral;
    }
};

struct BoolLiteral : public ExprNode
{
    bool val;
    BoolLiteral(const Token& tok) : ExprNode(tok,ExprNode::Kind::BoolLiteral){}
    
    std::string toStr();
    
    void accept(Visitor*);

    static bool kindOf(ExprNode* en)
    {
        return en->getType() == ExprNode::Kind::BoolLiteral;
    }
};

struct TextLiteral : public ExprNode
{
    TextLiteral(const Token& tok) : ExprNode(tok,ExprNode::Kind::TextLiteral) {}
    void accept(Visitor*);
    static bool kindOf(ExprNode* en)
    {
        return en->getType() == ExprNode::Kind::TextLiteral;
    }
};

struct InitList : public ExprNode
{
    InitList(const Token& tok) : ExprNode(tok,ExprNode::Kind::InitList){}
    
    std::vector<ExprNodePtr> list;
    
    void accept(Visitor*);

    static bool kindOf(ExprNode* en)
    {
        return en->getType() == ExprNode::Kind::InitList;
    }
};

struct IndexAccess : public ExprNode
{
    IndexAccess(const Token& tok) : ExprNode(tok,ExprNode::Kind::IndexAccess) {}

    ExprNodePtr object;
    
    std::string index;
};

using BinOpPtr = std::unique_ptr<BinaryOperator>;

template<typename T>
T* exprCast(ExprNode* e)
{
    return T::kindOf(e) ? static_cast<T*>(e) : nullptr;
}

TREE_END