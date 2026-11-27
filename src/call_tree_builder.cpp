#include<functional>
#include "call_tree_builder.h"
#include"tree/vardecl.h"
#include"tree/expr.h"
#include"tree/member_access.h"
#include"tree/call.h"
#include"tree/function.h"
#include "tree/namespace.h"
#include"tree/block.h"
#include"tree/while.h"
#include"tree/if.h"
#include"tree/raw_selector.h"

void CallTreeBuilder::visit(tree::Namespace* nmsp)
{
    fileCtx->currentNamespace = { nmsp->namespace_,nmsp->path };
    fileCtx->currentDefinedNamespace = { nmsp->namespace_,nmsp->path };
}

Type CallTreeBuilder::typeInferrence(const Type& rhs)
{
    if ( rhs == Type::numberLiteral ) return Type::score;
    else if ( rhs == Type::textLiteral ) return Type::string;
    else if ( rhs == Type::boolLiteral ) return Type::score_bool;
    else if ( rhs == Type::rawSelector ) return Type::selector; 
    else if ( rhs == Type::initializer_list ) throw std::runtime_error("Unable inferrence type of initializer list");
    else return rhs;
}

Type CallTreeBuilder::countType(tree::ExprNode* expr)
{
    if (!expr) return {Type::void_t};
    counted_type = Type::void_t;
    expr->accept(this);
    return counted_type;
}

void CallTreeBuilder::visit(tree::Variable* var)
{
    try
    {
        counted_type = getSymbol(var->getName());
    }
    catch(std::exception& e)
    {
        throw std::runtime_error(var->getRowCol()+" "+e.what());
    }
}

void CallTreeBuilder::visit(tree::BoolLiteral*)
{
    counted_type = Type(Type::boolLiteral);
}

void CallTreeBuilder::visit(tree::TextLiteral*)
{
    counted_type = Type(Type::textLiteral);
}

void CallTreeBuilder::visit(tree::NumberLiteral*)
{
    counted_type = Type(Type::numberLiteral);
}

void CallTreeBuilder::visit(tree::RawSelector*)
{
    counted_type = Type(Type::rawSelector);
}

void CallTreeBuilder::visit(tree::InitList*)
{
    counted_type = Type(Type::initializer_list);
}

void CallTreeBuilder::visit(tree::VarDecl* varDecl)
{
    auto& name = varDecl->var_name;
    auto& type_name = varDecl->type;
    
    if ( type_name != "auto" )
    {
        vars.top()[name] = fileCtx->strToType(type_name);
        vars.top()[name].list_depth = varDecl->list_depth;
        return;
    }
    
    if ( varDecl->expr == nullptr ) throw std::runtime_error(varDecl->getRowCol()+" Expected an expression after varaible declaration in scope");
    
    try{
        auto t = typeInferrence(countType(varDecl->expr.get()));
        vars.top()[name] = t;
        varDecl->type = fileCtx->typeToStr( t );
        vars.top()[name].list_depth = varDecl->list_depth;
    }
    catch(std::exception& e){
        throw std::runtime_error(varDecl->getRowCol()+" "+e.what());
    }
}

void CallTreeBuilder::visit(tree::BinaryOperator* binop)
{
    try
    {
        counted_type = fileCtx->getOperationResult(binop->getName(),countType(binop->left.get()),countType(binop->right.get()));
    }
    catch(std::exception& e)
    {
        throw std::runtime_error(binop->getRowCol()+" "+e.what());
    }
}

void CallTreeBuilder::visit(tree::UnaryOperator* unop)
{
    try
    {
        counted_type = fileCtx->getOperationResult(unop->getName(),countType(unop->expr.get()));
    }
    catch(std::exception& e)
    {
        throw std::runtime_error(unop->getRowCol()+" "+e.what());
    }
}

void CallTreeBuilder::visit(tree::MemberAccess* mem)
{
    auto size = mem->members.size();
    auto& members = mem->members;
    size_t i = 0;
    Type current_type = Type::auto_t;
    while ( i < size  )
    {
        if ( auto var = exprCast<tree::Variable>( members[i].get() ) ) memAccessStruct(var,current_type,i);
        else if ( auto call = exprCast<tree::Call>( members[i].get() ) ) memAccessCall(call,current_type,i);
        else if ( auto sel = exprCast<tree::RawSelector>( members[i].get() ) ) memberAccessSelector(sel,current_type,i);
        else throw std::runtime_error(mem->members[i]->getRowCol() +" Unexpected Situation in member access");
    }
    counted_type = current_type;
}

void CallTreeBuilder::memAccessStruct(tree::Variable* var,Type& current_type,size_t& i)
{
    if ( current_type == Type::auto_t )
    {
        current_type = countType(var);
        i++;
    }
    else if ( current_type == Type::selector || current_type == Type::rawSelector )
    {
        current_type = Type::score;
        i++;
    }
    else
    {
        auto& field = var->getName();
        i++;
        if ( Type::isBuiltinType(current_type) ) throw std::runtime_error(  var->getRowCol() + " Builtin type " + fileCtx->typeToStr(current_type) + " has no field '"+field + "'" );
        if ( !fileCtx->hasStorage(current_type) ) throw std::runtime_error(var->getRowCol() + " Unknown storage '" + fileCtx->typeToStr(current_type) +"'");
        auto& fields = fileCtx->getStorageDecl(current_type).fields;
        auto it = fields.find(field);
        if ( it == fields.end() ) throw std::runtime_error(var->getRowCol() + " Unknown member '"+ field + "' of storage '"+ fileCtx->typeToStr(current_type) +"'");
        current_type = it->second;
    }
}

