#pragma once

#include "ExpressionNode.h"
#include <vector>

struct CallExprNode : public ExpressionNode {
  std::vector<ExpressionNode *> arguments;
  ExpressionNode *callee;

  CallExprNode(ExpressionNode *callee, std::vector<ExpressionNode *> arguments)
      : callee(callee), arguments(arguments) {}
};