#pragma once

#include <vector>
#include "../expression/ExpressionNode.h"
#include "StatementNode.h"


struct DoWhileStatementNode : public StatementNode {
    ExpressionNode* condition;
    StatementNode* loopBody;

    DoWhileStatementNode(ExpressionNode* condition, StatementNode* body)
        : condition(condition),
          loopBody(loopBody) {}
};