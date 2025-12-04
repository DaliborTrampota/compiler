#include <iostream>
#include <memory>

#include "AST/ProgramNode.h"
#include "AST/fromJSON.h"


// using json = nlohmann::json;

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "1# json ast path #2 output path [#3] print AST\n";
        exit(1);
    }
    std::string filePath = argv[1];
    nlohmann::json ast = loadASTFromFile(filePath);
    ProgramNode* program = fromJSON(ast);
    if (argc == 3)
        printAST(program);

    CodeGen codeGen("MyProgram");
    codeGen.generate(program);
    codeGen.printToFile(argv[2]);

    //   InitializeAllTargets();
    //   InitializeAllTargetMCs();
    //   InitializeAllAsmPrinters();
    //   InitializeAllAsmParsers();

    std::cout << "Compiler initialized!" << std::endl;
    return 0;
}