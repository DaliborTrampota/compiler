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

    void accept(CodeGen& codeGen) override { codeGen.visitMemberExpr(this); }
};