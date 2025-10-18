#pragma once

#include "StatementNode.h"


struct ContinueStatementNode : public StatementNode {
    ContinueStatementNode() {}

    void accept(CodeGen& codeGen) override { codeGen.visitContinueStatement(this); }
};