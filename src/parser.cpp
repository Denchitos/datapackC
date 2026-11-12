#include"parser.h"
#include"tree/vardecl.h"
#include"tree/raw_selector.h"
#include"tree/namespace.h"
#include"tree/objective.h"
#include"tree/member_access.h"
#include"tree/if.h"
#include"tree/call.h"
#include"tree/while.h"
#include"tree/return.h"
#include"tree/storage.h"
#include<optional>
#include<unordered_set>
////PARSING////
////PARSING////
////PARSING////
////PARSING////

std::vector<tree::NodePtr> Parser::parse()
{
    std::vector<tree::NodePtr> v;

    while (!atEnd())
    {
        if (match(Token{"def",Token::Word})) v.emplace_back(parseFunction(true));
        else if (match(Token{"inline",Token::Word})){
            auto& tok = look();
            auto inl = parseFunction(false);
            v.emplace_back(std::make_unique<tree::InlinedFunction>(tok,std::move(inl)));
        }
        else if (compare(Token{"namespace",Token::Word})) v.emplace_back(parseNamespace());
        else if (compare(Token{"objective",Token::Word})) v.emplace_back(parseObjective());
        else if (compare(Token{"storage",Token::Word})) v.emplace_back(parseStorageDecl());
        else
        {
            error("Undefined Token in global space");
            statementPanic();
        }
    }

    return v;
}

tree::NodePtr Parser::parseObjective()
{
    if (!expect(Token{"objective",Token::Word},"Expected 'objective' in objective declaring") ){ statementPanic() ;return nullptr;}
    if ( !expect(Token::Word,"Objective name must be a word. Not Literal, Not Number") ){ statementPanic() ;return nullptr;}

    auto obj = std::make_unique<tree::Objective>(look(-2));
    obj->name = look(-1).tok;
    match(Token::EL);
    return obj;
}

tree::NodePtr Parser::parseNamespace()
{
    if (!expect(Token{"namespace",Token::Word},"Expected 'namespace' in namespace defining") ){ statementPanic() ;return nullptr;}
    
    auto nmsp = std::make_unique<tree::Namespace>(look(-1));

    if ( match(Token::Word) )
    {
        nmsp->namespace_ = look(-1).tok;
    }
    else
    {
        error( "The namespace name must be a Word. Not Literal, Not Number" );
    }

    if ( match(Token::Colon) ) while ( !compare(Token::EL) )
    {
        if ( !expect(Token::Word,"Namespace path contains only Words") ) { statementPanic(); return nmsp; }

        nmsp->path.emplace_back(look(-1).tok);

        if ( !match(Token{"/",Token::Operator}) ) break; 
    }
    if ( !expect(Token::EL,"namespace declaration must be finished with newline symbol")) statementPanic();
    return nmsp;
}

tree::NodePtr Parser::parseStorageDecl()
{
    if (!expect(Token{"storage",Token::Word},"Expected 'storage' in storage defining") ) return nullptr;

    if (!expect(Token::Word,"Expected a WORD as a storage name")) return nullptr;

    auto storage = std::make_unique<tree::StorageDecl>(look(-1));

    match(Token::EL);

    if (!expect(Token::OBrace,"Expected a '{' in storage definition") ) return nullptr;
    
    match(Token::EL);

    while ( true )
    {
        //std::cout<<"STORAGE DECL CYCLE " << look().tok<<"\n";
        if ( match(Token{"def",Token::Word} ) ){
            storage->pushMethod(parseFunction(true));
            match(Token::EL);
        }
        else if ( match(Token{"inline",Token::Word} ) ){
            auto& tok = look();
            auto inl = parseFunction(false);
            auto inl_ptr = std::make_unique<tree::InlinedFunction>(tok,std::move(inl));
            storage->pushInlined(std::move(inl_ptr));
            match(Token::EL);
        }
        else if ( compare(Token::CBrace) )
            break;
        else if ( compare(Token::Word ) ) {            
            storage->pushField(parseVarDecl(false));
            match(Token::EL);
        }
        else {
            error("Unexpected Token");
            statementPanic();
            return nullptr;
        }
    }
    match(Token::EL);

    if (!expect(Token::CBrace,"Expected a '}' after storage definition") ) return nullptr;

    match(Token::EL);

    return storage;
}

