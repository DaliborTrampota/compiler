#pragma once

#include <vector>
#include "ExpressionNode.h"


struct CallExprNode : public ExpressionNode {
    ExpressionNode* callee;
    std::vector<ExpressionNode*> arguments;

    CallExprNode(ExpressionNode* callee, std::vector<ExpressionNode*> arguments)
        : callee(callee),
          arguments(arguments) {}

    llvm::Value* accept(CodeGen& codeGen) override { return codeGen.visitCallExpr(this); }
};