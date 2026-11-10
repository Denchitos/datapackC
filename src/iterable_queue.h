#include<queue>

template<typename T>
class IterableQueue : public std::queue<T>
{
    using std::queue<T>::c;
public:
    auto size()
    {
        return c.size();
    }

    auto begin()
    {
        return c.begin();
    }
    auto end() 
    {
        return c.end();
    }

    auto cbegin() const
    {
        return c.cbegin();
    }

    auto cend() const 
    {
        return c.cend();
    }

    auto rbegin() 
    {
        return c.rbegin();
    }

    auto rend() 
    {
        return c.rend();
    }

    auto crbegin() const
    {
        return c.crbegin();
    }

    auto crend() const
    {
        return c.crend();
    }

    const auto& operator[](long long i) const
    {
        if ( i>=0 ) return c[i];
        else return c[size()+i];
    };

    auto& operator[](long long i)
    {
        if ( i>=0 ) return c[i];
        else return c[size()+i];
    };

};