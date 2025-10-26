#pragma once

#include <vector>
#include "StatementNode.h"


struct BlockStatementNode : public StatementNode {
    std::vector<StatementNode*> body;

    BlockStatementNode(std::vector<StatementNode*> body) : body(body) {}

    static BlockStatementNode* Empty() { return new BlockStatementNode({}); }

    void accept(CodeGen& codeGen) override { codeGen.visitBlockStatement(this); }
};