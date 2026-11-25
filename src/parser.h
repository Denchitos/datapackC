#pragma once
#include<iostream>
#include<vector>
#include<string>
#include<expected>
#include<optional>

#include<filesystem>
namespace fs = std::filesystem;

#include"parser_base.h"

#include"token.h"
#include"tree/node.h"
#include"tree/block.h"
#include"tree/function.h"
#include"tree/raw.h"
#include"tree/expr.h"
#include"tree/vardecl.h"
#include "file_ast.h"
class Parser : public ParserBase<Token>
{
    fs::path currentFile;
public:
    void setCurrentFile(const fs::path& file_name)
    {
        if ( file_name.is_absolute() ) currentFile = file_name;
        else currentFile = fs::absolute(file_name);
    }
    Parser()
    {
    
    }
    void setTokens(const std::vector<Token>& tokens)
    {
        setTokenStream(tokens,{"BeforeStartToken",0,0,Token::UNDEFINED},{"AfterEndToken",0,0,Token::EF});
    }

    void statementPanic();

    bool varDeclPanic();
    std::unique_ptr<AST> parse(bool isGlobal);

    std::string parseName();
    tree::NodePtr parseImport();    
    tree::NodePtr parseObjective();
    tree::NodePtr parseNamespace();
    tree::FunctionPtr parseFunction( bool can_have_return ); 
    tree::NodePtr parseStorageDecl();

    std::vector<tree::VarDeclPtr> parseFunctionArg();

    std::optional<tree::Raw::Variant> parseRawItem();
    tree::NodePtr parseRaw();
    tree::RawPtr parseRawLine();
    tree::BlockPtr parseRawBlock();

    tree::NodePtr parseStatement();
    tree::BlockPtr parseBlock();

    struct VarSignature {
        std::optional<std::string> type;
        size_t list_depth;
        std::vector<std::string> modifiers;
        bool is_link = false;
    };

    VarSignature parseVarSignature();

    tree::NodePtr parseIf();
    tree::NodePtr parseWhile();

    tree::VarDeclPtr parseVarDecl(bool init_required = true);

    tree::NodePtr parseReturn();
    tree::ExprNodePtr parseInitList();
    tree::ExprNodePtr parseCall();
    tree::ExprNodePtr parseExpr(float bp = 0);
    tree::ExprNodePtr parseExprLine(float bp = 0);

    tree::ExprNodePtr parseIndexAccess();

    tree::ExprNodePtr parsePrimaryTerm();
    tree::ExprNodePtr parseTerm();
};