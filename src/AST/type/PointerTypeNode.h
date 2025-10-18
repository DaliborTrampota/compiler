#pragma once

#include <string>
#include "TypeNode.h"

struct PointerTypeNode : public TypeNode {
    TypeNode* baseType;

    PointerTypeNode(TypeNode* baseType) : baseType(baseType) {}
};