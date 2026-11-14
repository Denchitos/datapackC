#pragma once
#include"visitor.h"

class EmptyVisitor : public Visitor
{
public:
    void visit(tree::Node*) override  {};
    void visit(tree::Function*) override {};
    void visit(tree::Block*) override {};
    void visit(tree::Raw*) override {};
    void visit(tree::BinaryOperator*) override {};
    void visit(tree::Variable*) override {};
    void visit(tree::VarDecl*) override {};
    void visit(tree::NumberLiteral*) override {};
    void visit(tree::RawSelector*) override {};
    void visit(tree::Namespace*) override {};
    void visit(tree::MemberAccess*) override {};
    void visit(tree::Objective*) override {};
    void visit(tree::BoolLiteral*) override {};
    void visit(tree::If*) override {};
    void visit(tree::While*) override {};
    void visit(tree::Break*) override {};
    void visit(tree::ScheduledWhile*) override {};
    void visit(tree::TextLiteral*) override {};
    void visit(tree::Call*) override {};
    void visit(tree::Return*) override {};
    void visit(tree::StorageDecl*) override {};
    void visit(tree::InitList*) override {};
    void visit(tree::InlinedFunction*) override {};
    virtual ~EmptyVisitor() = default;
};