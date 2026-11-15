#pragma once
#include<unordered_map>
#include<unordered_set>
#include "empty_visitor.h"
#include "iterable_stack.h"
#include "types.h"

#include"compilation.h"

class CallTreeBuilder : public EmptyVisitor
{
    inline static char IN_STORAGE = 1;
    inline static char IN_FUNCTION = 2;
    std::shared_ptr<FileCompilationContext> fileCtx;
    IterableStack<std::unordered_map<std::string,Type>> vars;
    
    Type counted_type;

    std::string current_function_name;

    std::unordered_map<std::string,std::unordered_set<std::string>> graph;

    Type countType(tree::ExprNode*);
    Type typeInferrence(const Type& rhs);

public:

    CallTreeBuilder(std::shared_ptr<FileCompilationContext> fileCtx): fileCtx(fileCtx){}

    void visit(tree::Namespace* nmsp);

    void visit(tree::Function*);
    void visit(tree::InlinedFunction*);
    
    void visit(tree::VarDecl*);
    void visit(tree::Variable*);
    void visit(tree::NumberLiteral* var);
    void visit(tree::TextLiteral* var);
    void visit(tree::RawSelector* var);
    void visit(tree::BoolLiteral* var);
    void visit(tree::InitList* init_list);

    void visit(tree::MemberAccess*);
        void memAccessStruct(tree::Variable*,Type& current_type , size_t& i);
        void memAccessCall(tree::Call*,Type& current_type,size_t& i);
        void memAccessList(tree::Call*,Type& current_type,size_t& i);
        void memberAccessSelector(tree::RawSelector* call, Type& current_type,size_t &i);
    void visit(tree::BinaryOperator*);
    
    void visit(tree::Call*);

    void visit(tree::Block*);
    void visit(tree::While*);
    void visit(tree::If*);
    void visit(tree::ScheduledWhile*);

    Type getSymbol(const std::string& s)
    {
        auto last = static_cast<long long>(vars.size()) - 1;
        while ( last>=0 )
        {
            if (vars[last].contains(s)) return vars[last].at(s);
            --last;
        }
        throw std::runtime_error("Unknown Symbol [" + s + "]");
    }

    std::vector<std::string> findRecursion();
};