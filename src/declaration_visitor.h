#pragma once

#include "empty_visitor.h"
#include "compilation.h"
#include"tree/block.h"
class StorageDeclVisitor : public EmptyVisitor
{
    std::shared_ptr<FileCompilationContext> fileCtx;

public:
    StorageDeclVisitor(std::shared_ptr<FileCompilationContext> ctx) : fileCtx(ctx) {}
    void visit(tree::Namespace*) override;
    void visit(tree::StorageDecl*) override;
    void visit(tree::Objective*) override;
};

class FuncDeclVisitor : public EmptyVisitor
{
    std::shared_ptr<FileCompilationContext> fileCtx;

    Value declArg(tree::VarDecl*);
    FunctionDeclaration makeFuncDecl(tree::Function* func);
    InlinedFunctionDeclaration makeInlinedFuncDecl(tree::InlinedFunction* func);
public:
    FuncDeclVisitor(std::shared_ptr<FileCompilationContext> ctx) : fileCtx(ctx) {}
    void visit(tree::Namespace*) override;
    void visit(tree::Function*) override;
    void visit(tree::InlinedFunction*) override;
    void visit(tree::StorageDecl*) override;
};