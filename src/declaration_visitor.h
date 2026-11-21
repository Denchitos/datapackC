#pragma once
#include<unordered_set>
#include "empty_visitor.h"
#include "compilation.h"
#include"tree/block.h"
#include"modules.h"


class SupportVisitor : public EmptyVisitor
{
protected:
    std::shared_ptr<FileCompilationContext> fileCtx;
    std::shared_ptr<Modules> modules;

    SupportVisitor(std::shared_ptr<FileCompilationContext> ctx,std::shared_ptr<Modules> modules) : fileCtx(ctx),modules(modules) {}
};


class ForwardStorageDeclaration : public SupportVisitor
{
public:
    ForwardStorageDeclaration(std::shared_ptr<FileCompilationContext> ctx,std::shared_ptr<Modules> modules) : SupportVisitor(ctx,modules){}
    void visit(tree::StorageDecl*) override;
    void visit(tree::Objective*) override;
};

class RecursiveStorageDeclaration : public SupportVisitor
{
    std::unordered_set<fs::path> visited_import;
public:

    bool isImported(const fs::path& path)
    {
        return visited_import.contains(path);
    }

    RecursiveStorageDeclaration(const fs::path& path,std::shared_ptr<FileCompilationContext> ctx,std::shared_ptr<Modules> modules) : SupportVisitor(ctx,modules)
    {
        visited_import.emplace(path);
    }
    
    void visit(tree::Import*) override;
};



class ForwardDefiner : public SupportVisitor
{
    Value declArg(tree::VarDecl*);
    FunctionDeclaration makeFuncDecl(tree::Function* func);
    InlinedFunctionDeclaration makeInlinedFuncDecl(tree::InlinedFunction* func);
public:
    ForwardDefiner(std::shared_ptr<FileCompilationContext> ctx,std::shared_ptr<Modules> modules) : SupportVisitor(ctx,modules) {}
    void visit(tree::Function*) override;
    void visit(tree::InlinedFunction*) override;
    void visit(tree::StorageDecl*) override;
};


class RecursiveDefiner : public SupportVisitor
{
    std::unordered_set<fs::path> visited_import;
public:

    bool isImported(const fs::path& path)
    {
        return visited_import.contains(path);
    }
    RecursiveDefiner(const fs::path& path,std::shared_ptr<FileCompilationContext> ctx,std::shared_ptr<Modules> modules) : SupportVisitor(ctx,modules)
    {
        visited_import.emplace(path);
    }

    void visit(tree::Import*) override;
};