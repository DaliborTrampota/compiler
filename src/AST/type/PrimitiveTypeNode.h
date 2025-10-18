#pragma once

#include <string>
#include "TypeNode.h"

struct PrimitiveTypeNode : public TypeNode {
    enum Kind {
        Void,
        Char,
        Int,
        Double
    };

    Kind kind;

    PrimitiveTypeNode(Kind kind) : kind(kind) {}

    static Kind kindFromString(const std::string& kindStr) {
        if (kindStr == "void")
            return Void;
        if (kindStr == "int")
            return Int;
        if (kindStr == "double")
            return Double;
        if (kindStr == "Char")
            return Char;
    }
};