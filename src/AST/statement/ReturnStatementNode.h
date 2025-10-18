#pragma once

#include "../expression/ExpressionNode.h"
#include "StatementNode.h"

;


struct ReturnStatementNode : public StatementNode {
    ExpressionNode* returnValue;

    ReturnStatementNode(ExpressionNode* returnValue) : returnValue(returnValue) {}

    void accept(CodeGen& codeGen) override { codeGen.visitReturnStatement(this); }
};