#pragma once
#include"compilation.h"

template<typename T>
inline void resolveName(FileCompilationContext* fileCtx,const Namespace& base,std::string& var_type, T predicate)
{
    if ( !var_type.contains(":") && !(fileCtx->*predicate)(var_type) )
    {
        auto nmsp = base;
        std::cout<<"Got unresolved Name: "<<var_type<<"\n";
        while ( true )
        {
            std::cout<<"Resolving Name: "<<nmsp.toMinecraftPath()<<var_type<<"\n";
            if ( nmsp.path.empty() || (fileCtx->*predicate)( nmsp.toMinecraftPath() + var_type ) ) break;
            nmsp.pop();
        }
        var_type = nmsp.toMinecraftPath() + var_type;
    }
}

template<typename T>
inline void resolveName(FileCompilationContext* fileCtx,std::string& var_type,T predicate)
{
    resolveName(fileCtx,fileCtx->currentNamespace,var_type,predicate);
}