#pragma once

#include <string>
#include "TypeNode.h"

struct NamedTypeNode : public TypeNode {
    std::string identifier;

    NamedTypeNode(std::string identifier) : identifier(identifier) {}

    void accept(CodeGen& codeGen) override { codeGen.visitNamedType(this); }
};