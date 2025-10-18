#pragma once

#include "CodeGen.h"

struct TypeNode {
    virtual ~TypeNode() = default;
    virtual void accept(CodeGen& codeGen) = 0;
};