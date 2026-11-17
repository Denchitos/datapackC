#pragma once
#include<unordered_map>
#include<filesystem>
#include<vector>

#include "tree/node.h"

namespace fs = std::filesystem;

struct ProcessedFile
{
    fs::path file_name;
    std::vector<tree::NodePtr> ast;
};

using Modules = std::unordered_map<fs::path,ProcessedFile>;