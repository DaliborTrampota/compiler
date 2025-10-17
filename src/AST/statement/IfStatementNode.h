#pragma once

#include "../expression/ExpressionNode.h"
#include "StatementNode.h"


struct IfStatementNode : public StatementNode {
    ExpressionNode* condition;
    StatementNode* thenBody;
    StatementNode* elseBody;

    IfStatementNode(ExpressionNode* condition, StatementNode* thenBody, StatementNode* elseBody)
        : condition(condition),
          thenBody(thenBody),
          elseBody(elseBody) {}
};