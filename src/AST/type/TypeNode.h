#pragma once

#include <llvm/IR/Type.h>

// Forward declaration
class CodeGen;

struct TypeNode {
    virtual ~TypeNode() = default;
    virtual llvm::Type* accept(CodeGen& codeGen) = 0;
};