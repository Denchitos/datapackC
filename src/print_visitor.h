#include"visitor.h"

class PrintVisitor : public Visitor
{
public:
    void visit(tree::Node*);
    void visit(tree::Function*) ;
    void visit(tree::Block*) ;
    void visit(tree::Raw*) ;
    void visit(tree::BinaryOperator*) ;
    void visit(tree::Variable*) ;
    void visit(tree::VarDecl*) ;
    void visit(tree::NumberLiteral*);
    void visit(tree::RawSelector*);
    void visit(tree::Namespace*);
    void visit(tree::MemberAccess*);
    void visit(tree::Objective*);
    void visit(tree::BoolLiteral*);
    void visit(tree::If*);
    void visit(tree::While*);
    void visit(tree::Break*);
    void visit(tree::ScheduledWhile*);
};