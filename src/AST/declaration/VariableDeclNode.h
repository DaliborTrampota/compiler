#pragma once

#include "../expression/ExpressionNode.h"
#include "../type/TypeNode.h"
#include "DeclarationNode.h"
#include <string>


struct VariableDeclNode : public DeclarationNode {
  std::string identifier;
  TypeNode *type;
  ExpressionNode *arraySize;
  ExpressionNode *initializer;

  VariableDeclNode(std::string identifier, TypeNode *type,
                   ExpressionNode *arraySize, ExpressionNode *initializer)
      : identifier(identifier), type(type), arraySize(arraySize),
        initializer(initializer) {}
};