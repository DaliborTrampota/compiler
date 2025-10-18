#pragma once

#include <vector>
#include "declaration/DeclarationNode.h"


class CodeGen;

struct ProgramNode {
    std::vector<DeclarationNode*> declarations;

    ProgramNode(std::vector<DeclarationNode*> declarations) : declarations(declarations) {}
};
