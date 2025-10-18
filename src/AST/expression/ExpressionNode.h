#pragma once

#include "CodeGen.h"

struct ExpressionNode {
    virtual ~ExpressionNode() = default;
    virtual void accept(CodeGen& codeGen) = 0;
};