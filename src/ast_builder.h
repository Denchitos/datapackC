#include<filesystem>
#include<unordered_map>
#include<fstream>
#include"iterable_stack.h"
#include"empty_visitor.h"
#include"tree/node.h"
#include"lexer.h"
#include"parser.h"

#include"tree/import.h"

#include"modules.h"


class FilesASTBuilder : public EmptyVisitor
{
    std::shared_ptr<Modules> built_files = nullptr;
    fs::path current_file;

    std::shared_ptr<Parser> parser;
    std::shared_ptr<Lexer> lexer;

    std::unordered_map<fs::path,std::unordered_set<fs::path>> dependencies_graph;
public:
    FilesASTBuilder(std::shared_ptr<Lexer> lexer,std::shared_ptr<Parser> parser) : lexer(lexer) , parser(parser)
    {
        built_files = std::make_shared<Modules>();
    }
    FilesASTBuilder(std::shared_ptr<Modules> built_files,std::shared_ptr<Lexer> lexer,std::shared_ptr<Parser> parser) : built_files(built_files) , lexer(lexer) , parser(parser)
    {
        built_files = std::make_shared<Modules>();
    }
    bool buildFile(fs::path path)
    {

        if ( !path.is_absolute() ) path = fs::absolute(path);

        if ( built_files->contains(path) ) return false;

        std::ifstream file(path);

        if ( !file.is_open())
        {
            std::cerr<<"Файл "<<path.generic_string()<<" не найден. Был импортирован из "<<fs::relative(current_file)<<"\n";
            return false;
        }
        std::string line;
        std::string text;
        while ( std::getline(file,line) )
        {
            text+=line+'\n';
        }

        std::cout<<"LEXING... "<<fs::relative(path).generic_string()<<"\n";

        lexer->setText(std::move(text));
        auto tokens = lexer->lex();

        std::cout<<"PARSING... "<<fs::relative(path).generic_string()<<"\n";

        parser->setCurrentFile(path);
        parser->setTokens(std::move(tokens));

        auto ast = parser->parse();

        auto& parse_msg = parser->getMessages();

        if ( !parse_msg.empty() )
        {
            std::cerr<<"ERROR OCCURED WHILE PARSING:\n  "<<fs::relative(path).generic_string()<<":\n\n";
            for ( auto& it : parse_msg )
            {
                std::cerr<<"        ["<<it.token.row<<" - "<<it.token.col<<" - "<< it.token.tok << "] "<<it.msg<<"\n";
            }
            return false;
        }

        ProcessedFile m{path,std::move(ast)};

        built_files->emplace(path,std::move(m));


        auto old_current = current_file;
        current_file = path;

        for ( auto & it : built_files->at(path).ast )
        {
            it->accept(this);
        }

        current_file = old_current;

        return true;
    }

    void visit(tree::Import* imp)
    {
        if ( current_file == imp->file_name )
        {

            std::cerr<<"Файл "<<fs::relative(current_file).generic_string() << " импортирует сам себя\n    Этот импорт не засчитается\n";
            imp->bad_import = true;
            return;
        }
        dependencies_graph[current_file].insert(imp->file_name);

        if ( !built_files->contains(imp->file_name) ) buildFile(imp->file_name);
    }

    void printDependenciesGraph()
    {
        std::unordered_set<fs::path> used_files;
        std::stack<fs::path> stack;
        for ( auto& [path , depends] : dependencies_graph )
        {  
            std::cout<<fs::relative(path)<<"\n    ";
            for ( auto& imported : depends  )
            {
                std::cout<<fs::relative(imported)<<"\n";
            }
        }
    }

    std::shared_ptr<Modules> getProcFiles()
    {
        return built_files;
    }

private:
    enum class Color
    {
        Gray,Black,White
    };
public:
    class RecursionFinder
    {
        enum class Color
        {
            Gray,Black,White
        };
        std::unordered_map<fs::path,Color> colors;
        std::vector<fs::path> found_cycles;

        IterableStack<fs::path> current_pathes;

        std::unordered_map<fs::path,std::unordered_set<fs::path>>& dependencies;
        // true - has recursion , false - not recursive

        Color& getColor(const fs::path& path)
        {
            auto it = colors.find(path);

            if ( it == colors.end() )
            {
                colors[path] = Color::White;
            }
            
            return colors[path];
        }

        bool dfs(const fs::path& path)
        {
            auto& color = getColor(path);

            bool found = false;

            if ( color == Color::Black) return found;

            else if ( color == Color::Gray ){ 
                current_pathes.emplace(path);
                found = true;
            }

            else if ( color == Color::White )
            {
                color = Color::Gray;
                for ( auto & child : dependencies[path] )
                {
                    if ( child == path )
                    {
                        color = Color::Black;
                        found = true;
                        break;
                    }
                    else if ( dfs(child) ){
                        current_pathes.emplace(path);
                        found = true;
                        color = Color::White;
                        break;
                    }
                    else
                    {
                        color = Color::Black;
                    }
                }
            }

            return found;
        }
    public:
        RecursionFinder(std::unordered_map<fs::path,std::unordered_set<fs::path>>& dependencies) : dependencies(dependencies)
        {

        }

        std::vector<std::vector<fs::path>> findRecursion()
        {
            std::vector<std::vector<fs::path>> res;

            for ( auto& [ path,_ ] : dependencies )
            {   
                auto color = getColor(path);
                if ( color == Color::Black ) continue;
                
                if ( dfs( path ) )
                {
                    res.emplace_back();
                    res.back().reserve(current_pathes.size());
                    for ( auto& item : current_pathes )
                    {
                        res.back().emplace_back(item);
                    }
                    res.back().emplace_back(current_pathes[0]);
                    current_pathes.clear();
                }
            }

            colors.clear();
            found_cycles.clear();

            return res;
        }
    };

    std::vector<std::vector<fs::path>> findCyclicImport()
    {
        RecursionFinder recF( dependencies_graph );
        return recF.findRecursion();


    }
};