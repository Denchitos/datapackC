#include<unordered_set>
#include<functional>
#include "ExprCodeGen.h"
#include "compilation.h"
#include"types.h"

#define GEN_OPERATOR(genOperation)\
Value ExprCodeGen::genOperation(const Value& lhs,const Value& rhs)\
{\
    std::pair pair{ lhs.type,rhs.type };\
    auto it =  genOperation##_map.find(pair);\
    if ( it != genOperation##_map.end() )\
    {\
        return (*it).second(fileCtx.get(),lhs,rhs);\
    }\
    else\
    {\
        throw std::runtime_error( #genOperation "(lhs,rhs) is undefined for Values: lhs - ["+fileCtx->typeToStr(lhs.type)+"] and rhs = ["+fileCtx->typeToStr(rhs.type)+"]" );\
    }\
}\

////USEFUL////USEFUL////USEFUL////USEFUL////USEFUL////USEFUL////USEFUL////USEFUL////USEFUL////
////USEFUL////USEFUL////USEFUL////USEFUL////USEFUL////USEFUL////USEFUL////USEFUL////USEFUL////
////USEFUL////USEFUL////USEFUL////USEFUL////USEFUL////USEFUL////USEFUL////USEFUL////USEFUL////
////USEFUL////USEFUL////USEFUL////USEFUL////USEFUL////USEFUL////USEFUL////USEFUL////USEFUL////

static Value genComparissonScoreScore(FileCompilationContext* fileCtx,const std::string& op,const Value& lhs,const Value& rhs)
{
    auto temp = fileCtx->getTempScoreVar();
    auto tmp = temp[0] +" "+ temp[1];
    if ( op == "==" ) fileCtx->println("execute store result score "+tmp+" run execute if score "+lhs.value[0] + " " + lhs.value[1]+" = "+rhs.value[0] + " " + rhs.value[1]);
    else if ( op=="!=") fileCtx->println("execute store result score "+tmp+" run execute unless score "+lhs.value[0] + " " + lhs.value[1]+" = "+rhs.value[0] + " " + rhs.value[1]);
    else fileCtx->println("execute store result score "+tmp+" run execute if score "+lhs.value[0] + " " + lhs.value[1]+" "+op+" "+rhs.value[0] + " " + rhs.value[1]);
    return {temp,Type::score_bool};
}

static Value genComparissonScoreNum(FileCompilationContext* fileCtx,const std::string& op,const Value& lhs,const Value& rhs)
{
    auto temp = fileCtx->getTempScoreVar();
    auto tmp = temp[0] +" "+ temp[1];
    auto rv = std::to_string(std::stoi(rhs.value[0]));
    if ( op == "==" ) fileCtx->println("execute store result score "+tmp+" run execute if score "+lhs.value[0] + " " + lhs.value[1]+" matches "+rv);
    else if ( op == "!=" ) fileCtx->println("execute store result score "+tmp+" run execute unless score "+lhs.value[0] + " " + lhs.value[1]+" matches "+rv);
    else if ( op=="!=") fileCtx->println("execute store result score "+tmp+" run execute unless score "+lhs.value[0] + " " + lhs.value[1]+" matches "+rv);
    else if ( op == ">" ) fileCtx->println("execute store result score "+tmp+" run execute unless score "+lhs.value[0] + " " + lhs.value[1]+" matches .."+rv );
    else if ( op == "<" ) fileCtx->println("execute store result score "+tmp+" run execute unless score "+lhs.value[0] + " " + lhs.value[1]+" matches "+rv+"..");
    else if ( op == ">=" ) fileCtx->println("execute store result score "+tmp+" run execute if score "+lhs.value[0] + " " + lhs.value[1]+" matches "+rv+"..");
    else if ( op == "<=" ) fileCtx->println("execute store result score "+tmp+" run execute if score "+lhs.value[0] + " " + lhs.value[1]+" matches .."+rv);
    return {temp,Type::score_bool};
}

static inline bool StrToBool(const std::string& b)
{
    return b == "true";
}

static inline std::string BoolToStr(bool b)
{
    return b ? "true" : "false";
}

static std::unordered_set<std::string> scoreboard_assigning_operators = {
    "=","+=","-=","*=","/=","%="
};

static std::unordered_set<std::string> scoreboard_operators = {
    "+","-","/","*","%"
};

static std::unordered_set<std::string> comparison_operators = {
    ">","<",">=","<=","==","!="
};

static inline std::string command_scoreboard_players(std::string op,const std::string& lhs,const std::string& rhs)
{
    int val = std::stoi(rhs);
    if ( op == "add" && val < 0){ op = "remove" ; val*=-1 ;}
    else if ( op == "remove" && val < 0 ){ op = "add" ; val*=-1 ; }
    return "scoreboard players " + op + " " + lhs + " " + std::to_string(val);
}

static inline std::string command_scoreboard_players_operation(const std::string& op,const std::string& lhs,const std::string& rhs)
{
    return "scoreboard players operation "+lhs+" "+op +" "+rhs;
}

////USEFUL////USEFUL////USEFUL////USEFUL////USEFUL////USEFUL////USEFUL////USEFUL////USEFUL////
////USEFUL////USEFUL////USEFUL////USEFUL////USEFUL////USEFUL////USEFUL////USEFUL////USEFUL////
////USEFUL////USEFUL////USEFUL////USEFUL////USEFUL////USEFUL////USEFUL////USEFUL////USEFUL////
////USEFUL////USEFUL////USEFUL////USEFUL////USEFUL////USEFUL////USEFUL////USEFUL////USEFUL////

using operator_form = Value(*) (FileCompilationContext*,const Value& lhs,const Value& rhs);

ExprCodeGen::ExprCodeGen(std::shared_ptr<FileCompilationContext> ctx): fileCtx(ctx)
{

}

Value ExprCodeGen::genBinaryOperator(const std::string& op, const Value& lhs,const Value& rhs)
{
    if ( op=="=" )
        return genAssign(lhs,rhs);
    else if ( op=="+" )
        return genAdd(lhs,rhs);
    else if ( op=="-" )
        return genSub(lhs,rhs);
    else if ( op=="*" )
        return genMul(lhs,rhs);
    else if ( op=="/" )
        return genDiv(lhs,rhs);
    else if ( op==">" )
        return genMore(lhs,rhs);
    else if ( op=="<" )
        return genLess(lhs,rhs);
    else if ( op=="<=" )
        return genLessEq(lhs,rhs);
    else if ( op==">=" )
        return genMoreEq(lhs,rhs);
    else if ( op=="==" )
        return genEquals(lhs,rhs);
    else if ( op=="&&" )
        return genAnd(lhs,rhs);
    else if ( op=="||" )
        return genOr(lhs,rhs);
    else if ( op=="%" )
        return genMod(lhs,rhs);
    else if ( op=="+=" )
        return genAddEq(lhs,rhs);
    else if ( op=="-=" )
        return genSubEq(lhs,rhs);
    else if ( op=="*=" )
        return genMulEq(lhs,rhs);
    else if ( op=="/=" )
        return genDivEq(lhs,rhs);
    else if ( op=="%=" )
        return genModEq(lhs,rhs);
    else
        throw std::runtime_error( "Unexpected expression triplet : "+op+" lhs - ["+fileCtx->typeToStr(lhs.type)+"] and rhs = ["+fileCtx->typeToStr(rhs.type)+"]" );
}

static Value genAssignStatic(FileCompilationContext* fileCtx,const  Value& lhs,const Value& rhs);

static std::unordered_map<std::pair<Type,Type>,operator_form>  genAssign_map{
    { {Type::score,Type::score},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs)
    {
        fileCtx->println("scoreboard players operation " + lhs.value[0] + " "  + lhs.value[1] + " = " + rhs.value[0]+ " "  + rhs.value[1] );
        return lhs;
    }},
    { {Type::score,Type::numberLiteral},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs)
    {
        fileCtx->println("scoreboard players set " + lhs.value[0] + " "  + lhs.value[1] + " " + std::to_string(std::stoi(rhs.value[0])) );
        return lhs;
    }},
    { {Type::selector,Type::rawSelector},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        fileCtx->getSymbol(lhs.symbol_name).value = rhs.value; 
        return {rhs.value,lhs.type.type, lhs.symbol_name};
    }},
    { {Type::selector,Type::selector},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        fileCtx->getSymbol(lhs.symbol_name).value = rhs.value; 
        return { rhs.value,lhs.type.type, lhs.symbol_name};
    }},
    { {Type::score_bool,Type::boolLiteral},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        if ( rhs.value[0] == "true" )
            fileCtx->println( "scoreboard players set " + lhs.value[0] + " "  + lhs.value[1] + " 1" );
        else
            fileCtx->println( "scoreboard players set " + lhs.value[0] + " "  + lhs.value[1] + " 0" );
        return lhs;
    }},
    { {Type::score_bool,Type::score_bool},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        fileCtx->println( "scoreboard players operation " + lhs.value[0] + " "  + lhs.value[1] + " = " + rhs.value[0] + " " + rhs.value[1] );
        return lhs;
    }},
    { {Type::string,Type::textLiteral},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        fileCtx->println("data modify "+ lhs.value[0] +" "+ lhs.value[1] + " " + lhs.value[2] + " set value \"" + rhs.value[0] + "\"" );
        return lhs;
    }},
    { {Type::string,Type::string},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        fileCtx->println("data modify "+ lhs.to_string() + " set from " + rhs.to_string() );
        return lhs;
    }},
    { { Type::int_t,Type::numberLiteral },[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        fileCtx->println("data modify "+lhs.to_string() + " set value " + std::to_string(std::stoi(rhs.to_string())) );
        return lhs;
    }},
    { { Type::int_t,Type::int_t },[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        fileCtx->println("data modify "+lhs.to_string() + " set from " + rhs.to_string() );
        return lhs;
    }},
    { {Type::struct_t,Type::struct_t},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        if ( lhs.type != rhs.type ) throw std::runtime_error( "Unexpected situation in assigning two structures; got different types" );
        fileCtx->println("data modify "+ lhs.to_string() + " set from " + rhs.to_string() );
        return lhs;
    }},
    { {Type::struct_t,Type::initializer_list}, [](FileCompilationContext* fileCtx,const Value& lhs, const Value& rhs ) -> Value
    {

        auto storage = fileCtx->getStorageDecl(lhs.type);
        
        size_t rhs_value_i = 0;
        auto size = rhs.value.size();

        for ( auto & field_name : storage.field_names )
        {

            /// { FIELD MAKING
            auto field_value = lhs.value;
            
            field_value.back()+=("."+field_name);
            
            Type field_type; 
            field_type = storage.fields[field_name];
            Value field { field_value , field_type };

            /// }

            /// { MAKING VALUE THAT ASSIGNS TO FIELD
            Type assignable_type = fileCtx->strToType(rhs.value[rhs_value_i + 1]);
            std::vector<std::string> assignable_value;
            size_t depth = 1;
            for ( size_t i = rhs_value_i + 2 ; i < size ; i++)
            {
                auto& str = rhs.value[i];
                if ( str != "<" && str != ">" )
                {
                    assignable_value.emplace_back(str);
                }
                else if ( str == "<" )
                {
                    depth++;
                    assignable_value.emplace_back("<");
                }
                else if ( str == ">" )
                {
                    depth--;
                    if ( depth == 0 )
                    {
                        rhs_value_i = i+1;
                        break;
                    }
                    else assignable_value.emplace_back(">");
                }
            }
            Value new_lhs{assignable_value,assignable_type};

            genAssignStatic(fileCtx,field,new_lhs);
        }
        return lhs;
    }}
};

