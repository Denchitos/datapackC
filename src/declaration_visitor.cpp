#include"declaration_visitor.h"

#include"tree/function.h"
#include"tree/namespace.h"
#include"tree/storage.h"
#include"tree/objective.h"
#include"tree/import.h"



void ForwardStorageDeclaration::visit(tree::StorageDecl* storage_decl)
{
    if ( !fileCtx->registerType(storage_decl->getName()) )
        throw std::runtime_error(storage_decl->getRowCol()+" Redefinition of type: '" + storage_decl->getName() + "' on line: " + storage_decl->getName() + " in row: " + storage_decl->getName() + " this type name is declared yet");
}

void ForwardStorageDeclaration::visit(tree::Objective* obj)
{
    if ( !fileCtx->registerObjective(obj->name) )
        throw std::runtime_error(obj->getRowCol() + "  Redifinitaion of objective '"+obj->name+"'" );
}

void RecursiveStorageDeclaration::visit(tree::Import* imp)
{
    auto& file = imp->file_name;

    std::cout<<file<<" ? -";

    if ( isImported(file) ) {std::cout <<"true\n"; return;}

    std::cout<<"false\n";

    visited_import.emplace(file);

    for ( auto& type : modules->at(file).fileCtx->getTypes() )
    {
        fileCtx->pushType(type);
    }

    for ( auto& type : modules->at(file).fileCtx->get_member_str_to_type() )
    {
        fileCtx->get_member_str_to_type().emplace(type);
    }

    std::cout<<"Current Path: "<<path<<"\n";
    std::cout<<"Path: "<<file<<"\n";

    for ( auto & [lval,name]: modules->at(file).fileCtx->getObjectives() )
    {
        std::cout<<"\t"<<lval<<"\t\t"<<name<<"\n";
    }

    for ( auto& [lval,name] : modules->at(file).fileCtx->getObjectives() )
    {
        if (!fileCtx->putObjective(lval,name))
            throw std::runtime_error("Redifinition of objective with inner identifier [" + lval + "] = '"+name+"'" );
    }

    for ( auto& type : modules->at(file).fileCtx->get_member_type_to_str() )
    {
        fileCtx->get_member_type_to_str().emplace(type);
    }

    for ( auto& node : modules->at(file).ast )
    {
        node->accept(this);
    }
}


Value ForwardDefiner::declArg(tree::VarDecl* varDecl)
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

FunctionDeclaration ForwardDefiner::makeFuncDecl(tree::Function* func)
{
    auto name = func->getName();
    fileCtx->newFunction(name);
    auto mc_name = fileCtx->currentFunction().toMinecraftPath();

    FunctionDeclaration func_decl(mc_name);

    for (auto& arg : func->args )
    {
        if ( arg != nullptr ){ 
            auto argument = declArg(arg.get());
            if (argument.is_link) throw std::runtime_error( func->getRowCol()+" Macro arguments aren't implemented yet" );
            
            if (argument.type == Type::selector && !argument.is_link ) throw std::runtime_error ( func->getRowCol() + " Selector can't be a non-macro-argument in 'def-functuons'"  );

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

void ForwardDefiner::visit(tree::Function* func)
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
        throw std::runtime_error(func->getRowCol() + " " + e.what());
    }
}

InlinedFunctionDeclaration ForwardDefiner::makeInlinedFuncDecl(tree::InlinedFunction* func)
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

void ForwardDefiner::visit(tree::InlinedFunction* func)
{
    //std::cout<<->getRowCol()func)<<" DECLARATING INLINE '"+func->getName()+"'\n";
    auto&& val = makeInlinedFuncDecl(func);
    if ( !fileCtx->registerInlined(func->getName(),std::move(val) ) )
        throw std::runtime_error(func->getRowCol()+" Redifinition of callable name '"+func->getName()+"'");
}

void ForwardDefiner::visit(tree::StorageDecl* storage_decl)
{
    auto& storage_name = storage_decl->getName();
    if ( fileCtx->hasType( storage_name ) )
    {
        StorageDefinition storageDecl;
        for ( auto & field : storage_decl->fields )
        {
            auto& name = field->var_name;
            auto& type_name = field->type;
            auto type = fileCtx->strToType(type_name);
            type.list_depth = field->list_depth;

            if ( storageDecl.fields.contains(name) ) throw std::runtime_error( storage_decl->getRowCol()+"Redefinition of field in storage '"+storage_name+"'");
            
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

void RecursiveDefiner::visit(tree::Import* imp)
{
    auto& file = imp->file_name;
    if ( isImported(file) ) return;

    visited_import.emplace(file);

    for ( auto& [name,decl] : modules->at(file).fileCtx->getFuncDeclarations() )
    {
        fileCtx->getFuncDeclarations().emplace(name,decl);
    }

    for ( auto& [name,decl] : modules->at(file).fileCtx->getInlinedDeclarations() )
    {
        fileCtx->getInlinedDeclarations().emplace(name,decl);
    }

    for ( auto& [name,decl] : modules->at(file).fileCtx->getStorageDeclarations() )
    {
        fileCtx->getStorageDeclarations().emplace(name,decl);
    }



    for ( auto& node : modules->at(file).ast )
    {
        node->accept(this);
    }
}
