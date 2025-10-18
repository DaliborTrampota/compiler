#pragma once

#include <string>
#include <vector>
#include "../type/TypeNode.h"
#include "DeclarationNode.h"
#include "ParameterDeclNode.h"


struct FunctionPtrDeclNode : public DeclarationNode {
    std::string identifier;
    TypeNode* returnType;
    std::vector<ParameterDeclNode*> parameters;

    FunctionPtrDeclNode(
        std::string identifier, TypeNode* returnType, std::vector<ParameterDeclNode*> parameters
    )
        : identifier(identifier),
          returnType(returnType),
          parameters(parameters) {}

};