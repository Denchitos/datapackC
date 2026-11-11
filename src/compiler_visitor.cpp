#include<iostream>
#include<iomanip>
#include<expected>
#include<unordered_set>
#include"compiler_visitor.h"
#include"tree/node.h"
#include"tree/block.h"
#include"tree/function.h"
#include"tree/raw.h"
#include"tree/expr.h"
#include"tree/vardecl.h"
#include"tree/raw_selector.h"
#include"tree/namespace.h"
#include"tree/member_access.h"
#include"tree/objective.h"
#include"tree/If.h"
#include"tree/while.h"
#include"tree/call.h"
#include"tree/return.h"
#include"types.h"

CompilerVisitor::CompilerVisitor(std::shared_ptr<FileCompilationContext> ctx,std::shared_ptr<ExprCodeGen> codeG) : fileCtx(ctx),codeGen(codeG)
{

}

Type CompilerVisitor::typeInferrence(const Value& rhs)
{
    Type type{Type::auto_t}; 
    if (rhs.type.type!=Type::numberLiteral && rhs.type.type!=Type::boolLiteral && rhs.type.type != Type::textLiteral && rhs.type.type != Type::rawSelector && type != Type::initializer_list )
    {
        type = rhs.type; 
    }
    else if ( rhs.type.type==Type::numberLiteral )
    {
        if ( rhs.value[0].find('.') != std::string::npos ) type.type = Type::float_t;
        type.type = Type::score;
    }
    else if ( rhs.type.type==Type::boolLiteral )
    {
        type.type = Type::score_bool;
    }
    else if ( rhs.type.type==Type::rawSelector )
    {
        type.type = Type::selector;
    }
    else if ( rhs.type.type==Type::textLiteral )
    {
        type.type = Type::string;
    }
    else if ( rhs.type.type == Type::initializer_list )
    {
        throw std::runtime_error("Can not to infer type of initializer list");
    } 
    return type;
}

static std::string getRowCol( tree::Node* node ) 
{
    return "[ " + std::to_string(node->getRow()) + " , " + std::to_string(node->getColumn()) + " ]";
} 

//RETURN LAST TWO OPERANDS [ PRE-LAST , LAST ] AND REMOVES IT FORM STACK

std::pair<Value,Value> CompilerVisitor::getTwoOperands()
{   
    if ( op_stack.size() < 2 ) throw std::runtime_error("op_stack has less than 2 elements when called getTwoOperands()");
    auto rhs = std::move(op_stack.top());
    op_stack.pop();
    auto lhs = std::move(op_stack.top());
    op_stack.pop();
    return {lhs,rhs};
};

Value CompilerVisitor::getTopOperand()
{   
    if ( op_stack.size() == 0 ) throw std::runtime_error("op_stack is empty");
    auto rhs = std::move(op_stack.top());
    op_stack.pop();
    return rhs;
};

void CompilerVisitor::visit(tree::Namespace* nmsp)
{
    fileCtx->currentNamespace = { nmsp->namespace_,nmsp->path };
}

void CompilerVisitor::visit(tree::Function* func)
{
    fileCtx->newFunction(func->getName());

    for (auto& it : func->args)
    {
        if (it) it->accept(this);
        else error("NULLPTR IN FUNCTION ARGS");
    }

    if (func) func->body->accept(this);
    
    else error("NULLPTR FUNCTION BODY");
    
    compiled_functions.emplace_back(fileCtx->currentFunction());

    fileCtx->upFunction();
}

void CompilerVisitor::visit(tree::VarDecl* vardecl)
{
    //std::cout<<"VARDECL\n";
    bool isMut = vardecl->isMut();
    auto name = vardecl->getVarName();
    auto list_depth = vardecl->list_depth;

    //if ( list_depth > 0 ) std::cout<<"HERE "<<name<<" ["<<list_depth<<"]\n";

    auto type_name = vardecl->getVarType();
    
    if ( !fileCtx->hasType(type_name) )
    {
        throw std::runtime_error("Unknown type : '"+ type_name + "' on line:" + std::to_string(vardecl->getRow()) + " in col: "+ std::to_string(vardecl->getColumn()) );
    }

    Type type = fileCtx->strToType(type_name);
    
    type.list_depth = list_depth;
    
    std::vector<std::string> value{name};

    if ( !vardecl->expr ){
        if (type.type==Type::auto_t)
        {
            throw std::runtime_error("Здесь невозможен автоматический вывод типа");
        }
        value = fileCtx->nameMangle(type,name);
        fileCtx->registerVar(name,{value,type,isMut});
    } else {
    vardecl->expr->accept(this);

    auto rhs = std::move(op_stack.top());
    op_stack.pop();

    if ( type.type == Type::auto_t ){
        type = typeInferrence(rhs);
    }

    value = fileCtx->nameMangle(type,name);
    Value lhs{ value,type,name };
    
    fileCtx->registerVar(name,{value,type,isMut});
    
    try { codeGen->genBinaryOperator("=",lhs,rhs); }
    catch( std::exception& e )
    {
        throw std::runtime_error( getRowCol(vardecl) + e.what() );
    }
    
    }
}

