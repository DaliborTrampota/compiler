#pragma once

#include "ExpressionNode.h"

struct IndexExprNode : public ExpressionNode {
    ExpressionNode* array;
    ExpressionNode* index;

    IndexExprNode(ExpressionNode* array, ExpressionNode* index) : array(array), index(index) {}

    void accept(CodeGen& codeGen) override { codeGen.visitIndexExpr(this); }
};