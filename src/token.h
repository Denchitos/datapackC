#pragma once
#include<string>
#include<unordered_map>
struct Token
{
public:
    enum Type
    {
        UNDEFINED,
        EF,
        Number,
        Literal,
        Word,
        Operator,
        EL,
        OAngle, // <
        CAngle, // >
        OParen, // (
        CParen, // )
        OBracket, // [
        CBracket, // ]
        OBrace, // {
        CBrace, // }
        Comma,
        Colon,
        At,
        Dot,
        Dollar,
    };
    std::string tok;
    size_t col = 0;
    size_t row = 0;
    Type type = UNDEFINED;
    bool space_before = false;
    Token();

    Token(std::string&& s,Type type);

    Token(std::string&& s,size_t col,size_t row,Type type,bool space_before = false);
    //RETURN THE True Token ( for example EL has tok [EndOfLine], but it is [\n] and Literal has tok attribute [Hello world] , but it means ["Hello World"] 
    std::string toString() const {
        switch (type){
            case EL: return "\n";
            case Literal: return "\""+tok+"\"";
            default: return tok;
        }
    }

    std::string trueString() const 
    {
        switch (type){
            case EL: return "\n"; 
            case Literal: return "\""+tok+"\"";
            default: return tok;
        }
    }

    bool operator==(const std::string& rhs) const
    {
        return tok == rhs;
    }

    bool operator==(const Type rhs) const
    {
        return type == rhs;
    }

    bool operator==(const Token& rhs) const
    {
        return tok==rhs.tok && type == rhs.type;
    }

    static std::unordered_map<Type,std::string> type_strings;
    static std::string typeToString(Type);
};