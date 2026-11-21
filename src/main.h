#include<fstream>
#include<iostream>
#include<iomanip>
#include<string>
#include<filesystem>
#include<memory>
#include"lexer.h"
#include"parser.h"
#include"compiler_visitor.h"
#include"declaration_visitor.h"
#include"return_checker.h"
#include"call_tree_builder.h"
#include"ast_builder.h"
#include<algorithm>

struct Settings
{
    std::string target = "datapack";
    std::string root_file = "src";
};


class ArgumentParser
{
    Settings settings;
    std::string cerr;
public:

    std::expected<Settings,std::string> getRet()
    {
        if (cerr.empty())
            return settings;
        else
            return std::unexpected(cerr);
    }

    std::expected<Settings,std::string> parse(int argc, char** argv)
    {
        if ( (argc != 2 && argc !=3) )
        {
            fs::path path(argv[0]);
            auto program_file = fs::relative(path).generic_string(); 
            cerr.append("How to use:\n");
            cerr.append(program_file +" <source>\n");
            cerr.append(program_file +" <source> <target directory>\n");

            return getRet();
        }

        settings.root_file = argv[1];

        if ( argc==3 )
        {
            settings.target = argv[2];
        }
        return getRet();
    };
};

inline int BAD_ARGS = 1;
inline int BAD_BUILD = 2;
inline int BAD_FORWARD_DECL = 3;
inline int BAD_RECURSIVE_DECL = 4;
inline int BAD_FORWARD_DEF = 5;
inline int BAD_RECURSIVE_DEF = 6;
inline int BAD_COMPILING = 7;
inline int OBJECTS_COLLISION = 8;

struct Object
{
    fs::path source;
    Function func;
};

class Program
{
    Settings settings;;
    int res = 0;
    std::shared_ptr<Modules> modules = nullptr;
    std::shared_ptr<FilesASTBuilder> ast_builder = nullptr;

    std::unordered_map<fs::path,Object> compiled_objects;
public:
    int main(int argc,char** argv)
    {
        auto arg_parser =   std::make_unique<ArgumentParser>();
        auto lexer =        std::make_shared<Lexer>();
        auto parser =       std::make_shared<Parser>();
        
        ast_builder = std::make_shared<FilesASTBuilder>(lexer,parser);

        auto args = arg_parser->parse(argc,argv);
        if ( !args ){
            std::cerr<<args.error();
            return BAD_ARGS;
        }
        settings = *args;

        if (!buildStage()) return getRet();

        modules = ast_builder->getProcFiles();

        if (!forwardDeclaringStorages()) return getRet();

        //printProcFiles();
        if (!recursiveStorageDecl()) return getRet();
        //printProcFiles();
        
        if (!forwardDefining()) return getRet();
        //printProcFiles();
        
        if (!recursiveDefining()) return getRet();
        //printProcFiles();
        
        if ( !compilingStage() ) return getRet();
        //printProcFiles();
        codeGenStage();

        return getRet();
    };

    int getRet()
    {
        if ( res == 0 )
            std::cout<<"=Project compiled succesfully=\n";
        else
            std::cout<<"=Project compiled with ERRORS=\n";
        return res;
    }

    bool buildStage()
    {
        std::cout<<"[Building Project Stage]\n";

        ast_builder->pushCandidate(settings.root_file);

        bool ast_building_success = true;

        while (true)
        {
            auto candidate = ast_builder->getNextCandidates();
            
            if ( !candidate.has_value() ) break;
            
            if ( !ast_builder->buildFile(candidate.value()) ) ast_building_success = false;
        }

        std::cout<<"[AST BUILDING FINISHED]\n";

        if ( !ast_building_success )
        {
            std::cerr<<"While build AST, Compiler got errors. Compilation stage is unavavilable\n";
            res = BAD_BUILD;
            return false;
        }
        return true;
    }

    bool forwardDeclaringStorages()
    {
        std::cout<<"[Forward Declaration Stage]\n";
        bool result = true;
        for (auto & [ path, file] : *modules )
        {
            std::cout<<"\tFile: "<<fs::relative(path).generic_string()<<"\n";
            auto ctx = std::make_shared<FileCompilationContext>();

            ctx->currentNamespace = file.nmsp;
            ctx->currentDefinedNamespace = file.nmsp;

            auto forward = std::make_unique<ForwardStorageDeclaration>(ctx,modules);
            
            try
            {
                for ( auto& node : file.ast )
                {
                    node->accept(forward.get());
                }    
            }
            catch ( std::exception& e ){
                std::cerr<<"[ERROR]"<<" "<< fs::relative(path) <<"\n\t"<<e.what()<<"\n";
                result = false;
                res = BAD_FORWARD_DECL;
            }

            file.fileCtx = ctx;

        }

        return result;
    }

