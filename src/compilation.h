#pragma once

#include<memory>
#include<vector>
#include<string>
#include<iostream>
#include<optional>
#include<unordered_set>
#include"types.h"
#include"iterable_stack.h"
struct Symbol
{
    std::vector<std::string> value;
    Type type;
    bool mut = false;
};

struct Value
{
    std::vector<std::string> value;
    Type type;
    std::string symbol_name;

    Value(Type type) : type(type) {}
    Value(const std::vector<std::string>& value,Type type,const std::string& sym = "") : value(value),type(type),symbol_name(sym) {}
    Value(std::vector<std::string>&& value,Type type,const std::string& sym = "") : value(std::move(value)),type(type),symbol_name(sym) {}
    Value(const std::string& value,Type type,const std::string& sym = "") : value({value}),type(type),symbol_name(sym) {}
    Value(std::string&& value,Type type,const std::string& sym = "") : value({value}),type(type),symbol_name(sym) {}

    std::string to_string() const
    {
        auto r = value[0];
        auto s = value.size();
        for ( size_t i = 1 ; i < s ; i++ )
        {
            r+=(" "+value[i]);
        }
        return r;
    }
};

using SymPtr = std::unique_ptr<Symbol>;

struct Namespace
{
    std::string name;
    std::vector<std::string> path;

    Namespace(const std::string& name): name(name) {}
    Namespace(const std::string& name,const std::vector<std::string>& path): name(name) , path(path) {}

    void push(const std::string&);
    std::string toMinecraftPath()const;
    std::string toPath(const std::string& where)const;
    std::string toDotPath() const;
};

struct Function
{
    Namespace nmsp;
    std::string name = "func";
    std::vector<std::string> text;
    std::size_t control_block_counter = 0;
    
    Function() = default;

    Function(const Namespace& nmsp,const std::string& name) : nmsp(nmsp),name(name) {}

    const std::vector<std::string>& getText(){ return text; }
    
    void print(const std::string&);
    
    void print(std::string&&);

    void println(const std::string&);
    
    void println(std::string&&);
    
    const std::string& getName() const { return name ;}

    std::string toMinecraftPath() const;
    
    std::string toPath() const;
    
    std::string toDotPath() const;
    
    void show() const
    {
        std::cout<<"/function "<<toMinecraftPath()<<"\n";
        std::cout<<"in path: "<<toPath()<<"\n";

        std::cout<<"TEXT\n";
        
        for (auto& it : text)
        {
            std::cout<<'\t'<<it<<"\n";
        }
    }

};

struct FunctionDeclaration
{
    std::string minecraft_name;
    std::vector<Value> args;
    std::optional<Value> ret = std::nullopt;

    FunctionDeclaration() = default;
    FunctionDeclaration(const std::string& m_name) : minecraft_name(m_name) {}
    FunctionDeclaration(const std::string& m_name,std::vector<Value>&& args) : minecraft_name(m_name) {}
};

struct StorageDeclaration
{
    std::vector<std::string> field_names;
    std::unordered_map<std::string,Type> fields;

    bool hasField(const std::string& field) const
    {
        return fields.contains(field);
    }
};

struct FileCompilationContext
{
private:
    std::optional<Function> load_file;
    std::optional<Function> tick_file;

    IterableStack<Function> function_stack;
    
    std::unordered_map<std::string,FunctionDeclaration> declared_functions;
    std::unordered_map<std::string,StorageDeclaration > declared_storages;

    std::unordered_map<std::string,Type> str_to_type;
    
    std::unordered_map<Type,std::string> type_to_str;
    
    std::unordered_set<std::string> declared_types;

    int types_counter = 0;
public:

    IterableStack<std::unordered_map<std::string,Symbol>> symbols;
    IterableStack<std::string> while_break_stack;
    size_t tempVarNumber = 0;
    Namespace currentNamespace = {"minecraft"};

    FileCompilationContext();

