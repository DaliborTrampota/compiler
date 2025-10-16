#pragma once

#include "../type/TypeNode.h"
#include "DeclarationNode.h"
#include <string>

struct ParameterDeclNode : public DeclarationNode {
  std::string identifier;
  TypeNode *type;

  ParameterDeclNode(std::string identifier, TypeNode *type)
      : identifier(identifier), type(type) {}
};