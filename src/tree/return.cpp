#include "return.h"
#include "../visitor.h"

void tree::Return::accept(Visitor* v)
{
    v->visit(this);
}
