#pragma once

#include <string>
#include "TypeNode.h"

struct NamedTypeNode : public TypeNode {
    std::string identifier;

    NamedTypeNode(std::string identifier) : identifier(identifier) {}

    llvm::Type* accept(CodeGen& codeGen) override { return codeGen.visitNamedType(this); }
};