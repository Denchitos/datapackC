#include"while.h"
#include"../visitor.h"

NODE_CONSTRUCTOR(While)
TREE_ACCEPT_MACRO(While);

TREE_ACCEPT_MACRO(ScheduledWhile);

NODE_CONSTRUCTOR(Break)
TREE_ACCEPT_MACRO(Break);