    bool recursiveStorageDecl()
    {
        std::cout<<"[Recursive Declaration Stage]\n";
        bool result = true;
        for (auto & [ path, file] : *modules )
        {
            std::cout<<"\t"<<fs::relative(path).generic_string()<<"\n";
            auto recursive = std::make_unique<RecursiveStorageDeclaration>(path,file.fileCtx,modules);
            try
            {
                for ( auto& node : file.ast )
                {
                    node->accept(recursive.get());
                }    
            }
            catch ( std::exception& e ){
                std::cerr<<"[ERROR]"<<" "<< fs::relative(path) <<"\n\t"<<e.what()<<"\n";
                result = false;
                res = BAD_RECURSIVE_DECL;
            }

        }
        return result;
    }

    bool forwardDefining()
    {
        std::cout<<"[Forward Defining Stage]\n";
        bool result = true;
        for (auto & [ path, file] : *modules )
        {
            std::cout<<"\t"<<fs::relative(path).generic_string()<<"\n";
            
            auto recursive = std::make_unique<ForwardDefiner>(file.fileCtx,modules);
            
            try
            {
                for ( auto& node : file.ast )
                {
                    node->accept(recursive.get());
                }    
            }
            catch ( std::exception& e ){
                std::cerr<<"[ERROR]"<<" "<< fs::relative(path) <<"\n\t"<<e.what()<<"\n";
                result = false;
                res = BAD_FORWARD_DEF;
            }

        }
        return result;
    }

    bool recursiveDefining()
    {
        std::cout<<"[Recursive Defining Stage]\n";
        bool result = true;
        for (auto & [ path, file] : *modules )
        {
            std::cout<<"\t"<<fs::relative(path).generic_string()<<"\n";
            auto recursive = std::make_unique<RecursiveDefiner>(path,file.fileCtx,modules);
            try
            {
                for ( auto& node : file.ast )
                {
                    node->accept(recursive.get());
                }    
            }
            catch ( std::exception& e ){
                std::cerr<<"[ERROR]"<<" "<< fs::relative(path) <<"\n\t"<<e.what()<<"\n";
                result = false;
                res = BAD_RECURSIVE_DECL;
            }
        }
        return result;
    }
private:
    bool compile(ProcessedFile& file)
    {
        auto& ctx = file.fileCtx;
        auto& path = file.file_name;
        std::cout<<"\t"<<fs::relative(path).generic_string()<<"\n";
        auto codeGen = std::make_shared<ExprCodeGen>(ctx);
        auto compiler = std::make_unique<CompilerVisitor>(ctx,codeGen);
        try
        {
            for ( auto& node : file.ast )
            {
                node->accept(compiler.get());
            }
        }
        catch ( std::exception& e ){
            std::cerr<<"[ERROR]"<<" "<< fs::relative(path) <<"\n\t"<<e.what()<<"\n";
            std::cout<<"Finished With Errors\n";
            res = BAD_COMPILING;
            return false;
        }
        
        bool result = true;

        for ( auto& file : compiler->compiled_functions )
        {
            fs::path file_path(settings.target +"/"+file.toPath());

            auto it = compiled_objects.find(file_path);

            if ( it != compiled_objects.end() )
            {
                std::cerr<<"[ERROR] Object Names Collision:\n"<<
                    "\t"<<"Object: "<<file_path.generic_string()<<"\n\t\tfrom source file: "<<path<<"\n"<<
                    "\t"<<"Is collising with: " << it->first.generic_string() <<"\n"<<
                    "\t\t"<<"from source file: "<< it->second.source<<"\n";
                res = OBJECTS_COLLISION;
                result = false;
                continue;
            }
            compiled_objects.emplace(file_path,Object{path,std::move(file)});
        }

        return result;
    }

    void printProcFiles()
    {
        for ( auto& [path,procFiles] : *modules ){
            auto ctx = procFiles.fileCtx;
            std::cout<<fs::relative(path)<<"\n";
            std::cout<<"\tTypes\n";

            for ( auto& name : ctx->getTypes() )
            {
                std::cout<<"\t\t"<<name<<"\n";
            }

            std::cout<<"\tStorage Declarations\n";

            for ( auto& [name,_] : ctx->getStorageDeclarations() )
            {
                std::cout<<"\t\t"<<name<<"\n";
            }

            std::cout<<"\tFunctions\n";

            for ( auto& [name,_] : ctx->getFuncDeclarations() )
            {
                std::cout<<"\t\t"<<name<<"\n";
            }

            std::cout<<"\tInlined Functions\n";

            for ( auto& [name,_] : ctx->getInlinedDeclarations() )
            {
                std::cout<<"\t\t"<<name<<"\n";
            }
            std::cout<<"\n\n\n";
        }
    }

public:

    bool compilingStage()
    {
        std::cout<<"[Compiling Stage]\n";
        bool result = true;
        for (auto& [_,file] : *modules)
        {
            if ( !compile(file) ) result = false;
        }
        return result;
    }

    void codeGenStage()
    {
        for ( auto& [path,object] : compiled_objects )
        {            
            fs::create_directories(path.parent_path());

            std::ofstream real_file(path);
            for ( auto & line : object.func.text )
            {
                real_file<<line<<"\n";
            } 
        }
    }

};