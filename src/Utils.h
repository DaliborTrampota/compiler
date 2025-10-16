#pragma once

#include <nlohmann/json.hpp>
#include <string>


nlohmann::json loadASTFromJSON(const std::string &jsonFile);