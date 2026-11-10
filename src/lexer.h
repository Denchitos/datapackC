#pragma once
#include<string>
#include<optional>
#include<vector>
#include"token.h"

class Lexer
{
    std::string text;
    std::string current;
    std::vector<Token> res;
    bool space_before = false;
    long long i = 0;
    size_t col = 1;
    size_t row = 1;
    long long len = 0;
    Token::Type type;

    
    void flush(size_t col,size_t row);
    void reset();
    void next();

    void parseEOL();
    void parseNumber();
    void parseOperator();
    void parseLiteral();
    void parseWord();
    void parseParenthes();
    void parseComment();
    void parseSpecial();

    bool isParenthesis(char);
    bool isOperatorSym(char);
    bool isSpecialSym(char);
    bool isOperator(const std::string&);

    char getSymbol();
public:
    void setText(const std::string& text );
    void setText(std::string&& text );
    std::vector<Token> lex();
};