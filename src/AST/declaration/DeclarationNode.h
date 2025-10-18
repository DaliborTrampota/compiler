#pragma once

#include "CodeGen.h"

struct DeclarationNode {
    virtual ~DeclarationNode() = default;

    virtual void accept(CodeGen& codeGen) = 0;
};