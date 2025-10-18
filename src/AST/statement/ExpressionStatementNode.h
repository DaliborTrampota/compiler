#pragma once

#include <variant>
#include "../declaration/DeclarationNode.h"
#include "../expression/ExpressionNode.h"
#include "StatementNode.h"


struct ExpressionStatementNode : public StatementNode {
    using ExprT = std::variant<ExpressionNode*, DeclarationNode*>;
    ExprT expression;

    ExpressionStatementNode(ExprT expression) : expression(expression) {}

    void accept(CodeGen& codeGen) override {
        if (std::holds_alternative<ExpressionNode*>(expression)) {
            std::get<ExpressionNode*>(expression)->accept(codeGen);
        } else {
            std::get<DeclarationNode*>(expression)->accept(codeGen);
        }
    }
};