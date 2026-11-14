#include"declaration_visitor.h"

#include"tree/function.h"
#include"tree/namespace.h"
#include"tree/storage.h"
#include"tree/objective.h"

static std::string getRowCol( tree::Node* node ) 
{
    return "[ " + std::to_string(node->getRow()) + " , " + std::to_string(node->getColumn()) + " ]";
} 

void StorageDeclVisitor::visit(tree::Namespace* nmsp)
{
    fileCtx->currentNamespace = { nmsp->namespace_,nmsp->path };
}

void StorageDeclVisitor::visit(tree::StorageDecl* storage_decl)
{
    if ( !fileCtx->registerType(storage_decl->getName()) )
        throw std::runtime_error(getRowCol(storage_decl)+" Redefinition of type: '" + storage_decl->getName() + "' on line: " + storage_decl->getName() + " in row: " + storage_decl->getName() + " this type name is declared yet");
}

void StorageDeclVisitor::visit(tree::Objective* obj)
{
    if ( !fileCtx->registerObjective(obj->name) )
        throw std::runtime_error(getRowCol(obj) + "  Redifinitaion of objective '"+obj->name+"'" );
}

Value FuncDeclVisitor::declArg(tree::VarDecl* varDecl)
{
    Type type = fileCtx->strToType(varDecl->getVarType());

    auto name = varDecl->getVarName();
    type.list_depth = varDecl->list_depth;
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
            if (argument.is_link) throw std::runtime_error( getRowCol(func)+" Macro arguments aren't implemented yet" );
            
            if (argument.type == Type::selector && !argument.is_link ) throw std::runtime_error ( getRowCol(func) + " Selector can't be a non-macro-argument in 'def-functuons'"  );

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
    try{
        auto func_decl = makeFuncDecl(func);

        if ( func_decl.ret.has_value() && !func->alwaysReturn )
        {
            throw std::runtime_error("Function '"+func->getName()+"' don't return always");
        }

        if (!fileCtx->registerFunc(func->getName(),std::move(func_decl) ))
            throw std::runtime_error("Redifinition of callable name '"+func->getName()+"'");
    }
    catch(std::exception& e)
    {
        throw std::runtime_error(getRowCol(func) + " " + e.what());
    }
}

InlinedFunctionDeclaration FuncDeclVisitor::makeInlinedFuncDecl(tree::InlinedFunction* func)
{
    auto name = func->getName();
    fileCtx->newFunction(name);
    auto mc_name = fileCtx->currentFunction().toMinecraftPath();

    InlinedFunctionDeclaration func_decl(mc_name);

    for (auto& arg : func->args )
    {
        if ( arg != nullptr ){ 
            auto argument = declArg(arg.get());
            func_decl.args.emplace_back(arg->var_name,argument);
        }
    }

    func_decl.body = func->body.get();

    fileCtx->upFunction();
    return func_decl;
}

void FuncDeclVisitor::visit(tree::InlinedFunction* func)
{
    //std::cout<<getRowCol(func)<<" DECLARATING INLINE '"+func->getName()+"'\n";
    auto&& val = makeInlinedFuncDecl(func);
    if ( !fileCtx->registerInlined(func->getName(),std::move(val) ) )
        throw std::runtime_error(getRowCol(func)+" Redifinition of callable name '"+func->getName()+"'");
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
            auto type = fileCtx->strToType(type_name);
            type.list_depth = field->list_depth;

            if ( storageDecl.fields.contains(name) ) throw std::runtime_error( getRowCol(storage_decl)+"Redefinition of field in storage '"+storage_name+"'");

            storageDecl.field_names.emplace_back(name);
            storageDecl.fields.emplace(name,type);
        }
        if ( !storage_decl->methods.empty() )
        {
            fileCtx->currentNamespace.push(storage_decl->getName());

            for ( auto & method : storage_decl->methods )
            {
                auto method_decl = makeFuncDecl(method.get());
                storageDecl.methods[method->getName()] = std::move(method_decl);
            }

            fileCtx->currentNamespace.pop();
        }
        if ( !storage_decl->inlined_methods.empty() )
        {
            fileCtx->currentNamespace.push(storage_decl->getName());

            for ( auto & method : storage_decl->inlined_methods )
            {
                auto method_decl = makeInlinedFuncDecl(method.get());
                storageDecl.inlined_methods[method->getName()] = std::move(method_decl);
            }

            fileCtx->currentNamespace.pop();
        }
        fileCtx->registerStorage(storage_name,std::move(storageDecl));
    }
}
