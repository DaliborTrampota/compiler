#pragma once

#include "../type/TypeNode.h"
#include "ExpressionNode.h"
#include <string>

struct CastExprNode : public ExpressionNode {
  TypeNode *type;
  ExpressionNode *operand;

  CastExprNode(TypeNode *type, ExpressionNode *operand)
      : type(type), operand(operand) {}
};