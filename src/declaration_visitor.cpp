#include"declaration_visitor.h"

#include"tree/function.h"
#include"tree/namespace.h"
#include"tree/storage.h"

void StorageDeclVisitor::visit(tree::Namespace* nmsp)
{
    fileCtx->currentNamespace = { nmsp->namespace_,nmsp->path };
}

void StorageDeclVisitor::visit(tree::StorageDecl* storage_decl)
{
    if ( !fileCtx->registerType(storage_decl->getName()) )
        throw std::runtime_error("Redefinition of type: '" + storage_decl->getName() + "' on line: " + storage_decl->getName() + " in row: " + storage_decl->getName() + " this type name is declared yet");
}


Value FuncDeclVisitor::declArg(tree::VarDecl* varDecl)
{
    Type type = fileCtx->strToType(varDecl->getVarType());

    auto name = varDecl->getVarName();

    auto val = fileCtx->nameMangle( type, name );

    return { val,type,varDecl->getVarName() };
}

void FuncDeclVisitor::visit(tree::Namespace* nmsp)
{
    fileCtx->currentNamespace = { nmsp->namespace_,nmsp->path };
}

void FuncDeclVisitor::visit(tree::Function* func)
{

    auto name = func->getName();
    
    fileCtx->newFunction(name);

    auto mc_name = fileCtx->currentFunction().toMinecraftPath();

    FunctionDeclaration func_decl(mc_name);

    for (auto& arg : func->args )
    {
        if ( arg != nullptr ) func_decl.args.push_back(declArg(arg.get()));
    }

    if ( func->ret_type.has_value() )
    {
        auto type = fileCtx->strToType(func->ret_type.value());
        std::string name = "__returned";
        func_decl.ret = Value{fileCtx->nameMangle(type,name) , type, name };
    }
    else
    {
        func_decl.ret = std::nullopt;
    }

    fileCtx->registerFunc(name,std::move(func_decl));

    fileCtx->upFunction();
}

void FuncDeclVisitor::visit(tree::StorageDecl* storage_decl)
{
    auto& storage_name = storage_decl->getName();
    if ( fileCtx->hasType( storage_name ) )
    {
        StorageDeclaration storageDecl;
        for ( auto [ name,type_name ] : storage_decl->fields )
        {
            if ( storageDecl.fields.contains(name)) throw std::runtime_error("Redefinition of field in storage '+"+ storage_name +"'");
            //std::cout<<" registering field in storage "<<storage_name << "<"<<name<<">:<"<<type_name<<">\n";
            storageDecl.field_names.emplace_back(name);
            storageDecl.fields.emplace(name,fileCtx->strToType(type_name));
        }
        fileCtx->registerStorage(storage_name,std::move(storageDecl));
    }
}
