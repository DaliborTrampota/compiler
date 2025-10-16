#pragma once

#include "ExpressionNode.h"
#include <string>

struct UnaryExprNode : public ExpressionNode {
  ExpressionNode *operand;
  std::string op;
  bool prefix;

  UnaryExprNode(ExpressionNode *operand, std::string op, bool prefix)
      : operand(operand), op(op), prefix(prefix) {}
};