static Value genAssignListInitList(FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs);

static Value genAssignStatic(FileCompilationContext* fileCtx,const  Value& lhs,const Value& rhs)
{

    std::pair<Type,Type> pair;

    if ( ( lhs.type.type == Type::struct_t ) && (lhs.type == rhs.type) ) {  pair = { Type::struct_t,Type::struct_t }; }
    else if ( ( lhs.type.type == Type::struct_t ) && ( rhs.type.type == Type::initializer_list) ) {  pair = { Type::struct_t,Type::initializer_list }; }
    else pair = { lhs.type,rhs.type };
    
    if ( lhs.type.list_depth != 0 && rhs.type.type == Type::initializer_list )
    {
        return genAssignListInitList(fileCtx,lhs,rhs);
    }

    if ( lhs.type.list_depth != 0 && rhs.type.list_depth == lhs.type.list_depth && lhs.type.type == rhs.type.type )
    {
        fileCtx->println( "data modify "+lhs.to_string() + " set value []" );
        fileCtx->println( "data modify "+lhs.to_string() + " set from " + rhs.to_string() );
        return lhs ;
    }

    auto it =  genAssign_map.find(pair);
    
    if ( it != genAssign_map.end() )
    {
        return (*it).second(fileCtx,lhs,rhs);
    }
    else
    {
        throw std::runtime_error("generateAssign(lhs,rhs) is undefined for Values: lhs - ["+fileCtx->typeToStr(lhs.type)+"] and rhs = ["+fileCtx->typeToStr(rhs.type)+"]" );
    }
}

