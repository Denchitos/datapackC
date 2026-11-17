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
#include"call_tree_builder.h"
#include"ast_builder.h"
namespace fs = std::filesystem;

bool compile(std::vector<tree::NodePtr>& v,std::shared_ptr<Modules> modules,const std::string& target,bool create_files)
{
    auto ret_checker = std::make_unique<ReturnChecker>();
    
    for ( auto& it : v )
    {
        it->accept(ret_checker.get());
    }

    auto ctx = std::make_shared<FileCompilationContext>();
    
    auto codeG = std::make_shared<ExprCodeGen>(ctx);

    auto store_decl = std::make_unique<StorageDeclVisitor>(ctx,modules);

    try {
        for ( auto& it : v )
        {
            it->accept(store_decl.get());
        }
    }
    catch ( std::exception& e )
    {
        std::cout<<"[ERROR] [Declarations] "<<e.what()<<"\n";
        return false;
    }

    std::cout<<"Declaration...\n";


    auto declarator = std::make_unique<FuncDeclVisitor>(ctx,modules);

    try {
        for ( auto& it : v )
        {
            it->accept(declarator.get());
        }
    }
    catch ( std::exception& e )
    {
        std::cout<<"[ERROR] [Definer] "<<e.what()<<"\n";
        return false;
    }

    std::cout<<"Build call tree...\n";

    auto tree_builder = std::make_unique<CallTreeBuilder>(ctx);

    try {
        for ( auto& it : v )
        {
            it->accept(tree_builder.get());
        }
    }
    catch ( std::exception& e )
    {
        std::cout<<"[ERROR] [TreeBuilder] "<<e.what()<<"\n";
        return false;
    }

    auto rec = tree_builder->findRecursion();

    if ( rec.size() != 0 )
    {
        std::cout<<"Recursion detected!\nCompilation aborted\n";

        for ( auto& it : rec )
        {
            std::cout<<it<<"\n";
        }
        return false;
    }

    std::cout<<"No Recursion detected\n";

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
        return false;
    }

    if ( !create_files ) return false;

    for ( auto& it : visite->compiled_functions )
    {
        fs::path path( target+"/data/" + it.toPath() );
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
        fs::path path( target+"/data/" + load.toPath() );
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
        fs::path path( target+"/data/" + tick.toPath() );
        fs::create_directories(path.parent_path());
        std::ofstream out(path);
        for (auto& line : tick.text )
        {
            out<<line<<"\n";
        }
    }

    std::cout<<"Compilation Finished!\n";

    return true;
}

int start(int argc,char ** argv)
{
    system("chcp 65001 > nul");
    if ( (argc != 2 && argc !=3) )
    {
        fs::path path(argv[0]);
        auto program_file = fs::relative(path).generic_string(); 
        std::cerr<<"How to use:\n";
        std::cerr<< program_file <<" <source>\n";
        std::cerr<< program_file <<" <source> <target directory>\n";
        return 1;
    }
    
    std::string target = "datapack";

    if ( argc==3 )
    {
        target = argv[2];
    }

    std::string file_name(argv[1]);

    auto lexer = std::make_shared<Lexer>();
    auto parser = std::make_shared<Parser>();

    FilesASTBuilder ast_builder(lexer,parser);

    ast_builder.buildFile(file_name);

    std::cout<<"[AST BUILDING FINISHED]\n";
    
    auto vec = ast_builder.findCyclicImport();

    if ( !vec.empty() )
    {
        std::cerr<<"Detected Cyclic Import:\n";
        for ( auto & pathes : vec )
        {
            std::string pref="    ";
            std::cerr<<"...->\n";
            for ( auto & item : pathes )
            {
                std::cerr<<pref<<fs::relative(item).generic_string()<<"->\n";
                pref.append("    ");
            }
            std::cerr<<pref<<"->...\n";
        }
        return 2;
    }

    bool build_files = true;

    for ( auto & [ path,ast ] : *(ast_builder.getProcFiles()) )
    {
        std::cout<<"Compiling File...:\n   "<<path<<"\n";
        build_files = compile(ast.ast,ast_builder.getProcFiles(),target,build_files);
        std::cout<<"\n";
    }

    if ( build_files ) return 0;
    else return 3;
}

int main(int argc,char** argv)
{
    int res = 0;
    try {
        res = start( argc,argv );
    }
    catch(std::exception& e)
    {
        std::cerr<<e.what()<<"\n";
        res = 5;
    }


    if (res == 0)
        std::cout<<"Program Finished!\n";
    else
        std::cerr<<"[ERRORS] Got errors while compiling project\n";
    std::cout<<"Exit code: "<<res<<"\n";
    return res;
}   