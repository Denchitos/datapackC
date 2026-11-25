#pragma once
#include<memory>
#include<unordered_map>
#include<filesystem>

#include"compilation.h"

#include "compilation.h"

struct AST;

namespace fs = std::filesystem;

struct ProcessedFile
{
    fs::path file_name;
    std::unique_ptr<AST> ast;
    std::shared_ptr<FileCompilationContext> fileCtx = nullptr;

    Namespace nmsp;
};

using Modules = std::unordered_map<fs::path,ProcessedFile>;