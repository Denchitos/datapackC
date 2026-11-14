#pragma once
#include "empty_visitor.h"

#include "tree/block.h"
#include "tree/function.h"
#include "tree/if.h"
#include "tree/while.h"
#include "tree/return.h"

class ReturnChecker : public EmptyVisitor
{
    bool isAlwaysReturn = false;

    bool alwaysReturn(tree::Node* n)
    {
        isAlwaysReturn = false;
        if ( n ) n->accept(this);
        return isAlwaysReturn;
    }

public:
    void visit(tree::Function* func) override
    {
        func->alwaysReturn = alwaysReturn(func->body.get());
    }
    void visit(tree::Block*block) override
    {
        isAlwaysReturn = false;
        for ( auto& it : block->content )
        {
            if ( !alwaysReturn(it.get()) ) continue;
            isAlwaysReturn = true;
            break;
        }
    }
    void visit(tree::If* if_) override
    {
        isAlwaysReturn = false;

        if (if_->else_ == nullptr)
        {
            return;
        }

        for ( auto& it : if_->elif )
        {
            if ( !alwaysReturn(it.second.get()) )
            {
                isAlwaysReturn = false;
                return;
            }
        }

        isAlwaysReturn = alwaysReturn(if_->else_.get());
    }

    void visit(tree::Return*)
    {
        isAlwaysReturn = true;
    }
};
