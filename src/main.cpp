#include <iostream>
#include <memory>

#include "AST/ProgramNode.h"
#include "AST/fromJSON.h"


// using json = nlohmann::json;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "File argument needed\n";
        exit(1);
    }
    std::string filePath = argv[1];
    nlohmann::json ast = loadASTFromFile(filePath);
    ProgramNode* program = fromJSON(ast);
    printAST(program);

    CodeGen codeGen("MyProgram");
    codeGen.generate(program);

    //   InitializeAllTargets();
    //   InitializeAllTargetMCs();
    //   InitializeAllAsmPrinters();
    //   InitializeAllAsmParsers();

    std::cout << "Compiler initialized!" << std::endl;
    return 0;
}