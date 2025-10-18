#pragma once

#include <vector>
#include "ExpressionNode.h"


struct CommaExprNode : public ExpressionNode {
    std::vector<ExpressionNode*> expressions;

    CommaExprNode(std::vector<ExpressionNode*> expressions) : expressions(expressions) {}

    void accept(CodeGen& codeGen) override { codeGen.visitCommaExpr(this); }
};