tree::FunctionPtr Parser::parseFunction( bool can_have_return )
{
    if (!expect(Token::Word,"Expected a WORD as a function name")) return nullptr;
    
    auto function = std::make_unique<tree::Function>(look(-1));
    
    match(Token::EL);
    
    if ( compare(Token::OParen) ) function->args = parseFunctionArg();

    if ( compare(Token{"->",Token::Operator}) && can_have_return )
    {
        next();
        auto [ type_name,list_depth,modifeirs,is_link ] = parseVarSignature();
        if ( is_link )
        {
            error( "Function return type can't be link" );
        }
        if ( !type_name.has_value() )
        {
            error("Not defined name of type in function return statement");
            statementPanic();
        }

        function->ret_type = { type_name.value() , list_depth};
        next();
    }
    else if (compare(Token{"->",Token::Operator}) && !can_have_return)
    {
        error("Return type is unexpected here");
        statementPanic();
    }

    match(Token::EL);
    function->body = parseBlock();
    
    match(Token::EL);

    return function;
}

std::vector<tree::VarDeclPtr> Parser::parseFunctionArg()
{
    std::vector<tree::VarDeclPtr> args;

    expect(Token::OParen,"Expected '(' before function Arg List");

    match(Token::EL);

    if (!compare(Token::CParen))
        while (true)
        {
            if ( !compare(Token::Word) )
            {
                error("Unexpected token as function argument");
                return args;
            }
            
            args.emplace_back(parseVarDecl(false));

            if ( !match(Token::Comma) ) break;
            match(Token::EL);
        }

    match(Token::EL);

    if (!expect(Token::CParen,"Expected ')' after function Argument List")) return {};
    
    match(Token::EL);
    
    return args;
}

tree::NodePtr Parser::parseRaw()
{
    if (!expect(Token{"raw",Token::Word},"Raw code block must be started with \"raw\" keyword")) return nullptr;

    if (compare(Token::OBrace) || ( compare(Token::EL) && compare(Token::OBrace,1) ) )
    {
        match(Token::EL);
        return parseRawBlock();
    }
    else
    {
        if (match(Token::EL)){
            error("raw requires a command after keyword");
            return nullptr;
        }
        return parseRawLine();
    }
}

tree::BlockPtr Parser::parseRawBlock()
{
    if ( !expect(Token::OBrace,"The Raw Block is beginning with symbol '{', but it wasn't found") ) return nullptr;
    auto block = std::make_unique<tree::Block>(look(-1));
    match(Token::EL);
    while ( !match(Token::CBrace) )
    {
        block->push(parseRawLine());
    }
    match(Token::EL);
    return block;
}

tree::RawPtr Parser::parseRawLine()
{
    auto raw = std::make_unique<tree::Raw>(look(-1));
    bool cont = true;  
    while ( cont )
    {
        if ( match(Token::EL) ) break;
        else if ( ( match(Token{"%",Token::Operator})) )
        {
            bool space_before = look(-1).space_before;
            bool is_literal = false;
            int id = -1;
            std::string word = "undefined";
            if ( match(Token::Colon) ) is_literal = true;

            if ( match(Token::Word) ) word = look(-1).tok;

            if ( match(Token::Dot) )
            {
                if ( !expect(Token::Number,"after '.' in derefferencing exepcted a number. Example: %<var>.<param_id>%") ) {statementPanic(); return raw;}
                id = std::stoi(look(-1).tok);
            }
            //if ( is_literal ) std::cout<<"LITERAL DEREF\n";
            raw->pushVar(std::move(word),id,is_literal,space_before);
            if ( !expect(Token{"%",Token::Operator},"Every var derefferencing in raw block '% ... %' must be finished with % ") ) return raw;
        }
        else {
            raw->pushWord(look().trueString(),look().space_before);
            next();
        }
    }
    return raw;
}

