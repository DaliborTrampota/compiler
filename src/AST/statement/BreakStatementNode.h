#pragma once

#include "StatementNode.h"


struct BreakStatementNode : public StatementNode {
    BreakStatementNode() {}

    void accept(CodeGen& codeGen) override { codeGen.visitBreakStatement(this); }
};