static size_t getInitListLen(const Value& init_list)
{
    auto size = init_list.value.size();
    auto& value = init_list.value;
    size_t i = 0;
    size_t depth = 0;
    size_t result = 0;
    while ( i < size )
    {
        if ( value[i] == "<" )
        {
            depth++;
        }
        else if (value[i] == ">")
        {
            depth--;
            if ( depth == 0 ) result++;
        }
        i++;
    }
    return result;
}

static Value genAssignListInitList(FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs)
{       

    auto init_list_len = getInitListLen(rhs);

    std::string initialize;

    for ( size_t i = 0 ; i < init_list_len ; i++ )
    {
        initialize += "{},";
    }
    initialize.pop_back();

    fileCtx->println( "data modify " + lhs.to_string() + " set value ["+ initialize +"]" );

    size_t rhs_value_i = 0;
    auto size = rhs.value.size();
    size_t list_idx = 0;
    while ( rhs_value_i < size )
    {
        /// { FIELD MAKING
            auto field_value = lhs.value;
            
            field_value.back()+= ( "["+ std::to_string(list_idx++) +"]" );
            
            Type field_type; 
            field_type = lhs.type.getListDown();
            Value field { field_value , field_type };

            /// }

        /// { MAKING VALUE THAT ASSIGNS TO FIELD
        Type assignable_type = fileCtx->strToType(rhs.value[rhs_value_i + 1]);
        std::vector<std::string> assignable_value;
        size_t depth = 1;
        for ( size_t i = rhs_value_i + 2 ; i < size ; i++)
        {
            auto& str = rhs.value[i];
            if ( str != "<" && str != ">" )
            {
                assignable_value.emplace_back(str);
            }
            else if ( str == "<" )
            {
                depth++;
                assignable_value.emplace_back("<");
            }
            else if ( str == ">" )
            {
                depth--;
                if ( depth == 0 )
                {
                    rhs_value_i = i+1;
                    break;
                }
                else assignable_value.emplace_back(">");
            }
        }
        /// }

        Value new_lhs{assignable_value,assignable_type};

        genAssignStatic( fileCtx,field,new_lhs );
    }
    
    return lhs;
}

