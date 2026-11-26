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

//const auto Error = "[ERROR]";

#define Error(s) "\033[31m"<<s<<"\033[0m"

#define Info(s) "\033[36m"<<s<<"\033[0m"

#define Success(s) "\033[32m"<<s<<"\033[0m"

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
inline int RECURSION_DETECTED = 9;
inline int BAD_CALL_TREE = 10;

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
        returnChecking();
        if (!forwardDefining()) return getRet();
        //printProcFiles();
        
        if (!recursiveDefining()) return getRet();
        //printProcFiles();
        
        if ( !recursionChecking() ) return getRet(); 

        if ( !compilingStage() ) return getRet();
        //printProcFiles();
        codeGenStage();

        return getRet();
    };

    int getRet()
    {
        if ( res == 0 )
            std::cout<<Success("=Project compiled succesfully=\n");
        else
            std::cout<<Error("=Project compiled with ERRORS=\n");
        std::cout<<Info("Exit Code: ")<<"\033[33m"<<res<<"\033[0m\n";
        return res;
    }

    bool buildStage()
    {
        std::cout<<Info("[Building Project Stage]\n");

        ast_builder->pushCandidate(settings.root_file);

        bool ast_building_success = true;

        while (true)
        {
            auto candidate = ast_builder->getNextCandidates();
            
            if ( !candidate.has_value() ) break;
            
            if ( !ast_builder->buildFile(candidate.value()) ) ast_building_success = false;
        }

        if ( !ast_building_success )
        {
            std::cerr<<Error("While build AST, Compiler got errors. Compilation stage is unavavilable\n");
            res = BAD_BUILD;
            return false;
        }

        std::cout<<Success("[AST BUILDING FINISHED]\n");

        return true;
    }

    bool forwardDeclaringStorages()
    {
        std::cout<<Info("[Forward Declaration Stage]\n");
        bool result = true;
        for (auto & [ path, file] : *modules )
        {
            std::cout<<"\t"<<fs::relative(path).generic_string()<<"\n";
            auto ctx = std::make_shared<FileCompilationContext>();

            ctx->currentNamespace = file.nmsp;
            ctx->currentDefinedNamespace = file.nmsp;

            auto forward = std::make_unique<ForwardStorageDeclaration>(ctx,modules);
            
            try
            {
                for ( auto& node : file.ast->storages )
                {
                    node->accept(forward.get());
                }
                for ( auto& node : file.ast->objectives )
                {
                    node->accept(forward.get());
                } 
            }
            catch ( std::exception& e ){
                std::cerr<<Error("[ERROR]")<<" "<< fs::relative(path) <<"\n\t"<<e.what()<<"\n";
                result = false;
                res = BAD_FORWARD_DECL;
            }

            std::cout<<"\tAll storages:\n";
            
            ctx->printDeclaredStorages();

            file.fileCtx = ctx;

        }

        return result;
    }

    bool recursiveStorageDecl()
    {
        std::cout<<Info("[Recursive Declaration Stage]\n");
        bool result = true;
        for (auto & [ path, file] : *modules )
        {
            std::cout<<"\t"<<fs::relative(path).generic_string()<<"\n";
            auto recursive = std::make_unique<RecursiveStorageDeclaration>(path,file.fileCtx,modules);
            try
            {
                for ( auto& node : file.ast->imports )
                {
                    node->accept(recursive.get());
                }
            }
            catch ( std::exception& e ){
                std::cerr<<Error("[ERROR]")<<" "<< fs::relative(path) <<"\n\t"<<e.what()<<"\n";
                result = false;
                res = BAD_RECURSIVE_DECL;
            }

            file.fileCtx->printDeclaredStorages();
        }
        return result;
    }

    bool forwardDefining()
    {
        std::cout<<Info("[Forward Defining Stage]\n");
        bool result = true;
        for (auto & [ path, file] : *modules )
        {
            std::cout<<"\t"<<fs::relative(path).generic_string()<<"\n";
            
            auto recursive = std::make_unique<ForwardDefiner>(file.fileCtx,modules);
            
            try
            {
                for ( auto& node : file.ast->inlines )
                {
                    node->accept(recursive.get());
                }
                for ( auto& node : file.ast->functions)
                {
                    node->accept(recursive.get());
                }
                for ( auto& node : file.ast->storages )
                {
                    node->accept(recursive.get());
                }    
            }
            catch ( std::exception& e ){
                std::cerr<<Error("[ERROR]")<<" "<< fs::relative(path) <<"\n\t"<<e.what()<<"\n";
                result = false;
                res = BAD_FORWARD_DEF;
            }

            file.fileCtx->printDefinedStorages();

        }
        return result;
    }

    bool recursiveDefining()
    {
        std::cout<<Info("[Recursive Defining Stage]\n");
        bool result = true;
        for (auto & [ path, file] : *modules )
        {
            std::cout<<"\t"<<fs::relative(path).generic_string()<<"\n";
            auto recursive = std::make_unique<RecursiveDefiner>(path,file.fileCtx,modules);
            try
            {
                for ( auto& node : file.ast->imports )
                {
                    node->accept(recursive.get());
                }    
            }
            catch ( std::exception& e ){
                std::cerr<<Error("[ERROR]")<<" "<< fs::relative(path) <<"\n\t"<<e.what()<<"\n";
                result = false;
                res = BAD_RECURSIVE_DECL;
            }

            file.fileCtx->printDefinedStorages();
        }
        return result;
    }
