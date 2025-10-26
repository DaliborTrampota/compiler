#pragma once

#include <string>
#include "ExpressionNode.h"


struct MemberExprNode : public ExpressionNode {
    ExpressionNode* object;
    std::string member;
    bool arrowAccess;

    MemberExprNode(ExpressionNode* object, std::string member, bool arrowAccess)
        : object(object),
          member(member),
          arrowAccess(arrowAccess) {}

    llvm::Value* accept(CodeGen& codeGen) override { return codeGen.visitMemberExpr(this); }
};