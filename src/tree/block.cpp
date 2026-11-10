#include"block.h"
#include"../visitor.h"
tree::Block::Block(const Token& tok): Node(tok)
{
    
}

void tree::Block::push(NodePtr&& node)
{
    content.emplace_back(std::move(node));
}

void tree::Block::accept(Visitor* v)
{
    v->visit(this);
}