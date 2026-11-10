#include<iostream>
#include<iomanip>
#include"print_visitor.h"
#include"tree/node.h"
#include"tree/block.h"
#include"tree/function.h"
#include"tree/raw.h"
#include"tree/expr.h"
#include"tree/vardecl.h"
#include"tree/raw_selector.h"
#include"tree/namespace.h"
#include"tree/member_access.h"
#include"tree/objective.h"
#include"tree/if.h"
#include"tree/while.h"
void PrintVisitor::visit(tree::Node* n)
{
    if (n) std::cout<<"NODE ["<<n->getName()<<" : "<<std::setw(5)<<n->getRow()<<","<<std::setw(5)<<n->getColumn()<<"]\n";
}

void PrintVisitor::visit(tree::Function* f)
{
    if ( !f ) return;

    std::cout<<"FUNCTION ["<<f->getName()<<"]\n";
    std::cout<<"ARGS : \n(\n";
    for ( auto& it : f->args)
        it->accept(this);
    std::cout<<")\n";

    if (!f->body) return;

    visit(f->body.get());
}
void PrintVisitor::visit(tree::Block* b)
{
    std::cout<<"BLOCK {\n";

    for ( auto& it : b->content )
    {
        if ( it ) it->accept(this);
    }

    std::cout<<"}\n";
}
void PrintVisitor::visit(tree::Raw* r)
{
    if (!r) return;
    std::cout<<"[RAW]";
    for (auto &it : r->raw)
    {
        std::cout<<( it.space_before ? " " : "");
        switch (it.type){
            case tree::Raw::Word:{
                std::cout<<it.str;
                break;
            }
            case tree::Raw::Var:{
                std::cout<<"[VAR]"<<it.str;
            }
        }
    }
    std::cout<<"\n";
}

void PrintVisitor::visit(tree::BinaryOperator* b)
{
    std::cout<<"[Bin] "<<b->getName()<<" (\n";

    b->left->accept(this);
    b->right->accept(this);

    std::cout<<")\n";
}

void PrintVisitor::visit(tree::Variable* v)
{
    std::cout<<"[VAR] "<<v->getName()<<"\n";
}

void PrintVisitor::visit(tree::NumberLiteral* v)
{
    std::cout<<"[NUMBER] "<<v->getName()<<"\n";
}

void PrintVisitor::visit(tree::Objective* v)
{
    std::cout<<"[OBJECTIVE] "<<v->name<<"\n";
}

void PrintVisitor::visit(tree::RawSelector* v)
{
    std::cout<<"[SELECTOR] "<< v->type<<"["<<v->params<<"]\n";
}

void PrintVisitor::visit(tree::Namespace* v)
{
    std::cout<<"[NAMESPACE] "<< v->namespace_<<":";
    auto size = v->path.size();
    for ( size_t i = 0; i < size; i++ )
    {
        std::cout<<v->path[i];
        if ( i != size-1 ) std::cout<<"/";
    }
    std::cout<<"\n";
}

void PrintVisitor::visit(tree::MemberAccess* mem)
{
    std::cout<<"[MEMBER ACCESS]"<<mem->members[0];
    auto size = mem->members.size();
    for (size_t i = 1; i<size;i++)
    {
        std::cout<<"."<<mem->members[i];
    }
    std::cout<<"\n";
}


void PrintVisitor::visit(tree::VarDecl* vardecl)
{
    std::cout << "[VARDECL] (\n";
    
    std::cout<<vardecl->getVarName() << ":";
    std::cout<<vardecl->getVarType() << "\n";


    if ( vardecl->isMut() ) std::cout<<"Mutable\n";
    
    if (vardecl->expr) {;
        std::cout<<"=\n";
        vardecl->expr->accept(this);
    }
    std::cout<<")\n";
}

void PrintVisitor::visit(tree::BoolLiteral* lit)
{
    std::cout << "[BOOLLITERAL] " << ( lit->val ? "true" : "false" );
}

void PrintVisitor::visit(tree::If* If)
{
    std::cout<<"[CONDITION_BLOCK] (\n";
    for ( auto& it : If->elif ){
        std::cout << "[IF] (\n";

        it.first->accept(this);
        std::cout<<"[THEN] (\n";
        it.second->accept(this);
        std::cout<<") [THEN]\n) [IF]\n";
    }
    if ( If->else_ )
    {
        std::cout<<"[ELSE] (\n";
        If->else_->accept(this);
        std::cout<<") [ELSE]\n";
    }


    std::cout<<") [CONDITION_BLOCK]\n";
}

void PrintVisitor::visit(tree::While* While)
{
    std::cout<<"[WHILE BLOCK] (\n";
    std::cout<<"[CONDITION] (\n";
    While->expr->accept(this);
    std::cout<<"[THEN] (";
    While->body->accept(this);
    std::cout<<") [THEN]\n ) [CONDITION]\n";
    std::cout<<") [WHILE_BLOCK]\n";
}

void PrintVisitor::visit(tree::Break*)
{
    std::cout<<"[BREAK]\n";
}

void PrintVisitor::visit(tree::ScheduledWhile* While)
{
    std::cout<<"[SCHEDULED WHILE BLOCK] (\n";
    std::cout<<"[CONDITION] (\n";
    While->expr->accept(this);
    std::cout<<"[THEN] (";
    While->body->accept(this);
    std::cout<<") [THEN]\n ) [CONDITION]\n";
    std::cout<<") [WHILE_BLOCK]\n";
}