void CompilerVisitor::visit(tree::StorageDecl* storage_decl)
{
    if  ( storage_decl->methods.empty() ) return;
    else
    {
        fileCtx->currentNamespace.push( storage_decl->getName() );

        current_storage = &(fileCtx->getStorageDecl(storage_decl->getName()));

        for ( auto & method : storage_decl->methods )
        {
            method->accept(this);
        }
        
        current_storage = nullptr;
        
        fileCtx->currentNamespace.pop();
    }
}

void CompilerVisitor::visit(tree::Block* b)
{
    //std::cout<<"BLOCK\n";
    for ( auto& it : b->content )
    {
        if (it) it->accept(this);
        op_stack = {};
    } 
}

void CompilerVisitor::visit(tree::MemberAccess* mem)
{
    auto size = mem->members.size();
    auto& members = mem->members;
    size_t i = 0;
    std::vector<std::string> val;
    Value value(val,{});
    while ( i < size  )
    {
        if ( auto var = exprCast<tree::Variable>( members[i].get() ) ) genStructMemberAccess(var,value,i);
        else if ( auto call = exprCast<tree::Call>( members[i].get() ) ) genCallMemberAccess(call,value,i);
        else throw std::runtime_error(getRowCol(mem->members[i].get()) +" Unexpected Situation in member access");
    }
    op_stack.emplace(value);
} 

void CompilerVisitor::genStructMemberAccess(tree::Variable* var,Value& value,size_t& i)
{
    if ( value.value.size() == 0 )
    {
        auto & sym = fileCtx->getSymbol(var->getName());
        value.type = sym.type;
        value.value = sym.value;
        i++;
    }
    else if ( value.type == Type::rawSelector || value.type == Type::selector )
    {
        i++;
        try {
            auto & sym = fileCtx->getSymbol(var->getName());
            if ( sym.type != Type::objective )
                throw std::runtime_error("After selector expected a function call or objective, but got "+ fileCtx->typeToStr(value.type));
            value.value = { value.to_string() , sym.value[0] };
            value.type = Type(Type::score);
        }
        catch( std::exception& e )
        {
        throw std::runtime_error( getRowCol(var) + e.what() );
        }
    }
    else
    {
        auto& field = var->getName();
        i++;
        if ( Type::isBuiltinType(value.type) ) throw std::runtime_error(  getRowCol(var) + " Builtin type " + fileCtx->typeToStr(value.type) + " has no field '"+field + "'" );
        if ( !fileCtx->hasStorage(value.type) ) throw std::runtime_error(getRowCol(var) + " Unknown storage" );
        auto& fields = fileCtx->getStorageDecl(value.type).fields;
        auto it = fields.find(field);
        if ( it == fields.end() ) throw std::runtime_error(getRowCol(var) + " Unknown member '"+ field + "' of storage '"+ fileCtx->typeToStr(value.type) +"'");

        value.value.back().append(".").append(field);
        value.type = it->second;
    }
}

