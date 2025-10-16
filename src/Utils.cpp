#include "Utils.h"

#include <fstream>
#include <iostream>

nlohmann::json loadASTFromJSON(const std::string &jsonFile) {

  std::string srcFile = argv[1];
  std::ifstream R(srcFile);

  // Example: Parse the input file as JSON
  try {
    nlohmann::json data = nlohmann::json::parse(R);
    std::cout << "Parsed JSON successfully!\n";
    std::cout << "Pretty print:\n" << data.dump(2) << std::endl;

    return data;
  } catch (nlohmann::json::parse_error &e) {
    std::cout << "Parse error: Not a JSON file\n" << e.what() << std::endl;
  }
}