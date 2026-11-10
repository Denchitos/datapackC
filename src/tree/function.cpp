#include"function.h"
#include"../visitor.h"
tree::Function::Function(const Token& tok): Node(tok)
{

};

void tree::Function::accept(Visitor* v)
{
    v->visit(this);
}

void tree::Function::pushArg(VarDeclPtr&& arg)
{
    args.emplace_back(std::move(arg));
}