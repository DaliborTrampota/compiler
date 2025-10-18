#pragma once

#include <string>
#include "ExpressionNode.h"


struct UnaryExprNode : public ExpressionNode {
    ExpressionNode* operand;
    std::string op;
    bool prefix;

    UnaryExprNode(ExpressionNode* operand, std::string op, bool prefix)
        : operand(operand),
          op(op),
          prefix(prefix) {}
};