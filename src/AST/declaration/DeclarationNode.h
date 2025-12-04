#pragma once

#include "../NodeVisitor.h"
#include "../CodeGen.h"


struct DeclarationNode {
    virtual ~DeclarationNode() = default;

    virtual void accept(CodeGen& codeGen) = 0;
    //virtual void visit(NodeVisitor& visitor) = 0;
};