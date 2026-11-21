#pragma once

#include<memory>
#include<vector>
#include<string>
#include<iostream>
#include<optional>
#include<unordered_set>
#include"types.h"
#include"iterable_stack.h"
#include"tree/block.h"
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
    bool is_link = false;

    Value() = default;
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
    void pop()
    {
        path.pop_back();
    }
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
};

struct InlinedFunctionDeclaration
{
    std::string minecraft_name;
    std::vector<std::pair<std::string,Value>> args;
    tree::Block* body;
    InlinedFunctionDeclaration() = default;
    InlinedFunctionDeclaration(const std::string& m_name) : minecraft_name(m_name) {}
};

struct StorageDefinition
{
    std::vector<std::string> field_names;
    std::unordered_map<std::string,Type> fields;
    std::unordered_map<std::string,FunctionDeclaration> methods;
    std::unordered_map<std::string,InlinedFunctionDeclaration> inlined_methods;

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

    std::unordered_map<std::string,std::string> registered_objectives;
    
    
    std::unordered_map<std::string,InlinedFunctionDeclaration> declared_inlined_functions;
    
    std::unordered_map<std::string,FunctionDeclaration> declared_functions;
    
    std::unordered_map<std::string,StorageDefinition > declared_storages;

    
    std::unordered_map<std::string,Type> str_to_type;
    
    std::unordered_map<Type,std::string> type_to_str;
    
    std::unordered_set<std::string> declared_types;

    static int types_counter;
public:
    IterableStack<std::unordered_map<std::string,Symbol>> symbols;
    IterableStack<std::string> while_break_stack;
    size_t tempVarNumber = 0;
    Namespace currentDefinedNamespace = {"minecraft"};
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

    const auto& getTypes()
    {
        return declared_types;
    }

    bool isBadName(const std::string& name)
    {
        if ( name.contains(":") ) return false;
        return true;
    }

    bool hasType(const std::string& name)
    {
        auto type = resolveTypeName(name);
        return Type::hasType(type) || declared_types.contains(type);
    }

    std::string resolveName(const std::string& name) const
    {
        if ( !name.contains(":") ) return currentDefinedNamespace.toMinecraftPath()+name;
        return name;
    }

    std::string resolveTypeName(const std::string& name) const
    {
        if ( Type::hasType(name) ) return name;

        if ( !name.contains(":") ) return currentDefinedNamespace.toMinecraftPath()+name;
        return name;
    }


    void pushType(const std::string& name)
    {
        declared_types.emplace(name);
    }

    auto & get_member_str_to_type()
    {
        return str_to_type;
    }

    auto& get_member_type_to_str()
    {
        return type_to_str;
    }


    bool registerType(const std::string& name)
    {
        auto type = resolveTypeName(name);

        if ( hasType(type) ) return false;
        
        auto struct_id = types_counter++;

        str_to_type[type] = {Type::struct_t,struct_id};
        type_to_str[{Type::struct_t,struct_id}] = type;

        declared_types.emplace(type);
        return true;
    }
    
    Type strNotListToType(const std::string& name)
    {
        auto type = resolveTypeName(name);

        if ( Type::hasType(type) ) return Type::StrToType(type).value();
        
        auto it = str_to_type.find(type);

        if ( it != str_to_type.end() ) return (*it).second;

        throw std::runtime_error("Couldn't converse string '"+ type + "' to Type");
    }

    Type strToType(const std::string& name)
    {
        auto s = resolveTypeName(name);

        if ( !s.contains('[') ) return strNotListToType(s);

        auto p = s.find('[');

        Type type( strNotListToType(s.substr(0,p)) );

        for ( size_t i = p ; i < s.length() ;  i++)
        {
            if (s[i] == ']') type.list_depth++;
        }

        return type;
    }

