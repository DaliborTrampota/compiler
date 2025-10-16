#pragma once

#include "ExpressionNode.h"
#include <string>

struct IntegerLiteralNode : public ExpressionNode {
  int value;

  IntegerLiteralNode(int value) : value(value) {}
};

struct DoubleLiteralNode : public ExpressionNode {
  double value;

  DoubleLiteralNode(double value) : value(value) {}
};

struct CharacterLiteralNode : public ExpressionNode {
  char value;

  CharacterLiteralNode(char value) : value(value) {}
};

struct StringLiteralNode : public ExpressionNode {
  std::string value;

  StringLiteralNode(std::string value) : value(value) {}
};