tree::ExprNodePtr Parser::parseCall()
{
    //std::cout<<"ПАРСИНГ ВЫЗОВА " << look().tok <<" " << look().row << " " << look().col << "\n";

    if (!expect(Token::Word,"Function call name must be a word no anything else"))
    {
        statementPanic();
        return nullptr;   
    }
    auto name = look(-1).tok;
    auto call_name_id = current() - 1;

    if ( !expect(Token::OParen,"Function call is <function>(<args>...) , expected '('") )
    {
        statementPanic();
        return nullptr;
    }

    auto call = std::make_unique<tree::Call>(look(call_name_id - current()));

    if ( !compare(Token::CParen) )
    {
        while ( true )
        {
            call->push(parseExpr());
            match(Token::EL);
            if ( !match(Token::Comma) ){ match(Token::EL) ; break; } 
        }
    }

    if ( !expect(Token::CParen,"Function call is <function>(<args>...) , expected ')'") )
    {
        statementPanic();
        return nullptr;
    }

    //std::cout<<"Parsed Call "<<call->getName()<<" "<<name<<"\n";

    return call;
}

//Парсинг инструкций. ВАЖНО : Она не отвечает чтобы i останавливался не на i. За это отвечает фнукция parse*() какой-то инструкии
tree::NodePtr Parser::parseStatement()
{
    //std::cout<<"ParseStatement "<<a.tok<<" "<<a.row<<" "<<a.col<<" "<<Token::typeToString(a.type)<<"\n";
    if ( compare(Token{"raw",Token::Word}) ) return parseRaw();
    if ( match(Token{"let",Token::Word}) ) return parseVarDecl();
    if ( compare(Token{"if",Token::Word}) ) return parseIf();
    if ( match(Token{"break",Token::Word}) ) { match(Token::EL); return std::make_unique<tree::Break>(look(-1)); };
    if ( compare(Token{"while",Token::Word}) ) return parseWhile();
    if ( compare(Token::OBrace) ) return parseBlock();
    if ( compare(Token{"return",Token::Word}) ) return parseReturn();
    else {
        //std::cout<<"Парсинг Expr in Statement\n"
        return parseExpr();
    }
}

tree::NodePtr Parser::parseWhile()
{
    if ( !expect(Token{"while",Token::Word},"Expected 'while' as start of conditions block") ) { statementPanic() ; return nullptr;}
    auto whileToken = look(-1);
    bool is_scheduled = false;;
    int schedule = 0;
    if ( match(Token{"scheduled",Token::Word}) )
    {
        is_scheduled = true;
        if ( !expect(Token::Number,"Expected a number of tick for schedule") ) { statementPanic() ; return nullptr;}
        schedule = std::stoi(look(-1).tok);
        if ( schedule > 255 || schedule < 0) error("The Schedule must be less than 255",-1);
    }
    else if ( compare(Token::Word) )
    {
        error("May be You means 'scheduled' , but got '"+look().tok+"'");
        statementPanic();
        return nullptr;
    }
    match(Token::EL);

    if ( !expect(Token::OParen,"Expected '(' as start of conditional expression in while block") ) { statementPanic() ; return nullptr;}
    auto expr = parseExpr();
    if ( !expect(Token::CParen,"Expected ')' as end of conditional expression in while block") ) { statementPanic() ; return nullptr;}
    match(Token::EL);
    auto body = parseStatement();
    match(Token::EL);

    if (is_scheduled)
    {
        auto SWhile = std::make_unique<tree::ScheduledWhile>(whileToken);
        SWhile->expr = std::move(expr);
        SWhile->body = std::move(body);
        SWhile->schedule = schedule;
        if( match(Token{"then",Token::Word}))
        {
            SWhile->then = parseStatement();
            
        }
        return SWhile;
    }
    else
    {
        if( match(Token{"then",Token::Word}))
        {
            error("'then' block can be declared only after scheduled while");
            statementPanic();
            return nullptr;
        }
        auto While = std::make_unique<tree::While>(whileToken);
        While->expr = std::move(expr);
        While->body = std::move(body);
        return While;
    }
}

