#pragma once

#include "TypeNode.h"
#include <string>

struct PrimitiveTypeNode : public TypeNode {
  enum Kind { Void, Char, Int, Double };

  Kind kind;

  PrimitiveTypeNode(Kind kind) : kind(kind) {}
};