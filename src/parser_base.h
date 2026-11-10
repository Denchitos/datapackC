#pragma once
#include<string>
#include<vector>
#include<concepts>
#include<list>
#include<algorithm>

template<typename T>
concept TokenType = requires ( T tok )
{
    requires std::default_initializable<T>;
    { tok.toString() } -> std::convertible_to<std::string>;
};

template<typename Left , typename Right >
concept Comparable = requires( Left lhs, Right rhs )
{
    { lhs==rhs } -> std::convertible_to<bool>;
};

template<TokenType T>
class ParserBase
{
public:
    struct ParserMessage
    {
        T token;
        std::string msg;
    };
private:

    std::vector<T> stream;
    std::list<ParserMessage> messages;
    T beforeStartToken;
    T afterEndToken;

    using id = long long;
    id i = 0;
public:
    id size() const
    {
        return static_cast<id>(stream.size());
    }

    std::list<ParserMessage>& getMessages()
    {
        return messages;
    }
protected:

    id current() const 
    {
        return i;
    }

    void setTokenStream( const std::vector<T>& stream, const T& beforeStartToken , const T& afterEndToken  )
    {
        i = 0;
        this->stream = stream;
        this->beforeStartToken = beforeStartToken;
        this->afterEndToken = afterEndToken;
    }

    void next(id j = 1)
    {
        i = std::clamp(i+j,0LL,size());
    }

    const T& look(id j = 0) const
    {
        if ( i+j >= size() ) return afterEndToken;
        else if ( i+j < 0  ) return beforeStartToken;
        return stream[i+j];
    }
    void test(){
        

    }
   bool atEnd() const
    {
        return i>=size();
    }

    template<class Cmp> requires Comparable<T,Cmp>
    bool compare(const Cmp& cmpVal,id i = 0) const
    {
        return look(i)==cmpVal;
    }

    template<class Cmp> requires Comparable<T,Cmp>
    bool match(const Cmp& cmpVal,id i = 0)
    {
        bool cmp = compare(cmpVal,i);
        if ( cmp ) next();
        return cmp;
    }
    
    template<class Cmp> requires Comparable<T,Cmp>
    bool expect(const Cmp& cmpVal,const std::string& msg,id i = 0)
    {
        if (match(cmpVal,i)) return true;
        error(msg);
        return false;
    }

    virtual void error(const std::string& str,id i = 0)
    {
        messages.emplace_back(look(i),"[ERROR] : "+ str);
    }

    virtual void warn(const std::string& str,id i = 0)
    {
        messages.emplace_back(look(i),"[WARNING] : " + str);
    }
};