tree::NodePtr Parser::parseIf()
{

    if ( !expect(Token{"if",Token::Word},"Expected 'if' as start of conditions block") ){ statementPanic() ; return nullptr;};
    auto If = std::make_unique<tree::If>(look(-1));
    if ( !expect(Token::OParen,"Expected '(' as start of conditional expression in if block") ) { statementPanic() ; return If;};
    auto IfExpr = parseExpr();
    if ( !expect(Token::CParen,"Expected ')' as end of conditional expression in if block") ){ statementPanic() ; return If;}
    match(Token::EL);
    auto Then = parseStatement();
    If->elif.emplace_back(std::move(IfExpr),std::move(Then));
    while ( match(Token{"elif",Token::Word}) )
    {
        if ( !expect(Token::OParen,"Expected '(' as start of conditional expression in elif block") ) { statementPanic() ; return If;};
        auto IfExpr = parseExpr();
        if ( !expect(Token::CParen,"Expected ')' as end of conditional expression in elif block") ){ statementPanic() ; return If;};
        match(Token::EL);
        auto Then = parseStatement();
        If->elif.emplace_back(std::move(IfExpr),std::move(Then));
    }
    if ( match(Token{"else",Token::Word}) )
    {
        match(Token::EL);
        If->else_ = parseStatement();
    }
    match(Token::EL);
    return If;
}

tree::BlockPtr Parser::parseBlock()
{
    if (!expect(Token::OBrace,"Block of Statement must be begins with '{'")) {statementPanic();return nullptr;}
    
    auto block = std::make_unique<tree::Block>(look(-1));
    match(Token::EL);

    while( !match(Token::CBrace) )
    {
        block->push(parseStatement());
    }

    match(Token::EL);
    return block;
}

std::unique_ptr<tree::ExprNode> Parser::parseExpr(float bp)
{
    auto expr = parseExprLine(bp);
    match(Token::EL);
    return expr;
}

tree::ExprNodePtr Parser::parseInitList()
{
    if (!expect(Token::OBrace,"Expected a '{', that opens initializer list")) {statementPanic() ; return nullptr;}
    
    auto initlist = std::make_unique<tree::InitList>(look(-1));

    while ( !compare(Token::CBrace) )
    {
        initlist->list.emplace_back(parseExpr());
        if ( !match(Token::Comma) ) break;
    }
    if (!expect(Token::CBrace,"Expected a '}', that closes initializer list"))
    {
        statementPanic();
        return nullptr;
    }

    return initlist;
}

static std::unordered_map<std::string,std::pair<float,float>> operators{
    {"=",{0,1}},
    {"+=",{0,1}},
    {"-=",{0,1}},
    {"/=",{0,1}},
    {"*=",{0,1}},
    {"%=",{0,1}},

    {"||",{1.1,1.15}},
    {"&&",{1.3,1.2}},
    
    { ">",{ 1.32,1.31 }},
    { "<",{ 1.32,1.31 }},
    { ">=",{ 1.32,1.31 }},
    { "<=",{ 1.32,1.31 }},
    { "==",{ 1.32,1.31 }},
    { "!=",{ 1.32,1.31 }},

    {"+",{3,2}},
    {"-",{3,2}},
    {"*",{5,4}},
    {"/",{5,4}},
    {"%",{5,4}},
};


tree::NodePtr Parser::parseReturn()
{
    if (!expect(Token("return",Token::Word),"Expected a 'return' keyword in return"))
    {
        statementPanic();
        return nullptr;
    }

    auto ret = std::make_unique<tree::Return>(look(-1));
    if (compare(Token::EL) || compare(Token::CBrace) || atEnd())
        match(Token::EL);
    else
        ret->expr = parseExpr();
    return ret;
}

