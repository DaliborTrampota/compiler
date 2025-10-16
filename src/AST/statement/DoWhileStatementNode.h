#pragma once

#include "../expression/ExpressionNode.h"
#include "StatementNode.h"
#include <vector>

struct DoWhileStatementNode : public StatementNode {
  ExpressionNode *condition;
  StatementNode *loopBody;

  DoWhileStatementNode(ExpressionNode *condition, StatementNode *body)
      : condition(condition), loopBody(loopBody) {}
};