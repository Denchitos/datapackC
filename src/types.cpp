#include<unordered_map>
#include"types.h"
#include<algorithm>
std::optional<Type> Type::StrToType(const std::string& s)
{
    if ( !Type::hasType(s) ) return std::nullopt;

    if ( !s.contains('[') ) return str_to_kind[s];

    auto p = s.find('[');

    Type type( str_to_kind[s.substr(0,p)] );

    for ( size_t i = p ; i < s.length() ;  i++)
    {
        if (s[i] == ']') type.list_depth++;
    }

    return type;

};
std::unordered_map<std::string,Type> Type::str_to_kind
{
    { "auto",Type::auto_t },
    { "score",Type::score },
    { "score_bool",Type::score_bool },
    { "selector",Type::selector },
    { "objective",Type::objective },
    { "rawSelector",Type::rawSelector },
    { "numberLiteral",Type::numberLiteral },
    { "boolLiteral",Type::boolLiteral },
    { "textLiteral",Type::textLiteral },
    { "int",Type::int_t},
    { "bool",Type::bool_t},
    { "float",Type::float_t},
    { "string",Type::string},
    { "void",Type::void_t },
    { "initializer_list",Type::initializer_list },
};

std::unordered_map<Type,std::string> Type::type_to_str{
    { Type::auto_t,"auto" },
    { Type::score,"score" },
    { Type::score_bool,"score_bool" },
    { Type::selector,"selector" },
    { Type::objective,"objective" },
    { Type::rawSelector,"rawSelector" },
    { Type::numberLiteral,"numberLiteral" },
    { Type::boolLiteral,"boolLiteral" },
    { Type::textLiteral,"textLiteral" },
    { Type::int_t,"int" },
    { Type::bool_t,"bool" },
    { Type::float_t,"float" },
    { Type::string,"string" },
    { Type::void_t,"void" },
    { Type::initializer_list,"initializer_list" },
};

std::size_t std::hash<Type>::operator()(const Type& t) const
{
    return t.type^(t.struct_id << 2)^(t.list_depth<<4);
}

//Потом добавить std::expected
std::optional<std::string> Type::toStr(const Type& t)
{
    if ( !type_to_str.contains({t.type}) ) return std::nullopt;
    if ( t.list_depth == 0 && type_to_str.contains(t.type) ) return type_to_str[t.type];
    auto res = type_to_str[{t.type}];
    for ( size_t i = 0 ; i < t.list_depth ; i++ )
    {
        res+="[]";
    }
    return res;
}