//true - found "=" | false - not found "=", finished on next line or on file end
bool Parser::varDeclPanic()
{
    while ( !atEnd() )
    {
        if ( compare(Token{"=",Token::Operator}) ) return true;
        else if ( match(Token::EL) ) return false;
        next();
    }
    return false;
}

tree::VarDeclPtr Parser::parseVarDecl(bool init_required)
{
    //if (!expect(Token{"let",Token::Word},"Variable Declaration begins with \"let\" keyword")){ statementPanic();return nullptr;} 
    if (!compare(Token::Word))
    {
        statementPanic();
        return nullptr;
    }

    std::string name = look().tok;
    auto vardecl = std::make_unique<tree::VarDecl>(look(-2));
    vardecl->setVarName(name);
    
    next();

    std::optional<std::string> type = std::nullopt;

    if ( match(Token::Colon) )
    {
        auto [var_type,list_depth,modifiers,is_link] = parseVarSignature();

        if ( var_type.has_value() )
        {
            vardecl->setVarType(var_type.value());
            vardecl->list_depth = list_depth;
        }
        if ( is_link && init_required )
        {
            error("Variable can not be a link when declared in scope");
        }
        vardecl->is_link = is_link;
        for ( auto& it: modifiers )
        {
            if ( it == "mut" || it == "mutable" )
            {
                vardecl->setMut(true);
            }
        }
    }
    if (init_required)
    {
        if ( !expect(Token{"=",Token::Operator},"VARIABLE INITIALISATION IS REQUIRED HERE") ) statementPanic();
        else vardecl->expr = parseExpr();
    }
    
    return vardecl;
}

static std::unordered_set<std::string> modifiers = 
{
    "mut","mutable"
};

Parser::VarSignature Parser::parseVarSignature()
{
    std::optional<std::string> type = std::nullopt;
    std::vector<std::string> modif;
    size_t list_depth = 0;
    bool is_link = false; 
    while ( compare(Token::Word) || compare(Token::Dollar) )
    {
        auto& word = look().tok;
        if ( match(Token::Dollar) ) is_link = true;
        else if ( modifiers.contains(word))
        {
            modif.emplace_back(word);
            next();
        }
        else if ( !type.has_value() )
        {
            type = word;
            next();
            while ( match(Token::OBracket) )
            {
                if ( match(Token::CBracket) )
                {
                    list_depth++;
                }
                else
                {
                    error("Expected a ']' for '[' in col : "+ std::to_string(look(-1).col));
                }
            }

        }
        else
        {
            error("Undefined Token when parse Variable signature : \""+
                word + "\" on position "+
                std::to_string(look().row) + " " + std::to_string(look().col));
            next();
        }
    }
    return {type,list_depth,modif,is_link};
}

void Parser::statementPanic()
{
    while ( !match(Token::EL) ) next();
}

std::unique_ptr<tree::ExprNode> Parser::parseExprLine(float bp)
{
    //std::cout<<"ParseExpr\n";
    auto left = parseTerm();
    while ( !compare(Token::EL) || !atEnd() )
    {
        //std::cout<<"ЦИКЛ\n";
        if ( !compare(Token::Operator) )
        {
            //error("Expected Operator, but got something else");
            //while ( !match(Token::EL) ) next();
            break;
        }

        auto& op_token = look();
        auto op_str = op_token.tok;

        if ( !operators.contains(op_str) )
        {
            error("Unknown Operator");
            while ( !match(Token::EL) ) next();
            break;
        }
        
        auto [lbp,rbp] = operators[op_str];

        if ( lbp < bp ) { break;}
        next();
        
        auto bin_op = std::make_unique<tree::BinaryOperator>(op_token);
        //std::cout<<"Сейчас будет ошибка\n";
        auto right = parseExpr(rbp);
        bin_op->right = right ? std::move(right) : nullptr;
        bin_op->left = std::move(left);
        left = std::move(bin_op);
    }
    return left;

}

