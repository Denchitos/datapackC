#include"token.h"

Token::Token()
{

}

Token::Token(std::string&& s,Type type):tok(std::move(s)),type(type)
{

}

Token::Token(std::string&& s,size_t col,size_t row,Type type,bool space_before): tok(std::move(s)), col(col),row(row),type(type),space_before(space_before)
{

}

std::string Token::typeToString(Type t)
{
    if ( type_strings.contains(t) ) return type_strings[t];
    else return std::string(1,(int)t);
}

std::unordered_map<Token::Type,std::string> Token::type_strings = {
    { EL,"EOL" },
    { Word,"WORD" },
    { Literal,"Literal" },
    { EF,"Out Of Bounds" },
    { OParen, "(" },
    { CParen, ")" },
    { OBrace, "{" },
    { CBrace, "}" },
    { Colon, ":" },
    { Comma, "," },
    { Dot, "." },
    { Dollar, "$" },
};
