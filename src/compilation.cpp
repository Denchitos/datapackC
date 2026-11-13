#include"compilation.h"


void Namespace::push(const std::string& s)
{
    path.emplace_back(s);
}

std::string Namespace::toMinecraftPath() const
{    
    auto result = name + ":";
    if ( path.empty() ) return result;
    auto size = path.size();
    for ( size_t i = 0 ; i<size ; i++ )
    {
        result+=path[i];
        result+='/';
    }

    return result;

}

std::string Namespace::toPath(const std::string& s) const
{
    auto result = name+'/'+s+'/';
    if ( path.empty() ) return result;
    auto size = path.size();
    for ( size_t i = 0 ; i<size ; i++ )
    {
        result+=path[i];
        result+='/';
    }
    return result;
}

std::string Namespace::toDotPath() const
{
    if ( path.empty() ) return name;
    auto result = name;
    auto size = path.size();
    for ( size_t i = 0 ; i<size ; i++ )
    {
        result+='.';
        result+=path[i];
    }
    return result;
}

void Function::print(std::string&& s)
{
    if ( text.empty() ) text.push_back("");
    text.back().append(std::move(s));
}

void Function::print(const std::string& s)
{
    if ( text.empty() ) text.push_back("");
    text.back().append(s);
}

void Function::println(std::string&& s)
{
    print(std::move(s));
    text.push_back("");
}

void Function::println(const std::string& s)
{
    print(s);
    text.push_back("");
}

std::string Function::toPath() const
{
    return nmsp.toPath("function")+name+".mcfunction";
}

std::string Function::toMinecraftPath() const
{
    return nmsp.toMinecraftPath()+name;
}

std::string Function::toDotPath() const
{
    return nmsp.toDotPath()+'.'+name;
}

FileCompilationContext::FileCompilationContext()
{
    symbols.emplace();
}

void FileCompilationContext::registerVar(const std::string& lval, const Symbol& symbol)
{
    symbols.top()[lval] = symbol;
}

Function& FileCompilationContext::realFunction()
{
    if ( function_stack.empty() ) throw std::runtime_error("Function Stack is empty now"); 
    return function_stack[0];
}

Function& FileCompilationContext::currentFunction()
{
    if ( function_stack.empty() ) throw std::runtime_error("Function Stack is empty now"); 
    return function_stack.top();
}

void FileCompilationContext::print(const std::string&s)
{
    currentFunction().print(s);
}
void FileCompilationContext::print(std::string&& s)
{
    currentFunction().print(std::move(s));
}

void FileCompilationContext::println(const std::string&s)
{
    currentFunction().println(s);
}
void FileCompilationContext::println(std::string&& s)
{
    currentFunction().println(std::move(s));
}

void FileCompilationContext::extendFunction(const std::string& name)
{
    auto new_nmsp = currentFunction().nmsp;

    new_nmsp.push(currentFunction().getName());
    
    function_stack.emplace(new_nmsp,name);
    symbols.emplace();
}

void FileCompilationContext::newFunction(const std::string& name)
{
    function_stack.emplace(currentNamespace,name);
    symbols.emplace();
}

void FileCompilationContext::upFunction()
{
    function_stack.pop();
    symbols.pop();
}

std::vector<std::string> FileCompilationContext::getTempScoreVar()
{
    return {"."+currentFunction().toDotPath()+"."+getTempVar(),"local"};
}

std::vector<std::string> FileCompilationContext::nameMangle(const Type& type,const std::string& name)
{
    if ( type.type==Type::score || type.type == Type::score_bool)
    {
        return {"."+currentFunction().toDotPath()+'.'+name,"local"};
    }
    if ( type.type==Type::int_t || type.type == Type::float_t || type.type == Type::bool_t || type.type == Type::string || type.type == Type::struct_t )
    {
        return { "storage",currentFunction().toMinecraftPath(),name };
    }
    
    else return {name};
}

std::vector<std::string> FileCompilationContext::nameMangle(Function& func,const Type& type,const std::string& name)
{
    if ( type.type==Type::score || type.type == Type::score_bool)
    {
        return {"."+func.toDotPath()+'.'+name,"local"};
    }
    if ( type.type==Type::int_t || type.type == Type::float_t || type.type == Type::bool_t || type.type == Type::string )
    {
        return { "storage",func.toMinecraftPath(),name };
    }

    else return {name};
}
