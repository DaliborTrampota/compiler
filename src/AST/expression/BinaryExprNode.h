#pragma once

#include <string>
#include "ExpressionNode.h"


struct BinaryExprNode : public ExpressionNode {
    ExpressionNode* left;
    ExpressionNode* right;
    std::string op;

    BinaryExprNode(ExpressionNode* left, ExpressionNode* right, std::string op)
        : left(left),
          right(right),
          op(op) {}

    llvm::Value* accept(CodeGen& codeGen) override { return codeGen.visitBinaryExpr(this); }
};