void CompilerVisitor::genCallMemberAccess(tree::Call* call, Value& value,size_t &i)
{
    i++;
    if ( value.value.size() == 0 )
    {
        call->accept(this);
        value = getTopOperand();
    }
    else if ( value.type.isList() )
    {
        if ( call->getName() == "operator[]" )
        {
            auto len = call->call_args.size();
            if ( len != 1 ) throw std::runtime_error(getRowCol(call) + " index access for builtin list has only one number literal argument, but got " + std::to_string(len) );

            call->call_args[0]->accept(this);
            auto val = getTopOperand();
            if ( val.type != Type::numberLiteral ) throw std::runtime_error(getRowCol(call) + " index access for builtin list requires one number literal argument, but got " + fileCtx->typeToStr(val.type) );

            value.value.back().append("[").append(std::to_string(std::stoi(val.to_string()))).append("]");
            value.type = value.type.getListDown();
        }
        else if ( call->getName() == "len" )
        {
            auto len = call->call_args.size();
            if ( len != 0 ) throw std::runtime_error(getRowCol(call) + " builtin member function len() hasn't any arguments, but got " + std::to_string(len) );
            auto tmp = fileCtx->getTempScoreVar();
            fileCtx->println( "execute store result score "+tmp[0] + " " + tmp[1] + " run data get " + value.to_string() );
            value.value = tmp; value.type = Type::score;
        }
        else throw std::runtime_error(getRowCol(call) + " unknown member function '" + call->getName() + "' for builtin type " + fileCtx->typeToStr(value.type) );
    }
    else if ( value.type == Type::rawSelector || value.type == Type::selector )
    {
        fileCtx->print( "execute as "+value.to_string()+" at @s run " );
        call->accept(this);
        value = getTopOperand();
    }
    else {
        auto& storage_decl = fileCtx->getStorageDecl(value.type);
        if ( storage_decl.methods.contains(call->getName()) )
        {
            auto method_decl = storage_decl.methods[call->getName()];
            std::vector<Value> call_args{ value };
            for ( auto& arg : call->call_args ){ arg->accept(this) ; call_args.emplace_back(getTopOperand()) ; }
            emitCall(call_args,method_decl);
            value = getTopOperand();
        }
        else throw std::runtime_error( getRowCol(call) +" Unknown member function '" + call->getName() +"'");
    }
}

void CompilerVisitor::genSelectorMemberAccess(tree::RawSelector* call, Value& value,size_t &i)
{
    // if (size==2)
    //     {
    //         auto objective = fileCtx->getSymbol(members[i]);            
    //         if ( objective.type.type == Type::objective )
    //         {
    //             std::vector v = {fileCtx->getSymbol(members[0]).value[0],fileCtx->getSymbol(members[1]).value[0]};
    //             op_stack.emplace(v,Type::score);
    //         }
    //         else
    //         {
    //             throw std::runtime_error( getRowCol(node)+ " " +members[i] + " doesn't names an objective in this scope");
    //         }
    //     }
    //     else
    //     {
    //         throw std::runtime_error( getRowCol(node) +" If you start member accessing with selector expected an objective it");
    //     }
}

void CompilerVisitor::visit(tree::Variable* var)
{
    //std::cout<<"VAR\n";
    auto n = var->getName();
    try {op_stack.emplace(fileCtx->getSymbol(n).value,fileCtx->getSymbol(n).type,n); }
    catch( std::exception& e )
    {
        throw std::runtime_error( getRowCol(var) + e.what() );
    }
}

void CompilerVisitor::visit(tree::RawSelector* sel)
{
    if ( sel->params.empty() ) op_stack.emplace(sel->type,Type::rawSelector);
    else op_stack.emplace(sel->type + '[' + sel->params +']',Type::rawSelector);
}

void CompilerVisitor::visit(tree::NumberLiteral* var)
{
    op_stack.emplace(var->getName(),Type::numberLiteral);
}

void CompilerVisitor::visit(tree::BoolLiteral* lit)
{
    op_stack.emplace(lit->toStr(),Type::boolLiteral);
}

void CompilerVisitor::visit(tree::TextLiteral* lit)
{
    op_stack.emplace(lit->getName(),Type::textLiteral);
}

void CompilerVisitor::visit(tree::BinaryOperator* binop)
{
    //std::cout<<"BINOP\n";
    binop->left->accept(this);
    binop->right->accept(this);

    auto op = binop->getName();

    auto [lhs,rhs] = getTwoOperands();
    try {
        op_stack.emplace(codeGen->genBinaryOperator(op,lhs,rhs));
    }
    catch( std::exception& e )
    {
        throw std::runtime_error( getRowCol(binop) + e.what() );
    }
}

void CompilerVisitor::visit(tree::Objective* obj)
{
    auto name = obj->name;
    auto value = fileCtx->nameMangle(Type::objective,name);
    fileCtx->registerVar(name,{value,Type::objective,true});
    fileCtx->printToLoad("scoreboard objectives add "+ value[0] + " dummy");
}   

