#pragma once
#include<memory>
#include<unordered_map>
#include<filesystem>
#include<vector>

#include"compilation.h"

#include "tree/node.h"
#include "compilation.h"

namespace fs = std::filesystem;

struct ProcessedFile
{
    fs::path file_name;
    std::vector<tree::NodePtr> ast;
    std::shared_ptr<FileCompilationContext> fileCtx = nullptr;

    Namespace nmsp;
};

using Modules = std::unordered_map<fs::path,ProcessedFile>;