Value ExprCodeGen::genAssign(const Value& lhs,const Value& rhs)
{
    return genAssignStatic(fileCtx.get(),lhs,rhs);
}

static std::unordered_map<std::pair<Type,Type>,operator_form> genAnd_map{
    { {Type::boolLiteral,Type::boolLiteral},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        auto b1 = StrToBool(lhs.value[0]);
        auto b2 = StrToBool(rhs.value[0]);
        return {BoolToStr(b1&&b2),Type::boolLiteral};
    }},
    { {Type::score_bool,Type::score_bool},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        auto tmp = fileCtx->getTempScoreVar();
        fileCtx->println(
                "execute store result score "+tmp[0] + " " + tmp[1] +
                " run execute unless score "+lhs.value[0] + " " + lhs.value[1] +" matches 0 unless score "+
                rhs.value[0] + " " + rhs.value[1]+" matches 0"
        );
        return {tmp,Type::score_bool};
    }},
    { {Type::boolLiteral,Type::score_bool},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        auto b1 = StrToBool(lhs.value[0]);
        if ( b1 ) return rhs;
        else return lhs;
    }},
    { {Type::score_bool,Type::boolLiteral},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        return genAnd_map.at({Type::boolLiteral,Type::score_bool})(fileCtx,rhs,lhs);
    }},
};

GEN_OPERATOR(genAnd)

