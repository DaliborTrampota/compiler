#pragma once

#include "declaration/DeclarationNode.h"
#include <vector>

class CodeGen;

struct ProgramNode {
  std::vector<DeclarationNode *> declarations;

  ProgramNode(std::vector<DeclarationNode *> declarations)
      : declarations(declarations) {}

  void accept(CodeGen &codeGen) {}
};
