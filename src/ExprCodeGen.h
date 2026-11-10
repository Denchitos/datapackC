#pragma once
#include<memory>
#include<string>
#include<vector>
class FileCompilationContext;
class Value;

class ExprCodeGen
{
    std::shared_ptr<FileCompilationContext> fileCtx;
public:
    ExprCodeGen(std::shared_ptr<FileCompilationContext>);
    
    Value genAssign(const Value& lhs,const Value& rhs);

    Value genAnd(const Value& lhs,const Value& rhs);
    Value genOr(const Value& lhs,const Value& rhs);

    Value genMore(const Value& lhs,const Value& rhs);
    Value genLess(const Value& lhs,const Value& rhs);
    Value genEquals(const Value& lhs,const Value& rhs);
    Value genNotEquals(const Value& lhs,const Value& rhs);
    Value genMoreEq(const Value& lhs,const Value& rhs);
    Value genLessEq(const Value& lhs,const Value& rhs);

    Value genAdd(const Value& lhs,const Value& rhs); // +
    Value genSub(const Value& lhs,const Value& rhs); // -
    Value genDiv(const Value&  lhs,const Value& rhs); // /
    Value genMul(const Value&  lhs,const Value& rhs); // *
    Value genMod(const Value&  lhs,const Value& rhs); // %

    Value genAddEq(const Value& lhs,const Value& rhs); // +=
    Value genSubEq(const Value& lhs,const Value& rhs); // -=
    Value genDivEq(const Value&  lhs,const Value& rhs); // /=
    Value genMulEq(const Value&  lhs,const Value& rhs); // *=
    Value genModEq(const Value&  lhs,const Value& rhs); // %=
    
    Value genBinaryOperator(const std::string& op, const Value& lhs,const Value& rhs);
};