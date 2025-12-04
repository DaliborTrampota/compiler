#pragma once

#include <llvm/IR/Value.h>
#include "../../CodeGen.h"

struct ExpressionNode {
    virtual ~ExpressionNode() = default;
    virtual llvm::Value* accept(CodeGen& codeGen) = 0;
};