#include"objective.h"
#include"../visitor.h"
tree::Objective::Objective(const Token& tok) : Node(tok){

}

TREE_ACCEPT_MACRO(Objective);

