#include "node.h"
#include"expr.h"
#include"../visitor.h"

TREE_BEGIN

TREE_ACCEPT_MACRO(BinaryOperator)

TREE_ACCEPT_MACRO(Variable)

TREE_ACCEPT_MACRO(NumberLiteral)

long long NumberLiteral::toInteger()
{
    return std::stoll(getName());
}

double NumberLiteral::toDouble()
{
    return std::stod(getName());
}

TREE_ACCEPT_MACRO(TextLiteral)

TREE_ACCEPT_MACRO(InitList)

TREE_ACCEPT_MACRO(BoolLiteral)
std::string BoolLiteral::toStr()
{
    return ( val ? "true" : "false" );
}

TREE_END