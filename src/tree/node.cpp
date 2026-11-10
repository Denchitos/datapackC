#include "node.h"

#include<iostream>
#include<iomanip>
#include"../visitor.h"
tree::Node::Node(const Token& tok): col(tok.col),row(tok.row),token(tok.tok)
{

}

void tree::Node::accept(Visitor*)
{
    std::cout<<"["<<row<<","<<col<<"] - " << token << " - Узел не поддерживается\n";
}