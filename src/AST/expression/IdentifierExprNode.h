#pragma once

#include <string>
#include "ExpressionNode.h"


struct IdentifierExprNode : public ExpressionNode {
    std::string name;

    IdentifierExprNode(std::string name) : name(name) {}

    void accept(CodeGen& codeGen) override { codeGen.visitIdentifierExpr(this); }
};