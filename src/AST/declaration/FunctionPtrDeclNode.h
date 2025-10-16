#pragma once

#include "../type/TypeNode.h"
#include "DeclarationNode.h"
#include "ParameterDeclNode.h"
#include <string>
#include <vector>

struct FunctionPtrDeclNode : public DeclarationNode {
  std::string identifier;
  TypeNode *returnType;
  std::vector<ParameterDeclNode *> parameters;

  FunctionPtrDeclNode(std::string identifier, TypeNode *returnType,
                      std::vector<ParameterDeclNode *> parameters)
      : identifier(identifier), returnType(returnType), parameters(parameters) {
  }
};