    bool hasLoadFile() const { return load_file.has_value();}
    bool hasTickFile() const { return tick_file.has_value();}

    Function& getLoadFile() { return load_file.value();}
    Function& getTickFile() { return tick_file.value();}

    void printToLoad(const std::string& line)
    {
        if ( !load_file.has_value() )
        {
            load_file = {currentNamespace,"load"};
        }  
        load_file->println(line);

    }

    void printToTick(const std::string& line)
    {
        if ( !tick_file.has_value() )
        {
            tick_file = {currentNamespace,"tick"};
        }
        tick_file->println(line);
    }



    bool hasType(const std::string& type)
    {
        return Type::hasType(type) || declared_types.contains(type);
    }

    bool registerType(const std::string& type)
    {
        if ( hasType(type) ) return false;
        
        auto struct_id = types_counter++;

        str_to_type[type] = {Type::struct_t,struct_id};
        type_to_str[{Type::struct_t,struct_id}] = type;

        declared_types.emplace(type);
        return true;
    }
    Type strToType(const std::string& type)
    {
        if ( Type::hasType(type) ) return Type::StrToType(type).value();

        auto it = str_to_type.find(type);

        if ( it != str_to_type.end() ) return (*it).second;

        throw std::runtime_error("Couldn't converse "+ type + " to Type");
    }

    std::string typeToStr(const Type& type)
    {
        auto val = Type::toStr(type);
        if ( val.has_value() ) return val.value();        
        if (type_to_str.contains(type)) return type_to_str[type];
        else return "[UNDEFINED TYPE]";
    }

    std::vector<std::string> nameMangle(const Type& type,const std::string& name);
    static std::vector<std::string> nameMangle(Function& func,const Type& type,const std::string& name);

    Function& realFunction();

    Function& currentFunction();
    
    void newFunction(const std::string& name);
    
    void extendFunction(const std::string& name);
    
    void upFunction();
    
    Value getReturnConditionValue()
    {
        return {nameMangle(realFunction(),Type::score,"__is_returned"),Type::score};
    }
    std::string getTempVar()
    {
        return "__temp"+std::to_string(tempVarNumber++);
    }
    std::vector<std::string> getTempScoreVar();
    
    Symbol& getSymbol(const std::string& s)
    {
        auto last = static_cast<long long>(symbols.size()) - 1;
        while ( last>=0 )
        {
            if (symbols[last].contains(s)) return symbols[last].at(s);
            --last;
        }
        throw std::runtime_error("Unknown Symbol [" + s + "]");
    }
    void registerVar(const std::string&,const Symbol&);

    bool registerFunc(const std::string& name,FunctionDeclaration&& funcDecl)
    {
        if ( declared_functions.contains(name) ) return false;
        declared_functions[name] = std::move(funcDecl);
        return true;
    }

    bool registerStorage(const std::string& name,StorageDeclaration&& storageDecl)
    {
        if ( declared_functions.contains(name) ) return false;
        declared_storages[name] = std::move(storageDecl);
        return true;
    }

    bool hasFunc(const std::string& name) const
    {
        return declared_functions.contains(name);
    }

    bool hasStorage(const std::string& name) const
    {
        return declared_storages.contains(name);
    }
    bool hasStorage(const Type& type)
    {
        auto name = typeToStr(type);
        return declared_storages.contains(name);
    }

    StorageDeclaration& getStorageDecl(const std::string& name)
    {
        if ( declared_storages.contains(name) ) return declared_storages.at(name);
        else throw std::runtime_error("Unknown string "+ name + " while trying to converse it to StorageDecl ");
    }

    StorageDeclaration& getStorageDecl(const Type& type)
    {
        auto name = typeToStr(type);
        return getStorageDecl(name);
    }

    FunctionDeclaration& getFuncDecl(const std::string& name)
    {
        return declared_functions.at(name);
    }

    void print(const std::string&);
    void print(std::string&&);

    void println(const std::string&);
    void println(std::string&&);
};