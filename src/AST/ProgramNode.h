#pragma once

#include "declaration/DeclarationNode.h"
#include <vector>

struct ProgramNode {
  std::vector<DeclarationNode *> declarations;
};
