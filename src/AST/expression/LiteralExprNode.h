#pragma once

#include <string>
#include "ExpressionNode.h"


struct IntegerLiteralNode : public ExpressionNode {
    int value;

    IntegerLiteralNode(int value) : value(value) {}

    void accept(CodeGen& codeGen) override { codeGen.visitIntegerLiteral(this); }
};

struct DoubleLiteralNode : public ExpressionNode {
    double value;

    DoubleLiteralNode(double value) : value(value) {}

    void accept(CodeGen& codeGen) override { codeGen.visitDoubleLiteral(this); }
};

struct CharacterLiteralNode : public ExpressionNode {
    char value;

    CharacterLiteralNode(char value) : value(value) {}

    void accept(CodeGen& codeGen) override { codeGen.visitCharacterLiteral(this); }
};

struct StringLiteralNode : public ExpressionNode {
    std::string value;

    StringLiteralNode(std::string value) : value(value) {}

    void accept(CodeGen& codeGen) override { codeGen.visitStringLiteral(this); }
};