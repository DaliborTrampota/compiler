#pragma once

#include <string>
#include <vector>
#include "../statement/BlockStatementNode.h"
#include "DeclarationNode.h"
#include "ParameterDeclNode.h"
#include "VariableDeclNode.h"


// StructDeclaration for forward declaration
// StructDefinition for full definition
struct StructDeclNode : public DeclarationNode {
    std::string identifier;
    std::vector<VariableDeclNode*> fields;  // empty for forward declaration

    StructDeclNode(std::string identifier, std::vector<VariableDeclNode*> fields)
        : identifier(identifier),
          fields(fields) {}

    void accept(CodeGen& codeGen) override {
        if (fields.empty())
            codeGen.visitStructDeclaration(this);
        else
            codeGen.visitStructDefinition(this);
    }
};