void CompilerVisitor::visit(tree::Raw* raw)
{
    std::string a;
    for ( auto& it : raw->raw )
    {
        if ( it.space_before ) a+=' ';
        if ( it.is_literal ) a+='\"';
        switch (it.type){
            case tree::Raw::Word : a+=it.str; break;
            case tree::Raw::Var  : {
                if ( it.id == -1  )
                {
                    for ( auto& str : fileCtx->getSymbol(it.str).value )
                    {
                        a+=str+" ";
                    }
                    a.pop_back(); // Удаляем последний пробел ... не нашёл нормального решение пока
                }
                else
                {
                    a+=fileCtx->getSymbol(it.str).value[it.id];
                } 
                break;
            }
        }
        if ( it.is_literal ) a+='\"'; 
    }
    if ( a[0] != ' ')fileCtx->println(a);
    else fileCtx->println(a.substr(1,a.length()-1));
}

void CompilerVisitor::visit(tree::If* If)
{
    auto newName = "__cb__"+std::to_string(fileCtx->currentFunction().control_block_counter++);
    
    auto retCond = fileCtx->getReturnConditionValue(); // for return in if-statement
    
    fileCtx->extendFunction(newName);
    
    auto cb_name = fileCtx->currentFunction().toMinecraftPath();
    size_t i = 0;
    
    for ( auto& it : If->elif )
    {
        it.first->accept(this);

        auto res = getTopOperand().value;

        fileCtx->extendFunction("__"+std::to_string(i++));
        
        auto body_name = fileCtx->currentFunction().toMinecraftPath();
        
        it.second->accept(this);
        compiled_functions.emplace_back(fileCtx->currentFunction());
        fileCtx->upFunction();

        fileCtx->println( "execute unless score "+res[0] + " " + res[1]+" matches 0 run return run function "+body_name );
    }
    if ( If->else_ )
    {
        fileCtx->extendFunction("__"+std::to_string(i));
        auto body_name = fileCtx->currentFunction().toMinecraftPath();
        
        If->else_->accept(this);

        compiled_functions.emplace_back(fileCtx->currentFunction());
        fileCtx->upFunction();
        
        fileCtx->println("function "+ body_name);
    }

    compiled_functions.emplace_back(fileCtx->currentFunction());
    fileCtx->upFunction();

    codeGen->genAssign(retCond,{"0",Type::numberLiteral}); // for return in while

    fileCtx->println("function "+ cb_name);
    
    fileCtx->println("execute unless score "+retCond.to_string()+" matches 0 run return 0" ); // for return in while

    if ( !fileCtx->while_break_stack.empty() )
    {
        fileCtx->println( "execute unless score "+ fileCtx->while_break_stack.top() + " local matches 0 run return 0" );
    }

}

void CompilerVisitor::visit(tree::Break*)
{
    fileCtx->println("return run scoreboard players set "+ fileCtx->while_break_stack.top() + " local 1");
}

void CompilerVisitor::visit(tree::While* While)
{
    auto newName = "__cb__"+std::to_string(fileCtx->currentFunction().control_block_counter++);
    
    auto retCond = fileCtx->getReturnConditionValue(); // for return in while
    
    fileCtx->extendFunction(newName);
    auto cb_name = fileCtx->currentFunction().toMinecraftPath();
    fileCtx->while_break_stack.push('.'+fileCtx->currentFunction().toDotPath()+".__break");

    While->expr->accept(this);
    auto res = getTopOperand().value;

    fileCtx->extendFunction("__b");
    auto body_name = fileCtx->currentFunction().toMinecraftPath();
    
    While->body->accept(this);
    compiled_functions.emplace_back(fileCtx->currentFunction());
    fileCtx->upFunction();

    fileCtx->println( "execute unless score "+res[0] + " " + res[1]+" matches 0 run function "+body_name );
    fileCtx->println( "execute if score " + retCond.to_string() + " matches 0 unless score "+res[0] + " " + res[1]+" matches 0 if score "+ fileCtx->while_break_stack.top() + " local matches 0 run function "+cb_name );
    compiled_functions.emplace_back(fileCtx->currentFunction());
    fileCtx->upFunction();

    codeGen->genAssign(retCond,{"0",Type::numberLiteral}); // for return in while
    
    fileCtx->println("scoreboard players set "+ fileCtx->while_break_stack.top() + " local 0");
    fileCtx->while_break_stack.pop();
    fileCtx->println("function "+ cb_name);
    
    fileCtx->println("execute unless score "+retCond.to_string()+" matches 0 run return 0" ); // for return in while
}

