#pragma once
#include<unordered_set>
#include "empty_visitor.h"
#include "compilation.h"
#include"tree/block.h"
#include"modules.h"
class StorageDeclVisitor : public EmptyVisitor
{
    std::shared_ptr<FileCompilationContext> fileCtx;
    std::shared_ptr<Modules> modules;
    std::unordered_set<fs::path> visited_import;

public:
    StorageDeclVisitor(std::shared_ptr<FileCompilationContext> ctx,std::shared_ptr<Modules> modules) : fileCtx(ctx),modules(modules) {}
    void visit(tree::Namespace*) override;
    void visit(tree::StorageDecl*) override;
    void visit(tree::Objective*) override;
    void visit(tree::Import*) override;
};

class FuncDeclVisitor : public EmptyVisitor
{
    std::shared_ptr<FileCompilationContext> fileCtx;
    std::shared_ptr<Modules> modules;
    std::unordered_set<fs::path> visited_import;

    Value declArg(tree::VarDecl*);
    FunctionDeclaration makeFuncDecl(tree::Function* func);
    InlinedFunctionDeclaration makeInlinedFuncDecl(tree::InlinedFunction* func);
public:
    FuncDeclVisitor(std::shared_ptr<FileCompilationContext> ctx,std::shared_ptr<Modules> modules) : fileCtx(ctx),modules(modules) {}
    void visit(tree::Namespace*) override;
    void visit(tree::Function*) override;
    void visit(tree::InlinedFunction*) override;
    void visit(tree::StorageDecl*) override;
    void visit(tree::Import*) override;
};