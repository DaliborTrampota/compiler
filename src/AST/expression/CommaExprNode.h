#pragma once

#include "ExpressionNode.h"
#include <vector>

struct CommaExprNode : public ExpressionNode {
  std::vector<ExpressionNode *> expressions;

  CommaExprNode(std::vector<ExpressionNode *> expressions)
      : expressions(expressions) {}
};