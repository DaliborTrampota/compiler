#pragma once

#include "TypeNode.h"
#include <string>

struct PointerTypeNode : public TypeNode {
  TypeNode *baseType;

  PointerTypeNode(TypeNode *baseType) : baseType(baseType) {}
};