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

    bool is_link = varDecl->is_link;
    
    Value value{ val,type,varDecl->getVarName() };
    value.is_link = is_link;
    return value;
}

void FuncDeclVisitor::visit(tree::Namespace* nmsp)
{
    fileCtx->currentNamespace = { nmsp->namespace_,nmsp->path };
}

FunctionDeclaration FuncDeclVisitor::makeFuncDecl(tree::Function* func)
{
    auto name = func->getName();
    fileCtx->newFunction(name);
    auto mc_name = fileCtx->currentFunction().toMinecraftPath();

    FunctionDeclaration func_decl(mc_name);

    for (auto& arg : func->args )
    {
        if ( arg != nullptr ){ 
            auto argument = declArg(arg.get());
            if (argument.is_link) throw std::runtime_error( "Macro argument aren't implemented yet" );
            func_decl.args.emplace_back(argument);
        }
    }

    if ( func->ret_type.has_value() )
    {
        auto type = fileCtx->strToType(func->ret_type.value().first);
        type.list_depth = func->ret_type.value().second;
        std::string name = "__returned";
        func_decl.ret = Value{fileCtx->nameMangle(type,name) , type, name };
    }
    else
    {
        func_decl.ret = std::nullopt;
    }
    fileCtx->upFunction();
    return func_decl;
}

void FuncDeclVisitor::visit(tree::Function* func)
{
    auto func_decl = makeFuncDecl(func);
    fileCtx->registerFunc(func->getName(),std::move(func_decl));
}

void FuncDeclVisitor::visit(tree::StorageDecl* storage_decl)
{
    auto& storage_name = storage_decl->getName();
    if ( fileCtx->hasType( storage_name ) )
    {

        StorageDeclaration storageDecl;
        for ( auto & field : storage_decl->fields )
        {
            auto& name = field->var_name;
            auto& type_name = field->type;
            //std::cout<<"Declaration '" << storage_name << "' field  : <" << name <<"> : <"<< type_name <<"> (" << field->list_depth << ")\n";
            auto type = fileCtx->strToType(type_name);
            type.list_depth = field->list_depth;

            if ( storageDecl.fields.contains(name) ) throw std::runtime_error( "Redefinition of field in storage '"+storage_name+"'");

            storageDecl.field_names.emplace_back(name);
            storageDecl.fields.emplace(name,type);
        }
        if ( !storage_decl->methods.empty() )
        {
            fileCtx->currentNamespace.push(storage_decl->getName());

            for ( auto & method : storage_decl->methods )
            {
                auto method_decl = makeFuncDecl(method.get());
                //std::cout<<"Declared method " << method->getName();
                //std::cout<<" ,has ret type? - " << method_decl.ret.has_value();
                //std::cout<<"\n";
                storageDecl.methods[method->getName()] = std::move(method_decl);
            }

            fileCtx->currentNamespace.pop();
        }
        fileCtx->registerStorage(storage_name,std::move(storageDecl));
    }
}
