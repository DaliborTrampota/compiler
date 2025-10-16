#pragma once

#include "TypeNode.h"
#include <string>

struct NamedTypeNode : public TypeNode {
  std::string identifier;

  NamedTypeNode(std::string identifier) : identifier(identifier) {}
};