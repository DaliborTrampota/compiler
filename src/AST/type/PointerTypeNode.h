#pragma once

#include "TypeNode.h"

struct PointerTypeNode : public TypeNode {
    TypeNode* baseType;

    PointerTypeNode(TypeNode* baseType) : baseType(baseType) {}

    void accept(CodeGen& codeGen) override { codeGen.visitPointerType(this); }
};