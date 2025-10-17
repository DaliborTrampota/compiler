#pragma once

#include "ExpressionNode.h"
#include <vector>

struct CallExprNode : public ExpressionNode {
  ExpressionNode *callee;
  std::vector<ExpressionNode *> arguments;

  CallExprNode(ExpressionNode *callee, std::vector<ExpressionNode *> arguments)
      : callee(callee), arguments(arguments) {}
};