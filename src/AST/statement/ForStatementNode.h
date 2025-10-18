#pragma once

#include <variant>
#include <vector>
#include "../declaration/DeclarationNode.h"
#include "../expression/ExpressionNode.h"
#include "StatementNode.h"

struct ForStatementNode : public StatementNode {
    using InitT = std::variant<ExpressionNode*, DeclarationNode*>;
    InitT init;
    ExpressionNode* update;
    ExpressionNode* condition;
    StatementNode* body;

    ForStatementNode(
        InitT init, ExpressionNode* condition, ExpressionNode* update, StatementNode* body
    )
        : init(init),
          condition(condition),
          update(update),
          body(body) {}

    void accept(CodeGen& codeGen) override { codeGen.visitForStatement(this); }
};