private:
    void visit_all(Visitor* visitor,ProcessedFile& file)
    {
        for ( auto& node : file.ast->imports )
        {
            node->accept(visitor);
        }
        for ( auto& node : file.ast->storages)
        {
            node->accept(visitor);
        }
        for ( auto& node : file.ast->functions )
        {
            node->accept(visitor);
        }
        for ( auto& node : file.ast->inlines )
        {
            node->accept(visitor);
        }

    }

    bool compile(ProcessedFile& file)
    {
        auto& ctx = file.fileCtx;
        auto& path = file.file_name;
        std::cout<<"\t"<<fs::relative(path).generic_string()<<"\n";
        auto codeGen = std::make_shared<ExprCodeGen>(ctx);
        auto compiler = std::make_unique<CompilerVisitor>(ctx,codeGen);
        try
        {
            visit_all(compiler.get(),file);
        }
        catch ( std::exception& e ){
            std::cerr<<Error("[ERROR]")<<" "<< fs::relative(path) <<"\n\t"<<e.what()<<"\n";
            std::cout<<Error("Finished With Errors\n");
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
                std::cerr<<Error("[ERROR]")<<"Object Names Collision:\n"<<
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

    void returnChecking()
    {
        ReturnChecker retChecker;
        for ( auto& [path,file] : *modules  )
        {
            visit_all(&retChecker,file);
        }
    }

    bool recursionChecking()
    {
        std::cout<<Info("[Checking for recursion]\n");
        auto result = true;
        CallTreeBuilder recChecker(nullptr);
        for ( auto& [path,file] : *modules )
        {
            std::cout<<"\t"<<path.generic_string()<<"\n";
            recChecker.setFileCtx(file.fileCtx);
            try {
                visit_all(&recChecker,file);
            }
            catch ( std::exception& e)
            {
                std::cerr<<Error("[ERROR]")<<" "<< fs::relative(path) <<"\n\t"<<e.what()<<"\n";
                res = BAD_CALL_TREE;
                result = false;
            }
        }

        auto rec = recChecker.findRecursion();
        if ( rec.empty() ) return result;

        result = false;
        res = RECURSION_DETECTED;
        std::cout<<Error("[ERROR]")<<" Detected Recursion:" << "\n";

        for ( auto& it : rec )
        {
            std::cout<<it<<"\n";
        }
    

        return result;
    }

    bool compilingStage()
    {
        std::cout<<Info("[Compiling Stage]\n");
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