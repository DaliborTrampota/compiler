#pragma once

#include "TypeNode.h"

struct PointerTypeNode : public TypeNode {
    TypeNode* baseType;

    PointerTypeNode(TypeNode* baseType) : baseType(baseType) {}

    llvm::Type* accept(CodeGen& codeGen) override { return codeGen.visitPointerType(this); }
};