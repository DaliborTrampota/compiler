#pragma once

#include <variant>
#include "../declaration/DeclarationNode.h"
#include "../expression/ExpressionNode.h"
#include "StatementNode.h"



struct ExpressionStatementNode : public StatementNode {
    using ExprT = std::variant<ExpressionNode*, DeclarationNode*>;
    ExprT expression;

    ExpressionStatementNode(ExprT expression) : expression(expression) {}
};