std::unique_ptr<tree::ExprNode> Parser::parsePrimaryTerm()
{
    if ( match(Token::Literal) )
    {
        return std::make_unique<tree::TextLiteral>(look(-1));
    }
    else if ( compare(Token::Word) && (compare("true") || compare("false")) )
    {
        auto boolLit = std::make_unique<tree::BoolLiteral>(look());
        boolLit->val = (look().tok == "true");
        next();
        return boolLit;
    }
    else if ( match(Token::Number) )
    {
        return std::make_unique<tree::NumberLiteral>(look(-1));
    }
    else if ( match(Token::At) )
    {
        auto sel = std::make_unique<tree::RawSelector>(look(-1));
        
        if      (compare(Token{"s",Token::Word})) sel->type = tree::RawSelector::executor;
        else if (compare(Token{"n",Token::Word})) sel->type = tree::RawSelector::nearest;
        else if (compare(Token{"a",Token::Word})) sel->type = tree::RawSelector::all;
        else if (compare(Token{"e",Token::Word})) sel->type = tree::RawSelector::entities;
        else if (compare(Token{"r",Token::Word})) sel->type = tree::RawSelector::random;
        else
        {
            error("Undefined Selector symbol after @ : expected 's' 'n' 'r' 'a' 'e', but got " + look().tok );
            statementPanic();
        }
        next();
        if (match(Token::OBracket))
        {
            while (!( compare(Token::CBracket) || atEnd() || compare(Token::EL) ) )
            {
                sel->params+=look().trueString();
                next();
            }
            if ( !expect(Token::CBracket,"Expected ']' closing Selector parameters") ) statementPanic();
        }
        return sel;
    }
    else if ( match( Token::OParen) )
    {
        auto col = look(-1).col;
        auto row = look(-1).row;
        auto a = parseExpr();
        if ( !match(Token::CParen) )
        {
            error("Expected ')', finishing a statement, starts on " + std::to_string(row) + " - " + std::to_string(col));
            while ( !match(Token::EL) ) next();
        }
        return a;
    }
    else if (compare(Token::OBrace)) return parseInitList();
    else if ( compare(Token::Word) && compare(Token::OParen,1) ) return parseCall();
    else if ( compare(Token::Word) )
        { next() ; return std::make_unique<tree::Variable>(look(-1));}
    else
    {
        error("Something wrong when parse Primary Term");
        while ( !match(Token::EL) ) next();
        return nullptr;
    }
}

std::unique_ptr<tree::ExprNode> Parser::parseTerm()
{
    auto member = parsePrimaryTerm();
    if ( !compare(Token::Dot) && !compare(Token::OBracket) ) { return member; }

    auto mem_access = std::make_unique<tree::MemberAccess>(look());

    mem_access->members.emplace_back(std::move(member));

    while ( true )
    {
        //std::cout<<"PARSE TERM "<<look().tok<<"\n";
        if ( match(Token::Dot) )
            mem_access->members.emplace_back(parsePrimaryTerm());
        else if ( match(Token::OBracket) )
        {
            auto tok = look(-1);
            tok.tok = "operator[]";
            auto call = std::make_unique<tree::Call>(tok);
            while ( true )
            {
                call->call_args.push_back(parseExpr());
                if ( match(Token::Comma) ) continue;
                else if ( match(Token::CBracket) ) break;
                else error("Expected a '[' that closes 'operator[]', started in line: " + std::to_string(tok.row) + " in row: " + std::to_string(tok.col));
            }
            mem_access->members.push_back(std::move(call));
        }
        else
        {
            error("UnexpectedToken");
            statementPanic();
            return mem_access;
        }
        if ( !compare(Token::Dot) && !compare(Token::OBracket ) ) break;
    }

    return mem_access;
}