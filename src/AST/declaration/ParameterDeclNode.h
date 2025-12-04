#pragma once

#include <string>
#include "../type/TypeNode.h"
#include "DeclarationNode.h"


struct ParameterDeclNode : public DeclarationNode {
    std::string identifier;
    TypeNode* type;

    ParameterDeclNode(std::string identifier, TypeNode* type)
        : identifier(identifier),
          type(type) {}

    // Not needed
    void accept(CodeGen& codeGen) override {};
};