#pragma once

#include <vector>
#include "../expression/ExpressionNode.h"
#include "StatementNode.h"


struct WhileStatementNode : public StatementNode {
    ExpressionNode* condition;
    StatementNode* loopBody;

    WhileStatementNode(ExpressionNode* condition, StatementNode* body)
        : condition(condition),
          loopBody(loopBody) {}

    void accept(CodeGen& codeGen) override { codeGen.visitWhileStatement(this); }
};