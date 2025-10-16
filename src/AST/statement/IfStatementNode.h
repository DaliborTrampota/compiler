#pragma once

#include "StatementNode.h"
#include <vector>

struct BlockStatementNode : public StatementNode {
  std::vector<StatementNode *> body;

  BlockStatementNode(std::vector<StatementNode *> body) : body(body) {}
};