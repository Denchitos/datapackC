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
        if ( !n) return false;
        if (!dynamic_cast<tree::Function*>(n) && !dynamic_cast<tree::Block*>(n) && !dynamic_cast<tree::Return*>(n) && !dynamic_cast<tree::If*>(n) && !dynamic_cast<tree::While*>(n)) return false;
        isAlwaysReturn = false;
        n->accept(this);
        std::cout<<"Test is : "<<(isAlwaysReturn ? "true" : "false")<<"\n";
        return isAlwaysReturn;
    }

public:
    void visit(tree::Function* func) override
    {
        std::cout<<"Test always return function "<<func->getName()<<"\n";
        func->alwaysReturn = alwaysReturn(func->body.get());
    }
    void visit(tree::Block*block) override
    {
        isAlwaysReturn = false;
        std::cout<<"Test return block\n";
        for ( auto& it : block->content )
        {
            if ( !alwaysReturn(it.get()) ) continue;
            isAlwaysReturn = true;
            break;
        }
    }
    void visit(tree::If* if_) override
    {
        std::cout<<"Test if statement\n";
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