static std::unordered_map<std::pair<Type,Type>,operator_form> genOr_map{
    { {Type::boolLiteral,Type::boolLiteral},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        auto b1 = StrToBool(lhs.value[0]);
        auto b2 = StrToBool(rhs.value[0]);
        return {BoolToStr(b1||b2),Type::boolLiteral};
    }},
    { {Type::score_bool,Type::score_bool},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        auto tmp = fileCtx->getTempScoreVar();
        fileCtx->println(
            "scoreboard players operation "+tmp[0] + " " + tmp[1] +" = "+lhs.value[0] + " " + lhs.value[1]
        );
        fileCtx->println(
            "execute if score "+tmp[0] + " " + tmp[1] +" matches 0 run scoreboard players operation "+tmp[0] + " " + tmp[1] +" = "+rhs.value[0]+" "+rhs.value[1]
        );
            return {tmp,Type::score_bool};
    }},
    { {Type::boolLiteral,Type::score_bool},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        auto b1 = StrToBool(lhs.value[0]);
        if ( b1 ) return lhs;
        else return rhs;
    }},
    { {Type::score_bool,Type::boolLiteral},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        return genOr_map.at({Type::boolLiteral,Type::score_bool})(fileCtx,rhs,lhs);
    }},
};

GEN_OPERATOR(genOr)

static std::unordered_map<std::pair<Type,Type>,operator_form> genMore_map{
    { {Type::score,Type::score},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        return genComparissonScoreScore(fileCtx,">",lhs,rhs);
    }},
    { {Type::score,Type::numberLiteral},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        return genComparissonScoreNum(fileCtx,">",lhs,rhs);
    }},
    { {Type::numberLiteral,Type::score},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        return genComparissonScoreNum(fileCtx,"<",rhs,lhs);
    }},
    { {Type::numberLiteral,Type::numberLiteral},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        return {{BoolToStr(std::stod(lhs.value[0]) > std::stod(rhs.value[0]))},Type::boolLiteral};
    }},
};

GEN_OPERATOR(genMore)

static std::unordered_map<std::pair<Type,Type>,operator_form> genMoreEq_map{
    { {Type::score,Type::score},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        return genComparissonScoreScore(fileCtx,">=",lhs,rhs);
    }},
    { {Type::score,Type::numberLiteral},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        return genComparissonScoreNum(fileCtx,">=",lhs,rhs);
    }},
    { {Type::numberLiteral,Type::score},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        return genComparissonScoreNum(fileCtx,"<=",rhs,lhs);
    }},
    { {Type::numberLiteral,Type::numberLiteral},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        return {{BoolToStr(std::stod(lhs.value[0]) >= std::stod(rhs.value[0]))},Type::boolLiteral};
    }},
};

GEN_OPERATOR(genMoreEq)

static std::unordered_map<std::pair<Type,Type>,operator_form> genLess_map{
    { {Type::score,Type::score},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        return genComparissonScoreScore(fileCtx,"<",lhs,rhs);
    }},
    { {Type::score,Type::numberLiteral},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        return genComparissonScoreNum(fileCtx,"<",lhs,rhs);
    }},
    { {Type::numberLiteral,Type::score},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        return genComparissonScoreNum(fileCtx,">",rhs,lhs);
    }},
    { {Type::numberLiteral,Type::numberLiteral},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        return {{BoolToStr(std::stod(lhs.value[0]) < std::stod(rhs.value[0]))},Type::boolLiteral};
    }},
};

GEN_OPERATOR(genLess)

static std::unordered_map<std::pair<Type,Type>,operator_form> genLessEq_map{
    { {Type::score,Type::score},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        return genComparissonScoreScore(fileCtx,"<=",lhs,rhs);
    }},
    { {Type::score,Type::numberLiteral},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        return genComparissonScoreNum(fileCtx,"<=",lhs,rhs);
    }},
    { {Type::numberLiteral,Type::score},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        return genComparissonScoreNum(fileCtx,">=",rhs,lhs);
    }},
    { {Type::numberLiteral,Type::numberLiteral},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        return {{BoolToStr(std::stod(lhs.value[0]) <= std::stod(rhs.value[0]))},Type::boolLiteral};
    }},
};

GEN_OPERATOR(genLessEq)

