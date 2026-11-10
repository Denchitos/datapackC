#include"namespace.h"
#include"../visitor.h"
tree::Namespace::Namespace(const Token& tok): Node(tok)
{

}

TREE_ACCEPT_MACRO(Namespace)