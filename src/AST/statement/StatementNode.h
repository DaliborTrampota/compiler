#pragma once

#include "CodeGen.h"

struct StatementNode {
    virtual ~StatementNode() = default;
    virtual void accept(CodeGen& codeGen) = 0;
};