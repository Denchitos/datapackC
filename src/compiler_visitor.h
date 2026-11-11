#pragma once
#include"visitor.h"
#include"memory"
#include<unordered_map>
#include<stack>
#include<vector>
#include<iostream>
#include"iterable_stack.h"
#include"ExprCodeGen.h"
#include"compilation.h"
#include"tree/storage.h"
#include"tree/expr.h"
class CompilerVisitor : public Visitor
{
    std::shared_ptr<FileCompilationContext> fileCtx;
    std::shared_ptr<ExprCodeGen> codeGen;
    IterableStack<Value> op_stack;

    StorageDeclaration* current_storage = nullptr;
public:
    std::vector<Function> compiled_functions;
    CompilerVisitor(std::shared_ptr<FileCompilationContext>,std::shared_ptr<ExprCodeGen>);

    std::pair<Value,Value> getTwoOperands();
    Value getTopOperand();

    inline void error(const std::string& s)
    {
        std::cout<<s;
    }
    Type typeInferrence(const Value& rhs);
    void visit(tree::Node*) override {}
    void visit(tree::Raw*) override;
    void visit(tree::Function*) override;
    void visit(tree::Block*) override;
    void visit(tree::BinaryOperator*) override;
    void visit(tree::Variable*) override;
    void visit(tree::VarDecl*) override;
    void visit(tree::NumberLiteral*) override;
    void visit(tree::TextLiteral*) override;
    void visit(tree::RawSelector*) override;
    void visit(tree::Namespace*) override;
    void visit(tree::MemberAccess*) override;
        void genStructMemberAccess(tree::Variable* node,Value& value,size_t& i);
        void genCallMemberAccess(tree::Call*mem, Value& value,size_t &i);
        void genSelectorMemberAccess(tree::RawSelector*mem, Value& value,size_t &i);
    void visit(tree::Objective*) override;
    void visit(tree::BoolLiteral*) override;
    void visit(tree::If*) override;
    void visit(tree::While*) override;
    void visit(tree::ScheduledWhile*);
    void visit(tree::Break*) override;
    void visit(tree::Call*) override;
        void emitCall(const std::vector<Value>& call_args,FunctionDeclaration& fd);
    void visit(tree::Return*) override;
    void visit(tree::StorageDecl* ) override;

    void visit(tree::InitList* init_list) override
    {
        std::vector<std::string> value;

        for ( auto& it : init_list->list )
        {
            value.emplace_back("<");
            it->accept(this);
            auto v = getTopOperand();
            value.emplace_back(fileCtx->typeToStr(v.type));
            for ( auto & val : v.value )
            {
                value.emplace_back(val);
            }
            value.emplace_back(">");
        }

        // for (auto& it : value)
        // {
        //     std::cout<<it<<" ";
        // }
        // std::cout<<"\n";

        op_stack.emplace(value,Type::initializer_list);
    };
};