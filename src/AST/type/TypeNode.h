#pragma once

#include <llvm/IR/Type.h>
#include "../../CodeGen.h"

struct TypeNode {
    virtual ~TypeNode() = default;
    virtual llvm::Type* accept(CodeGen& codeGen) = 0;
};