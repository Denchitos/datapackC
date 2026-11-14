#include"node.h"

TREE_BEGIN

struct While : public Node
{
    NodePtr expr;
    NodePtr body;
    bool isBreakable : 1 = true;
    bool alwaysReturn : 1 = false;   
    While(const Token& tok);
    void accept(Visitor* v);
};

struct ScheduledWhile : public While
{
    unsigned short schedule = 0;
    NodePtr then;
    ScheduledWhile(const Token& tok) : While(tok) {};
    void accept(Visitor* v);
};

struct Break : public Node
{
    Break(const Token& tok);
    void accept(Visitor* v);
};

TREE_END