#pragma once

#include <llvm/IR/Value.h>

// Forward declaration
class CodeGen;

struct ExpressionNode {
    virtual ~ExpressionNode() = default;
    virtual llvm::Value* accept(CodeGen& codeGen) = 0;
};