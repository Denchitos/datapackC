#include<filesystem>
#include<unordered_map>
#include<fstream>
#include"empty_visitor.h"
#include"tree/node.h"
#include"lexer.h"
#include"parser.h"

#include"tree/import.h"
namespace fs = std::filesystem;

using fs::path;

struct ProcessedFile
{
    fs::path file_name;
    std::vector<tree::NodePtr> ast;
};

class FilesASTBuilder : public EmptyVisitor
{
    std::unordered_map<fs::path,ProcessedFile> built_files;
    fs::path current_file;

    std::shared_ptr<Parser> parser;
    std::shared_ptr<Lexer> lexer;
public:
    FilesASTBuilder(std::shared_ptr<Lexer> lexer,std::shared_ptr<Parser> parser) : lexer(lexer) , parser(parser)
    {

    }
    bool buildFile(const std::string& file_name)
    {
        fs::path path = file_name;

        if ( !path.is_absolute() ) path = fs::absolute(path);

        if ( built_files.contains(path) )
        {
            std::cerr<<"File is built yet:\n    "<<path<<"\n";
            return false;
        }

        std::ifstream file(path);

        if ( !file.is_open())
        {
            std::cerr<<"Файл "<<file_name<<" Не найден";
            return false;
        }
        std::string line;
        std::string text;
        while ( std::getline(file,line) )
        {
            text+=line+'\n';
        }

        std::cout<<"LEXING... "<<path<<"\n";

        lexer->setText(std::move(text));
        auto tokens = lexer->lex();

        std::cout<<"PARSING... "<<path<<"\n";

        parser->setCurrentFile(path);
        parser->setTokens(std::move(tokens));

        auto ast = parser->parse();

        auto& parse_msg = parser->getMessages();

        if ( !parse_msg.empty() )
        {
            std::cerr<<"ERROR OCCURED WHILE PARSING:\n  "<<path<<":\n\n";
            for ( auto& it : parse_msg )
            {
                std::cerr<<"        ["<<it.token.row<<" - "<<it.token.col<<" - "<< it.token.tok << "] "<<it.msg<<"\n";
            }
            return false;
        }

        ProcessedFile m{path,std::move(ast)};

        built_files[path] = std::move(m);

        for ( auto & it : built_files[path].ast )
        {
            it->accept(this);
        }

        return true;
    }

    void visit(tree::Import* imp)
    {
        if ( !built_files.contains(imp->file_name) ) buildFile(imp->file_name.string());
    }

    std::unordered_map<fs::path,ProcessedFile>& getProcFiles()
    {
        return built_files;
    } 
};