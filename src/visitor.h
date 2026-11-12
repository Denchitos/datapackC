#pragma once

#define TREE_ACCEPT_MACRO( Name )\
void tree::Name::accept(Visitor* v)\
{\
    v->visit(this);\
}


namespace tree
{
    class Node;
    class Function;
    class Block;
    class Raw;
    class BinaryOperator;
    class Variable;
    class VarDecl;
    class NumberLiteral;
    class RawSelector;
    class Namespace;
    class MemberAccess;
    class Objective;
    class BoolLiteral;
    class If;
    class While;
    class Break;
    class ScheduledWhile;
    class TextLiteral;
    class Call;
    class Return;
    class StorageDecl;
    class InitList;
    class IndexAccess;
    class InlinedFunction;
}

class Visitor
{
public:
    virtual void visit(tree::Node*) = 0;
    virtual void visit(tree::Function*) = 0;
    virtual void visit(tree::InlinedFunction*) = 0;
    virtual void visit(tree::Block*) = 0;
    virtual void visit(tree::Raw*) = 0;
    virtual void visit(tree::BinaryOperator*) = 0;
    virtual void visit(tree::Variable*) = 0;
    virtual void visit(tree::VarDecl*) = 0;
    virtual void visit(tree::NumberLiteral*) = 0;
    virtual void visit(tree::RawSelector*) = 0;
    virtual void visit(tree::Namespace*) = 0;
    virtual void visit(tree::MemberAccess*) = 0;
    virtual void visit(tree::Objective*) = 0;
    virtual void visit(tree::BoolLiteral*) = 0;
    virtual void visit(tree::If*) = 0;
    virtual void visit(tree::While*) = 0;
    virtual void visit(tree::Break*) = 0;
    virtual void visit(tree::ScheduledWhile*) = 0;
    virtual void visit(tree::TextLiteral*) = 0;
    virtual void visit(tree::Call*) = 0;
    virtual void visit(tree::Return*) = 0;
    virtual void visit(tree::StorageDecl*) = 0;
    virtual void visit(tree::InitList*) = 0;
    //virtual void visit(tree::IndexAccess*) = 0;
    virtual ~Visitor() = default;
};