void CallTreeBuilder::memAccessCall(tree::Call* call,Type& current_type,size_t& i)
{
    i++;
    if ( current_type == Type::auto_t )
    {
        current_type = countType( call );
    }
    else if ( current_type.isList() )
    {
        memAccessList(call,current_type,i);
    }
    else if ( current_type == Type::rawSelector || current_type == Type::selector )
    {
        current_type = countType(call);
    }
    else {
        auto& storage_decl = fileCtx->getStorageDecl(current_type);
        if ( storage_decl.methods.contains(call->getName()) )
        {
            auto method_decl = storage_decl.methods[call->getName()];

            for ( auto& it : call->call_args ) it->accept(this);

            if ( method_decl.ret.has_value() )
                current_type = method_decl.ret.value().type;
            else current_type = Type(Type::void_t);
            graph[current_function_name].insert(method_decl.minecraft_name);
        }
        else if ( storage_decl.inlined_methods.contains(call->getName()) )
        {
            graph[current_function_name].insert(storage_decl.inlined_methods[call->getName()].minecraft_name);

            for ( auto& it : call->call_args ) it->accept(this);

            current_type = Type(Type::void_t);
        }
        else throw std::runtime_error( call->getRowCol() +" Unknown member function '" + call->getName() +"'");
    }
}

void CallTreeBuilder::memAccessList(tree::Call* call,Type& current_type,size_t& i)
{
    auto& name = call->getName();
    if      ( name == "operator[]" ) current_type = current_type.getListDown();
    else if ( name == "len" )    current_type = Type::score;
    else if ( name == "append" || name == "prepend" || name == "insert" || name == "remove");
    else throw std::runtime_error(call->getRowCol() + " unknown member function '" + call->getName() + "' for builtin type " + fileCtx->typeToStr(current_type));
}

void CallTreeBuilder::memberAccessSelector(tree::RawSelector* call, Type& current_type,size_t &i)
{
    i++;
    current_type = Type::selector;
}

void CallTreeBuilder::visit(tree::Call* call)
{
    auto& name = call->getName();
    bool is_function = fileCtx->hasFunc(name);
    bool is_inlined = fileCtx->hasInlined(name);
    if ( !is_function && !is_inlined )
    {
        throw std::runtime_error( call->getRowCol() + " Unknown name to call: " + name);
    }
    try {

        for ( auto& it : call->call_args ) it->accept(this);

        if ( is_inlined ){
            counted_type = Type::void_t;
            graph[current_function_name].insert(fileCtx->getInlinedDecl(name).minecraft_name);
        }
            else if ( is_function ){
            auto& fd =fileCtx->getFuncDecl(name); 
            
            if (fd.ret.has_value())
                counted_type = fd.ret.value().type;
            else counted_type = Type::void_t;

            graph[current_function_name].insert(fd.minecraft_name);
        }
    }
    catch ( std::exception& e )
    {
        throw std::runtime_error(call->getRowCol() + " " + e.what());
    }
}

void CallTreeBuilder::visit(tree::Function* func)
{
    auto fd = fileCtx->getFuncDecl(fileCtx->currentNamespace.toMinecraftPath()+func->getName());

    current_function_name = fd.minecraft_name;

    vars.emplace();

    for ( auto& it : func->args )
    {
        visit(it.get());
    }

    func->body->accept(this);

    vars.pop();
}

void CallTreeBuilder::visit(tree::InlinedFunction* func)
{
    auto fd = fileCtx->getInlinedDecl(fileCtx->currentNamespace.toMinecraftPath()+func->getName());

    current_function_name = fd.minecraft_name;

    vars.emplace();

    for ( auto& it : func->args )
    {
        visit(it.get());
    }

    func->body->accept(this);

    vars.pop();
}

void CallTreeBuilder::visit(tree::Block* block){
    for ( auto& it : block->content )
    {
        if ( it ) it->accept(this);
    }
}

void CallTreeBuilder::visit(tree::If* if_){
    vars.emplace();

    for ( auto& it : if_->elif )
    {
        it.first->accept(this);
        it.second->accept(this);
    }

    if ( if_->else_ )
    {
        if_->else_->accept(this);
    }

    vars.pop();
}

void CallTreeBuilder::visit(tree::While* while_){
    vars.emplace();

    while_->expr->accept(this);
    while_->body->accept(this);

    vars.pop();
}

void CallTreeBuilder::visit(tree::ScheduledWhile* while_){
    vars.emplace();


    while_->expr->accept(this);
    while_->body->accept(this);

    if ( while_->then ) while_->then->accept(this);

    vars.pop();
}

std::vector<std::string> CallTreeBuilder::findRecursion()
{
    constexpr char GRAY = 0;
    constexpr char BLACK = 1;
    
    std::vector<std::string> res;
    std::unordered_map<std::string,char> colors;

    std::function<bool(const std::string&,std::vector<std::string>&path)> dfs = [&res,&dfs,this,&colors](const std::string& current,std::vector<std::string>& path) -> bool
    {
        colors[current] = GRAY;

        path.emplace_back(current);

        for ( auto& child : graph[current] )
        {
            if ( !colors.contains(child) )
            {
                if (dfs(child,path))
                    continue;
            }
            else if ( colors[child] == GRAY )
            {
                std::string pathstr;
                for ( auto & it : path )
                {
                    pathstr.append(it).append(" -> ");
                }
                pathstr.append(child);
                res.emplace_back(std::move(pathstr));
                return false;
            }
        }
        colors[current] = BLACK;
        return true;
    };


    for ( auto& [root,_] : graph )
    {
        if ( colors.contains(root) )
        {
            continue;
        }

        std::vector<std::string> path;
        dfs(root,path);
    }

    return res;

}