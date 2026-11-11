@echo off

chcp 65001 > nul

set "RUN=1"

set "src_code=lexer.cpp main.cpp token.cpp compilation.cpp types.cpp parser.cpp print_visitor.cpp declaration_visitor.cpp compiler_visitor.cpp ExprCodeGen.cpp tree/call.cpp tree/block.cpp tree/function.cpp tree/node.cpp tree/if.cpp tree/while.cpp tree/raw.cpp tree/expr.cpp tree/vardecl.cpp tree/raw_selector.cpp tree/namespace.cpp tree/member_access.cpp tree/objective.cpp tree/return.cpp"

set "output=program.exe"

del "%output%"

echo Сборка...

cd src
g++ %src_code% -o "../%output%" -std=c++23 -Wall -Wextra -Werror -Wuseless-cast -Wno-unused-parameter -static-libgcc -static-libstdc++
cd ..

echo Сборка завершена

if "%RUN%"== "1" (
    @echo on
    %output% %1
)