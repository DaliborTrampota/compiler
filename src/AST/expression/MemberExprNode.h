#pragma once

#include "ExpressionNode.h"
#include <string>

struct MemberExprNode : public ExpressionNode {
  ExpressionNode *object;
  std::string member;
  bool arrowAccess;

  MemberExprNode(ExpressionNode *object, std::string member, bool arrowAccess)
      : object(object), member(member), arrowAccess(arrowAccess) {}
};