static std::unordered_map<std::pair<Type,Type>,operator_form> genEquals_map{
    { {Type::score,Type::score},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        return genComparissonScoreScore(fileCtx,"==",lhs,rhs);
    }},
    { {Type::score,Type::numberLiteral},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        return genComparissonScoreNum(fileCtx,"==",lhs,rhs);
    }},
    { {Type::numberLiteral,Type::score},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        return genComparissonScoreNum(fileCtx,"==",rhs,lhs);
    }},
    { {Type::numberLiteral,Type::numberLiteral},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        return {{BoolToStr(std::stod(lhs.value[0]) == std::stod(rhs.value[0]))},Type::boolLiteral};
    }},
};

GEN_OPERATOR(genEquals)

static std::unordered_map<std::pair<Type,Type>,operator_form> genNotEquals_map{
    { {Type::score,Type::score},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        return genComparissonScoreScore(fileCtx,"!=",lhs,rhs);
    }},
    { {Type::score,Type::numberLiteral},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        return genComparissonScoreNum(fileCtx,"!=",lhs,rhs);
    }},
    { {Type::numberLiteral,Type::score},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        return genComparissonScoreNum(fileCtx,"!=",rhs,lhs);
    }},
    { {Type::numberLiteral,Type::numberLiteral},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        return {{BoolToStr(std::stod(lhs.value[0]) != std::stod(rhs.value[0]))},Type::boolLiteral};
    }},
};

GEN_OPERATOR(genNotEquals)

static std::unordered_map<std::pair<Type,Type>,operator_form> genAdd_map{
    { {Type::score,Type::score},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        auto tmp = fileCtx->getTempScoreVar();
        fileCtx->println( command_scoreboard_players_operation("=",tmp[0]+" "+tmp[1],lhs.value[0]+" "+lhs.value[1]));
        fileCtx->println( command_scoreboard_players_operation("+=",tmp[0]+" "+tmp[1],rhs.value[0]+" "+rhs.value[1]));
        return {tmp,Type::score};
    }},
    { {Type::score,Type::numberLiteral},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        auto tmp = fileCtx->getTempScoreVar();
        fileCtx->println( command_scoreboard_players_operation("=",tmp[0]+" "+tmp[1],lhs.value[0]+" "+lhs.value[1]));
        fileCtx->println( command_scoreboard_players("add",tmp[0]+" "+tmp[1],rhs.value[0]));
        return {tmp,Type::score};
    }},
    { {Type::numberLiteral,Type::score},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        return genAdd_map.at({Type::score,Type::numberLiteral})(fileCtx,rhs,lhs);
    }},
    { {Type::numberLiteral,Type::numberLiteral},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        return { std::to_string(std::stod(lhs.value[0])+std::stod(rhs.value[0])) ,Type::numberLiteral };
    }},
};

GEN_OPERATOR(genAdd)


static std::unordered_map<std::pair<Type,Type>,operator_form> genSub_map{
    { {Type::score,Type::score},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        auto tmp = fileCtx->getTempScoreVar();
        fileCtx->println( command_scoreboard_players_operation("=",tmp[0]+" "+tmp[1],lhs.value[0]+" "+lhs.value[1]));
        fileCtx->println( command_scoreboard_players_operation("-=",tmp[0]+" "+tmp[1],rhs.value[0]+" "+rhs.value[1]));
        return {tmp,Type::score};
    }},
    { {Type::score,Type::numberLiteral},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        auto tmp = fileCtx->getTempScoreVar();
        fileCtx->println( command_scoreboard_players_operation("=",tmp[0]+" "+tmp[1],lhs.value[0]+" "+lhs.value[1]));
        fileCtx->println( command_scoreboard_players("remove",tmp[0]+" "+tmp[1],rhs.value[0]));
        return {tmp,Type::score};
    }},
    { {Type::numberLiteral,Type::score},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        auto tmp = fileCtx->getTempScoreVar();
        fileCtx->println( command_scoreboard_players("set",tmp[0]+" "+tmp[1],lhs.value[0]));
        fileCtx->println( command_scoreboard_players_operation("-=",tmp[0]+" "+tmp[1],rhs.value[0]+" "+rhs.value[1]));
        return {tmp,Type::score};
    }},
    { {Type::numberLiteral,Type::numberLiteral},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        return { std::to_string(std::stod(lhs.value[0])-std::stod(rhs.value[0])) ,Type::numberLiteral };
    }},
};

GEN_OPERATOR(genSub)

