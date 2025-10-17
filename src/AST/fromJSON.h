#pragma once

#include <nlohmann/json.hpp>

struct ProgramNode;

ProgramNode* fromJSON(const nlohmann::json& json);

nlohmann::json loadASTFromFile(const std::string& jsonFile);

void printAST(const ProgramNode* ast);