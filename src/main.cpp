#include <iostream>
#include <memory>

#include "Utils.h"
#include "llvmIRBuilder.h"

using namespace llvm;
// using json = nlohmann::json;

int main(int argc, char *argv[]) {

  if (argc < 2) {
    std::cout << "File argument needed\n";
    exit(1);
  }

  nlohmann::json ast = loadASTFromJSON(argv[1]);

  //   InitializeAllTargets();
  //   InitializeAllTargetMCs();
  //   InitializeAllAsmPrinters();
  //   InitializeAllAsmParsers();

  std::cout << "Compiler initialized!" << std::endl;
  return 0;
}