static std::unordered_map<std::pair<Type,Type>,operator_form> genMul_map{
    { {Type::score,Type::score},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        auto tmp = fileCtx->getTempScoreVar();
        fileCtx->println( command_scoreboard_players_operation("=",tmp[0]+" "+tmp[1],lhs.value[0]+" "+lhs.value[1]));
        fileCtx->println( command_scoreboard_players_operation("*=",tmp[0]+" "+tmp[1],rhs.value[0]+" "+rhs.value[1]));
        return {tmp,Type::score};
    }},
    { {Type::score,Type::numberLiteral},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        auto tmp2 = fileCtx->getTempScoreVar();
        fileCtx->println( command_scoreboard_players("set",tmp2[0]+" "+tmp2[1],rhs.value[0]));
        fileCtx->println( command_scoreboard_players_operation("*=",tmp2[0]+" "+tmp2[1],lhs.value[0]+" "+lhs.value[1]));
        return {tmp2,Type::score};
    }},
    { {Type::numberLiteral,Type::score},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        return genMul_map.at({Type::score,Type::numberLiteral})(fileCtx,rhs,lhs);
    }},
    { {Type::numberLiteral,Type::numberLiteral},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        return { std::to_string(std::stod(lhs.value[0])*std::stod(rhs.value[0])) ,Type::numberLiteral };
    }},
};

GEN_OPERATOR(genMul)

static std::unordered_map<std::pair<Type,Type>,operator_form> genDiv_map{
    { {Type::score,Type::score},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        auto tmp = fileCtx->getTempScoreVar();
        fileCtx->println( command_scoreboard_players_operation("=",tmp[0]+" "+tmp[1],lhs.value[0]+" "+lhs.value[1]));
        fileCtx->println( command_scoreboard_players_operation("/=",tmp[0]+" "+tmp[1],rhs.value[0]+" "+rhs.value[1]));
        return {tmp,Type::score};
    }},
    { {Type::score,Type::numberLiteral},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        auto tmp = fileCtx->getTempScoreVar();
        auto tmp2 = fileCtx->getTempScoreVar();
        fileCtx->println( command_scoreboard_players_operation("=",tmp[0]+" "+tmp[1],lhs.value[0]+" "+lhs.value[1]));
        fileCtx->println( command_scoreboard_players("set",tmp2[0]+" "+tmp2[1],rhs.value[0]));
        fileCtx->println( command_scoreboard_players_operation("/=",tmp[0]+" "+tmp[1],tmp2[0]+" "+tmp2[1]));
        return {tmp,Type::score};
    }},
    { {Type::numberLiteral,Type::score},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        auto tmp = fileCtx->getTempScoreVar();
        fileCtx->println( command_scoreboard_players("set",tmp[0]+" "+tmp[1],lhs.value[0]));
        fileCtx->println( command_scoreboard_players_operation("/=",tmp[0]+" "+tmp[1],rhs.value[0]+" "+rhs.value[1]));
        return {tmp,Type::score};
    }},
    { {Type::numberLiteral,Type::numberLiteral},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        return { std::to_string(std::stod(lhs.value[0])/std::stod(rhs.value[0])) ,Type::numberLiteral };
    }},
};

GEN_OPERATOR(genDiv)

static std::unordered_map<std::pair<Type,Type>,operator_form> genMod_map{
    { {Type::score,Type::score},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        auto tmp = fileCtx->getTempScoreVar();
        fileCtx->println( command_scoreboard_players_operation("=",tmp[0]+" "+tmp[1],lhs.value[0]+" "+lhs.value[1]));
        fileCtx->println( command_scoreboard_players_operation("%=",tmp[0]+" "+tmp[1],rhs.value[0]+" "+rhs.value[1]));
        return {tmp,Type::score};
    }},
    { {Type::score,Type::numberLiteral},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        auto tmp = fileCtx->getTempScoreVar();
        auto tmp2 = fileCtx->getTempScoreVar();
        fileCtx->println( command_scoreboard_players_operation("=",tmp[0]+" "+tmp[1],lhs.value[0]+" "+lhs.value[1]));
        fileCtx->println( command_scoreboard_players("set",tmp2[0]+" "+tmp2[1],rhs.value[0]));
        fileCtx->println( command_scoreboard_players_operation("%=",tmp[0]+" "+tmp[1],tmp2[0]+" "+tmp2[1]));
        return {tmp,Type::score};
    }},
    { {Type::numberLiteral,Type::score},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        auto tmp = fileCtx->getTempScoreVar();
        fileCtx->println( command_scoreboard_players("set",tmp[0]+" "+tmp[1],lhs.value[0]));
        fileCtx->println( command_scoreboard_players_operation("%=",tmp[0]+" "+tmp[1],rhs.value[0]+" "+rhs.value[1]));
        return {tmp,Type::score};
    }},
    { {Type::numberLiteral,Type::numberLiteral},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        return { std::to_string(std::stoi(lhs.value[0])%std::stoi(rhs.value[0])) ,Type::numberLiteral };
    }},
};
GEN_OPERATOR(genMod)

