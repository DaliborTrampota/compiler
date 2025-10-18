#pragma once

#include <string>
#include "../type/TypeNode.h"
#include "ExpressionNode.h"


struct CastExprNode : public ExpressionNode {
    TypeNode* type;
    ExpressionNode* operand;

    CastExprNode(TypeNode* type, ExpressionNode* operand) : type(type), operand(operand) {}
};