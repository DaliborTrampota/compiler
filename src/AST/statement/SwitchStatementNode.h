#pragma once

#include "../expression/LiteralExprNode.h"
#include "StatementNode.h"

#include <vector>

struct CaseStatementNode : public StatementNode {
    IntegerLiteralNode* value;
    StatementNode* body;

    CaseStatementNode(IntegerLiteralNode* value, StatementNode* body) : value(value), body(body) {}

    void accept(CodeGen& codeGen) override { codeGen.visitCaseStatement(this); }
};

struct SwitchStatementNode : public StatementNode {
    ExpressionNode* condition;
    std::vector<CaseStatementNode*> cases;
    StatementNode* defaultBody;

    SwitchStatementNode(
        ExpressionNode* condition, std::vector<CaseStatementNode*> cases, StatementNode* defaultBody
    )
        : condition(condition),
          cases(cases),
          defaultBody(defaultBody) {}

    void accept(CodeGen& codeGen) override { codeGen.visitSwitchStatement(this); }
};
