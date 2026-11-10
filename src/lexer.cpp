#include"lexer.h"
#include<iostream>


void Lexer::setText(const std::string& s)
{
    text = s;
    len = static_cast<long long>(text.length());
}
void Lexer::setText(std::string&& s)
{
    text= std::move(s);
    len = static_cast<long long>(text.length());
}

char Lexer::getSymbol()
{
    if ( i<len ) return text[i];
    return '\0';
}

void Lexer::flush(size_t c,size_t r)
{
    res.emplace_back(std::move(current),c,r,type,space_before);
    current.clear();
    space_before = false;
}

void Lexer::next()
{
    i++;
    col++;
};

void Lexer::reset()
{
    i = 0;
    text.clear();
    current.clear();
    res.clear();
    row = 1;
    col = 1;
    len = 0;
}

void Lexer::parseNumber()
{
    //std::cout<<"число\n";
    type = Token::Number;
    bool find_point = false;
    size_t c = col;
    while ( i<len )
    {
        char c = getSymbol();
        if ( !( (c<='9' && c>='0') || c=='.') ) break;
        else if ( c=='.' && !find_point) find_point = true;
        else if ( c=='.' && find_point ) break;
        current+=c;
        next();
    }
    flush(c,row);
};

void Lexer::parseOperator()
{
    //std::cout<<"оператор";
    type = Token::Operator;
    size_t c = col;
    while ( i<len )
    {
        char c = getSymbol();
        if (!isOperator(current+c)) break;
        current+=c;
        next();
    }
    flush(c,row);
}

void Lexer::parseEOL()
{
    //std::cout<<"КОНЕЦ СТРОКИ\n";
    size_t c=col,r=row;
    if ( !res.empty() && res.back().type!=Token::EL ){
        type = Token::EL;
        current = "EOL";
        flush(c,r);
    }
    col = 1;
    while ( i<len && getSymbol()=='\n')
    {
        i++;
        row++;
    }
}

void Lexer::parseParenthes()
{
    //std::cout<<"Скобки\n";
    auto c = getSymbol();
    switch (c)
    {
        case '(': type = Token::OParen; break;
        case ')': type = Token::CParen; break;
        case '<': type = Token::OAngle; break;
        case '>': type = Token::CAngle; break;
        case '[': type = Token::OBracket; break;
        case ']': type = Token::CBracket; break;
        case '{': type = Token::OBrace; break;
        case '}': type = Token::CBrace; break;
    }
    current = c;
    flush(col,row);
    next();
}

void Lexer::parseWord()
{
    //std::cout<<"слово\n";
    
    type = Token::Word;
    
    size_t column = col;

    while ( i<len )
    {
        char c = getSymbol();
        if ( c==' ' ) break;
        else if ( c=='\n' || isParenthesis(c) || isOperatorSym(c) || isSpecialSym(c) ){ break; }
        current+=c;
        next();
    }

    flush(column,row);
}

void Lexer::parseLiteral()
{
    //std::cout<<"литерал\n";
    type=Token::Literal;
    size_t c = col;
    next();
    bool closed = false;
    while ( i<len )
    {
        char c = getSymbol();
        if ( c=='\"' ){
            auto length = current.length();
            bool escaped = false;
            if ( length!=0 && current.back() == '\\')
                escaped = ( current.length()==1 ) || /* Здесь length = 2  */ ( current[length-2]!='\\' );
            if ( !escaped ) { closed = true; next(); break;}
        }
        else if ( c=='\n' )
        {
            break;
        }
        current+=c;
        next();
    }
    if (!closed)
    {
        std::cout<<"Незакончен литерал\n";
    }
    flush(c,row);
};

void Lexer::parseComment()
{
    //std::cout<<"комментарий\n";
    while ( i<len && getSymbol() != '\n' ) next();
}

void Lexer::parseSpecial()
{
    //std::cout<<"Спецсимвол\n";
    auto c = getSymbol();
    switch (c)
    {
        case ',': type = Token::Comma; break;
        case ':': type = Token::Colon; break;
        case '@': type = Token::At; break;
        case '.': type = Token::Dot; break;
    }
    current = c;
    flush(col,row);
    next();
}

std::vector<Token> Lexer::lex()
{
    while (i<len)
    {
        //std::cout<<"парсинг...->";
        auto c = getSymbol();
        if (c=='\n') parseEOL();
        else if (c=='#') parseComment();
        else if ( c== ' ' || c=='\t' ) { space_before = true;/*std::cout<<"\n"*/;next();}
        else if ( c== ',' || c == ':' || c=='@' || c=='.') parseSpecial();
        else if (c == '\"') parseLiteral();
        else if ( isOperatorSym(c)) parseOperator();
        else if ( c>='0' && c<='9' ) parseNumber();
        else if ( isParenthesis(c) ) parseParenthes();
        else parseWord();
    }
    auto tmp = std::move(res);
    reset();
    return tmp;
}

bool Lexer::isParenthesis(char c)
{
    return c=='{' || c == '}' || c=='[' || c==']' || c=='(' || c == ')';
}

bool Lexer::isOperatorSym(char c)
{
    return c=='+' || c=='/' || c=='-' || c=='*' || c=='=' || c=='!' || c=='&' || c=='|' || c=='%' || c=='>' || c=='<';
}

bool Lexer::isOperator(const std::string& s)
{
    return
    s=="=="||
    s=="*" ||
    s=="/" ||
    s=="+" ||
    s=="-" ||
    s=="&"||
    s=="|"||
    s=="&&" ||
    s== "++"||
    s=="--" ||
    s == "||" ||
    s == "=" ||
    s == "*=" ||
    s == "-=" ||
    s == "+=" ||
    s == "/=" ||
    s == "%" ||
    s == "%=" ||
    s == "!" ||
    s == "!="||
    s=="<"||s==">"||
    s==">=" || s=="<=" ||
    s == "->";
}

bool Lexer::isSpecialSym(char c)
{
    return c == '#' || c=='\"' || c==':' || c== ',' || c=='@' || c=='.';
}