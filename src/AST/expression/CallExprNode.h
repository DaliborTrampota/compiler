#pragma once

#include <vector>
#include "ExpressionNode.h"


struct CallExprNode : public ExpressionNode {
    IdentifierExprNode* callee;
    std::vector<ExpressionNode*> arguments;

    CallExprNode(IdentifierExprNode* callee, std::vector<ExpressionNode*> arguments)
        : callee(callee),
          arguments(arguments) {}

    llvm::Value* accept(CodeGen& codeGen) override { return codeGen.visitCallExpr(this); }
};