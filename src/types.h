#pragma once
#include<unordered_map>
#include<string>
#include<optional>
#include <iostream>
struct Type;

namespace std {
template<>
struct hash<Type> {
    std::size_t operator()(const Type& t) const;
};
}

struct Type
{
    enum Kind : int
    {
        void_t,
        auto_t,score,score_bool,selector,objective,
        numberLiteral,boolLiteral,rawSelector,textLiteral,
        int_t,float_t,bool_t,string,
        struct_t,
        initializer_list
    };
    
    Kind type = Type::auto_t;
    int struct_id = 0;
    size_t list_depth = 0;

    Type() = default;
    Type(Kind kind) : type(kind) {}
    Type(Kind kind,int struct_id) : type(kind),struct_id(struct_id){}

    static std::optional<Type> StrToType(const std::string&);
    
    static std::optional<std::string> toStr(const Type& t);
    
    bool operator==(const Type&t) const
    {
        return type == t.type && struct_id == t.struct_id && list_depth == t.list_depth ;
    }

    bool isList() const { return list_depth > 0 ;}

    Type getListDown() const
    {
        if (list_depth == 0) return *this;
        
        auto t2 = *this;
        --t2.list_depth;
        return t2;
    } 
    static bool hasType(const std::string& type )
    {
        if ( type.contains('[') ) return str_to_kind.contains(type.substr(0,type.find('[')));
        return str_to_kind.contains(type);
    }
    static bool isBuiltinType(const Type& type )
    {
        return type_to_str.contains(type);
    }
private:
    static std::unordered_map<std::string,Type> str_to_kind;
    static std::unordered_map<Type,std::string> type_to_str;

};

namespace std {
template<>
struct hash<std::pair<Type,Type>>
{
    std::size_t operator()(const std::pair<Type,Type>& t) const
    {
        std::size_t h1 = std::hash<Type>{}(t.first);
        std::size_t h2 = std::hash<Type>{}(t.second);
        return  h2^(h1>>1)^(h2<<6);
    }
};
}