    std::string typeToStr(const Type& type)
    {
        Type base_type = type;
        base_type.list_depth = 0;

        std::string result;
        auto val = Type::toStr(base_type);
        if ( val.has_value() ) result = val.value();        
        else if (type_to_str.contains(base_type)) result = type_to_str[base_type];
        else result = "[UNDEFINED TYPE]";

        for (size_t i = 0; i < type.list_depth; ++i) {
            result += "[]";
        }
        return result;
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


    auto& getFuncDeclarations()
    {
        return declared_functions;
    }


    bool registerFunc(const std::string& func_name,FunctionDeclaration&& funcDecl)
    {
        auto name = resolveName(func_name);
        if ( declared_inlined_functions.contains(name) || declared_functions.contains(name) ) return false;
        declared_functions[name] = std::move(funcDecl);
        return true;
    }
    bool hasFunc(const std::string& func_name) const
    {
        auto name = resolveName(func_name);
        return declared_functions.contains(name);
    }
    FunctionDeclaration& getFuncDecl(const std::string& func_name)
    {
        auto name = resolveName(func_name);
        return declared_functions.at(name);
    }


    auto& getInlinedDeclarations()
    {
        return declared_inlined_functions;
    }

    bool registerInlined(const std::string& func_name,InlinedFunctionDeclaration&& funcDecl)
    {
        auto name = resolveName(func_name);
        if ( declared_inlined_functions.contains(name) || declared_functions.contains(name) ) return false;
        declared_inlined_functions[name] = std::move(funcDecl);
        return true;
    }
    bool hasInlined(const std::string& func_name) const
    {
        auto name = resolveName(func_name);
        return declared_inlined_functions.contains(name);
    }

    InlinedFunctionDeclaration& getInlinedDecl(const std::string& func_name)
    {
        auto name = resolveName(func_name);
        return declared_inlined_functions.at(name);
    }

    bool registerObjective(const std::string& obj_name)
    {
        std::cout<<"registered objective:\n";
        auto name = resolveName(obj_name);
        std::cout<<name<<"\n";
        if ( registered_objectives.contains(name) ) return false;
        registered_objectives[name] = currentNamespace.toDotPath()+'.'+obj_name;
        return true;
    }

    bool hasObjective(const std::string& obj_name)
    {
        auto name = resolveName(obj_name);
        return registered_objectives.contains(name);
    }

    const std::string& getObjective(const std::string& obj_name)
    {
        auto name = resolveTypeName(obj_name);
        auto it = registered_objectives.find(name);
        if ( it == registered_objectives.end() ) throw std::runtime_error("Unknown objective '"+name+"'");
        return it->second;
    } 


    auto& getStorageDeclarations()
    {
        return declared_storages;
    }

    bool registerStorage(const std::string& stor_name,StorageDefinition&& storageDecl)
    {
        auto name = resolveTypeName(stor_name);
        if ( declared_functions.contains(name) ) return false;
        declared_storages[name] = std::move(storageDecl);
        return true;
    }

    bool hasStorage(const std::string& name) const
    {
        return declared_storages.contains(resolveTypeName(name));
    }
    bool hasStorage(const Type& type)
    {
        auto name = resolveTypeName(typeToStr(type));
        return declared_storages.contains(name);
    }

    StorageDefinition& getStorageDecl(const std::string& stor_name)
    {
        auto name = resolveTypeName(stor_name);
        if ( declared_storages.contains(name) ) return declared_storages.at(name);
        else throw std::runtime_error("Unknown string "+ name + " while trying to converse it to StorageDecl ");
    }
    
    StorageDefinition& getStorageDecl(const Type& type)
    {
        auto name = resolveTypeName(typeToStr(type));
        return getStorageDecl(name);
    }

    void print(const std::string&);
    void print(std::string&&);

    void println(const std::string&);
    void println(std::string&&);

    bool hasOperation(const std::string& op,const Type& t1,const Type& t2)
    {
        auto it = Type::builtin_operation_table.find({op,t1,t2});
        if (it != Type::builtin_operation_table.end()) return true;

        if ( t1.type == Type::struct_t && t2.type == Type::initializer_list ) return true;

        return false;
    }

    Type getOperationResult(const std::string& op,const Type& t1,const Type& t2)
    {
        auto it = Type::builtin_operation_table.find({op,t1,t2});
        if ( it != Type::builtin_operation_table.end() ) return it->second;;        

        if ( t1.type == Type::struct_t && t2.type == Type::initializer_list ) return t1;

        throw std::runtime_error("Unexpected Operation Triplet - ["+typeToStr(t1) + " " + op + " " + typeToStr(t2) +"]");
    }

    bool hasOperation(const std::string& op,const Type& t1)
    {
        auto it = Type::builtin_unary_operation_table.find({op,t1});
        if (it != Type::builtin_unary_operation_table.end()) return true;
        return false;
    }

    Type getOperationResult(const std::string& op,const Type& t1)
    {
        auto it =  Type::builtin_unary_operation_table.find({op,t1});
        if ( it != Type::builtin_unary_operation_table.end() ) return it->second;

        throw std::runtime_error("Unexpected Operation Duplet - [ "+op +" " + typeToStr(t1) +" ]");
    }
};