#pragma once

#include "ExpressionNode.h"
#include <string>

struct IdentifierExprNode : public ExpressionNode {
  std::string name;

  IdentifierExprNode(std::string name) : name(name) {}
};