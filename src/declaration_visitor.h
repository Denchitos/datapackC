#pragma once

#include "empty_visitor.h"
#include "compilation.h"

class StorageDeclVisitor : public EmptyVisitor
{
    std::shared_ptr<FileCompilationContext> fileCtx;

public:
    StorageDeclVisitor(std::shared_ptr<FileCompilationContext> ctx) : fileCtx(ctx) {}
    void visit(tree::Namespace*) override;
    void visit(tree::StorageDecl*) override;
};

class FuncDeclVisitor : public EmptyVisitor
{
    std::shared_ptr<FileCompilationContext> fileCtx;

    Value declArg(tree::VarDecl*);
public:
    FuncDeclVisitor(std::shared_ptr<FileCompilationContext> ctx) : fileCtx(ctx) {}
    void visit(tree::Namespace*) override;
    void visit(tree::Function*) override;
    void visit(tree::StorageDecl*) override;
};