static std::unordered_map<std::pair<Type,Type>,operator_form> genAddEq_map{
    { {Type::score,Type::score},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        fileCtx->println( command_scoreboard_players_operation("+=",lhs.value[0]+" "+lhs.value[1],rhs.value[0]+" "+rhs.value[1]));
        return lhs;
    }},
    { {Type::score,Type::numberLiteral},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        fileCtx->println( command_scoreboard_players("add",lhs.value[0]+" "+lhs.value[1],rhs.value[0]));
        return lhs;
    }}
};
GEN_OPERATOR(genAddEq)

static std::unordered_map<std::pair<Type,Type>,operator_form> genSubEq_map{
    { {Type::score,Type::score},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        fileCtx->println( command_scoreboard_players_operation("-=",lhs.value[0]+" "+lhs.value[1],rhs.value[0]+" "+rhs.value[1]));
        return lhs;
    }},
    { {Type::score,Type::numberLiteral},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        fileCtx->println( command_scoreboard_players("remove",lhs.value[0]+" "+lhs.value[1],rhs.value[0]));
        return lhs;
    }}
};
GEN_OPERATOR(genSubEq)

static std::unordered_map<std::pair<Type,Type>,operator_form> genMulEq_map{
    { {Type::score,Type::score},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        fileCtx->println( command_scoreboard_players_operation("*=",lhs.value[0]+" "+lhs.value[1],rhs.value[0]+" "+rhs.value[1]));
        return lhs;
    }},
    { {Type::score,Type::numberLiteral},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        auto tmp = fileCtx->getTempScoreVar();
        fileCtx->println( command_scoreboard_players("set",tmp[0]+" "+tmp[1],rhs.value[0]));
        fileCtx->println( command_scoreboard_players_operation("*=",lhs.value[0]+" "+lhs.value[1],tmp[0]+" "+tmp[1]));
        return lhs;
    }}
};
GEN_OPERATOR(genMulEq)

static std::unordered_map<std::pair<Type,Type>,operator_form> genDivEq_map{
    { {Type::score,Type::score},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        fileCtx->println( command_scoreboard_players_operation("/=",lhs.value[0]+" "+lhs.value[1],rhs.value[0]+" "+rhs.value[1]));
        return lhs;
    }},
    { {Type::score,Type::numberLiteral},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        auto tmp = fileCtx->getTempScoreVar();
        fileCtx->println( command_scoreboard_players("set",tmp[0]+" "+tmp[1],rhs.value[0]));
        fileCtx->println( command_scoreboard_players_operation("/=",lhs.value[0]+" "+lhs.value[1],tmp[0]+" "+tmp[1]));
        return lhs;
    }}
};
GEN_OPERATOR(genDivEq)

static std::unordered_map<std::pair<Type,Type>,operator_form> genModEq_map{
    { {Type::score,Type::score},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        fileCtx->println( command_scoreboard_players_operation("%=",lhs.value[0]+" "+lhs.value[1],rhs.value[0]+" "+rhs.value[1]));
        return lhs;
    }},
    { {Type::score,Type::numberLiteral},[](FileCompilationContext* fileCtx,const Value& lhs,const Value& rhs) -> Value
    {
        auto tmp = fileCtx->getTempScoreVar();
        fileCtx->println( command_scoreboard_players("set",tmp[0]+" "+tmp[1],rhs.value[0]));
        fileCtx->println( command_scoreboard_players_operation("%=",lhs.value[0]+" "+lhs.value[1],tmp[0]+" "+tmp[1]));
        return lhs;
    }}
};
GEN_OPERATOR(genModEq)