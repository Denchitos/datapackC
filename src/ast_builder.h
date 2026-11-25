#include<queue>
#include<filesystem>
#include<unordered_map>
#include<fstream>
#include<expected>
#include"iterable_stack.h"
#include"empty_visitor.h"
#include"tree/node.h"
#include"lexer.h"
#include"parser.h"
#include"tree/import.h"
#include"modules.h"
#include"tree/namespace.h"


class FilesASTBuilder : public EmptyVisitor
{
    std::shared_ptr<Modules> built_files = nullptr;
    fs::path current_file;

    std::shared_ptr<Parser> parser;
    std::shared_ptr<Lexer> lexer;

    std::queue<fs::path> build_candidates;

    std::unordered_map<fs::path,std::unordered_set<fs::path>> dependencies_graph;

    std::optional<Namespace> current_nmsp = std::nullopt;


    std::expected<std::string,std::string> readFile(const fs::path& path)
    {
        std::ifstream file(path);

        if ( !file.is_open())
        {
            return std::unexpected("File \033[33m"+fs::relative(path).generic_string()+"\033[0m can not be open.\n\t It was imported from: \033[33m"+fs::relative(current_file).generic_string()+"\033[0m\n");
        }

        std::string line;
        std::string text;
        while ( std::getline(file,line) )
        {
            text+=line+'\n';
        }
        
        return text;
    }

    auto makeAST(std::string&& text,const fs::path& path)
    {
        std::cout<<"LEXING... "<<fs::relative(path).generic_string()<<"\n";

        lexer->setText(std::move(text));
        auto tokens = lexer->lex();

        std::cout<<"PARSING... "<<fs::relative(path).generic_string()<<"\n";

        parser->setCurrentFile(path);
        parser->setTokens(std::move(tokens));

        return parser->parse(true);
    }

public:
    FilesASTBuilder(std::shared_ptr<Lexer> lexer,std::shared_ptr<Parser> parser) : lexer(lexer) , parser(parser)
    {
        built_files = std::make_shared<Modules>();
    }
    
    FilesASTBuilder(std::shared_ptr<Modules> built_files,std::shared_ptr<Lexer> lexer,std::shared_ptr<Parser> parser) : built_files(built_files) , lexer(lexer) , parser(parser)
    {
        built_files = std::make_shared<Modules>();
    }
    
    std::optional<fs::path> getNextCandidates()
    {
        if ( build_candidates.empty() ) return std::nullopt;

        auto can = build_candidates.front();

        build_candidates.pop();
        return can;
    }

    void pushCandidate(fs::path path)
    {
        if ( !path.is_absolute() ) path = fs::absolute(path);

        if ( built_files->contains(path) ) return;

        build_candidates.push(path);
    }

    bool buildFile(fs::path path)
    {
        if ( !path.is_absolute() ) path = fs::absolute(path);

        if ( built_files->contains(path) ) return false;

        auto read = readFile(path);
        
        if ( !read )
        {
            std::cerr<<"\t"<<read.error();
            return false;
        }

        auto& text = read.value(); 
        
        auto ast = makeAST(std::move(text),path);

        auto parse_msg = parser->getMessages();

        if ( !parse_msg.empty() )
        {
            std::cerr<<"ERROR OCCURED WHILE PARSING:\n  "<<fs::relative(path).generic_string()<<":\n\n";
            for ( auto& it : parse_msg )
            {
                std::cerr<<"        ["<<it.token.row<<" - "<<it.token.col<<" - "<< it.token.tok << "] "<<it.msg<<"\n";
            }
            return false;
        }
        
        current_file = path;
        
        current_nmsp = std::nullopt;

        if ( ast->nmsp ) ast->nmsp->accept(this);  

        for ( auto & it : ast->imports )
        {
            it->accept(this);
        }

        if ( !current_nmsp.has_value() )
        {
            std::cerr<<"[ERROR] File "<<fs::relative(path)<<" has not namespace declaration ( Compilation stage is unavailable )\n";
            return false;
        }

        ProcessedFile m{path,std::move(ast),nullptr,current_nmsp.value()};
        built_files->emplace(path,std::move(m));

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

        if ( !built_files->contains(imp->file_name) ) build_candidates.push(imp->file_name);
    }

    void visit(tree::Namespace* nmsp)
    {
        if ( current_nmsp == std::nullopt )
        {
            Namespace new_nmsp(nmsp->namespace_);

            new_nmsp.path.reserve(nmsp->path.size());

            for ( auto& it : nmsp->path )
            {
                new_nmsp.path.emplace_back(it);
            }

            current_nmsp = new_nmsp;
        }
        else
        {
            std::cerr<<nmsp->getRowCol()<<" Redeclaration of namespace in file " << fs::relative(current_file)<<"\n";
            current_nmsp = std::nullopt;
        }
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