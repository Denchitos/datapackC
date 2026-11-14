#include<fstream>
#include<iostream>
#include<iomanip>
#include<string>
#include<filesystem>
#include<memory>
#include"lexer.h"
#include"parser.h"
//#include"print_visitor.h"
#include"compiler_visitor.h"
#include"declaration_visitor.h"
#include"return_checker.h"
namespace fs = std::filesystem;

void compile(std::vector<tree::NodePtr>& v)
{
    auto ret_checker = std::make_unique<ReturnChecker>();
    
    for ( auto& it : v )
    {
            it->accept(ret_checker.get());
    }

    auto ctx = std::make_shared<FileCompilationContext>();
    
    auto codeG = std::make_shared<ExprCodeGen>(ctx);

    auto store_decl = std::make_unique<StorageDeclVisitor>(ctx);

    std::cout<<"Declaration...\n";

    try {
        for ( auto& it : v )
        {
            it->accept(store_decl.get());
        }
    }
    catch ( std::exception& e )
    {
        std::cout<<"[ERROR] "<<e.what()<<"\n";
        return;
    }

    auto declarator = std::make_unique<FuncDeclVisitor>(ctx);

    try {
        for ( auto& it : v )
        {
            it->accept(declarator.get());
        }
    }
    catch ( std::exception& e )
    {
        std::cout<<"[ERROR] "<<e.what()<<"\n";
        return;
    }

    auto visite =  std::make_unique<CompilerVisitor>(ctx,codeG); 
    
    std::cout<<"Compilation...\n";

    try {
        for ( auto& it : v )
        {
            it->accept(visite.get());
        }
    }
    catch ( std::exception& e )
    {
        std::cout<<"[ERROR] "<<e.what()<<"\n";
        return;
    }

    std::cout<<"Making Datapack\n";

    for ( auto& it : visite->compiled_functions )
    {
        fs::path path( "datapack/data/" + it.toPath() );
        fs::create_directories(path.parent_path());
        std::ofstream out(path);
        for (auto& line : it.text )
        {
            out<<line<<"\n";
        }
    }

    if ( ctx->hasLoadFile() )
    {
        auto& load = ctx->getLoadFile();
        fs::path path( "datapack/data/" + load.toPath() );
        fs::create_directories(path.parent_path());
        std::ofstream out(path);
        for (auto& line : load.text )
        {
            out<<line<<"\n";
        }
    }

    if ( ctx->hasTickFile() )
    {
        auto& tick = ctx->getTickFile();
        fs::path path( "datapack/data/" + tick.toPath() );
        fs::create_directories(path.parent_path());
        std::ofstream out(path);
        for (auto& line : tick.text )
        {
            out<<line<<"\n";
        }
    }

    std::cout<<"Compilation Finished!\n";
}

int main(int argc,char** argv)
{
    system("chcp 65001 > nul");
    if ( argc != 2 )
    {
        std::cerr<<"Программа ожидает 1 файл";
        return 1;
    }

    std::string file_name(argv[1]);
    std::ifstream file(file_name);
    std::string line;
    std::string text;
    while ( std::getline(file,line) )
    {
        text+=line+'\n';
    }
    //std::cout<<text<<"\n\n\n";

    std::cout<<"Лексинг...\n";
    Lexer l;
    l.setText(std::move(text));
    auto res = l.lex();

    // for (auto& it : res)
    // {
    //     std::cout<<"["<<std::setw(5)<<it.row<<","<<std::setw(5)<<it.col<<"]"<<"\t - ";
    //     if ( it.tok=="\n" ) std::cout<<"EOL\n";
    //     else std::cout<<"{"<<it.tok<<"}\n";
    // }

    std::cout<<"Лексинг окончен\n";

    Parser p;
    p.setTokens(res);

    std::cout<<"Парсинг...\n";

    auto v = p.parse();

    auto parsing_msg = p.getMessages();

    for ( auto& it : parsing_msg )
    {
        std::cout<<"["<<it.token.row<<" - "<<it.token.col<<" - "<< it.token.tok << "] "<<it.msg<<"\n";
    }

    std::cout<<"Парсинг окончен\n";
    
    if (!parsing_msg.empty()) return 1;

    compile(v);
}