void CompilerVisitor::visit(tree::ScheduledWhile* While)
{
    auto newName = "__cb__"+std::to_string(fileCtx->currentFunction().control_block_counter++);
    fileCtx->extendFunction(newName);
    auto cb_name = fileCtx->currentFunction().toMinecraftPath();
    fileCtx->while_break_stack.push('.'+fileCtx->currentFunction().toDotPath()+".__break");

    While->expr->accept(this);
    auto res = getTopOperand().value;

    fileCtx->extendFunction("__b");
    auto body_name = fileCtx->currentFunction().toMinecraftPath();
    
    While->body->accept(this);
    compiled_functions.emplace_back(fileCtx->currentFunction());
    fileCtx->upFunction();

    fileCtx->println( "execute unless score "+res[0] + " " + res[1]+" matches 0 run function "+body_name );
    fileCtx->println( "execute unless score "+res[0] + " " + res[1]+" matches 0 if score "+ fileCtx->while_break_stack.top() + " local matches 0 run return run schedule function "+cb_name + " " + std::to_string(While->schedule) + "t" );
    
    fileCtx->extendFunction("__then");
    auto then_name = fileCtx->currentFunction().toMinecraftPath();
    if ( While->then ) While->then->accept(this);

    compiled_functions.emplace_back(fileCtx->currentFunction());
    fileCtx->upFunction();
    fileCtx->println("function "+ then_name);
    compiled_functions.emplace_back(fileCtx->currentFunction());
    fileCtx->upFunction();
    fileCtx->println("scoreboard players set "+ fileCtx->while_break_stack.top() + " local 0");
    fileCtx->while_break_stack.pop();
    fileCtx->println("function "+ cb_name);
}

void CompilerVisitor::emitCall(const std::vector<Value>& call_args,FunctionDeclaration& fd)
{
    auto got_args_num = call_args.size();
    auto expected_args_num = fd.args.size();
    if ( got_args_num != expected_args_num )
    {
        throw std::runtime_error("Function exepcted "+std::to_string(expected_args_num)+" ,but got "+std::to_string(got_args_num));
    }

    size_t i = 0;
    
    for ( auto& arg : call_args )
    {
        codeGen->genAssign( fd.args[i++] ,arg );
    }

    fileCtx->println("function "+fd.minecraft_name);

    if ( fd.ret.has_value() )
    {
        op_stack.push(fd.ret.value());
    }
    else
    {
        op_stack.emplace(Type::void_t);
    }
}

void CompilerVisitor::visit(tree::Call* call)
{
    auto& name = call->getName();
    if ( !fileCtx->hasFunc(name) )
    {
        throw std::runtime_error( getRowCol(call) + " Unknown name to call :" + name);
    }
    std::vector<Value> call_args;

    for ( auto& arg : call->call_args )
    {
        arg->accept(this);
        call_args.emplace_back(getTopOperand());
    }

    try {
        emitCall(call_args,fileCtx->getFuncDecl(name));
    }
    catch ( std::exception& e )
    {
        throw std::runtime_error(getRowCol(call) + " " + e.what());
    }
}

void CompilerVisitor::visit(tree::Return* ret)
{
    auto name = fileCtx->realFunction().getName();

    FunctionDeclaration funcDecl; 
    if ( current_storage==nullptr ) funcDecl = fileCtx->getFuncDecl(name);
    else funcDecl = current_storage->methods[fileCtx->realFunction().getName()];

    if ( funcDecl.ret.has_value() )
    {
        if ( ret->expr == nullptr ) throw std::runtime_error("In function that returns value, return statement exists without returning expression");
        auto retval = funcDecl.ret.value();
        ret->expr->accept(this);
        auto expr = getTopOperand();
        try { codeGen->genAssign( retval ,expr ); }
        catch ( std::exception& e ) { throw std::runtime_error(getRowCol(ret) + " " + e.what()); }
    }
    else
    {
        if ( ret->expr != nullptr ) throw std::runtime_error("In function that doesn't return value, return statement exists that returns expression");
    }

    if ( (&fileCtx->realFunction()) != (&fileCtx->currentFunction()) )
    {
        auto v = fileCtx->getReturnConditionValue();
        codeGen->genAssign(v,{"1",Type::numberLiteral});
    }
    fileCtx->println("return 0");
}