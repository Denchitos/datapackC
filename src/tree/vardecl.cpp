#include"vardecl.h"
#include"../visitor.h"

TREE_BEGIN

VarDecl::VarDecl(const Token& t): Node(t)
{

}

void VarDecl::setMut(bool t){ mut = t; };
bool VarDecl::isMut()
{
    return mut;
}

void VarDecl::setVarName(const std::string& t)
{
    var_name = t;    
}

void VarDecl::setVarType(const std::string& t)
{
    type = t;



}

const std::string&  VarDecl::getVarName()
{
    return var_name;
}

const std::string& VarDecl::getVarType()
{
    return type;
}








TREE_ACCEPT_MACRO(VarDecl);





TREE_END