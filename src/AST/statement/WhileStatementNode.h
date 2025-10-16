#pragma once

#include "../expression/ExpressionNode.h"
#include "StatementNode.h"
#include <vector>


struct WhileStatementNode : public StatementNode {
  ExpressionNode *condition;
  StatementNode *loopBody;

  WhileStatementNode(ExpressionNode *condition, StatementNode *body)
      : condition(condition), loopBody(loopBody) {}
};