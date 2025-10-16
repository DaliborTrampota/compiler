#pragma once

#include "../expression/ExpressionNode.h"
#include "StatementNode.h"


struct ExpressionStatementNode : public StatementNode {
  ExpressionNode *expression;

  ExpressionStatementNode(ExpressionNode *expression)
      : expression(expression) {}
};