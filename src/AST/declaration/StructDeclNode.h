#pragma once

#include "../statement/BlockStatementNode.h"
#include "DeclarationNode.h"
#include "ParameterDeclNode.h"
#include "VariableDeclNode.h"
#include <string>
#include <vector>


// StructDeclaration for forward declaration
// StructDefinition for full definition
struct StructDeclNode : public DeclarationNode {
  std::string identifier;
  std::vector<VariableDeclNode *> fields; // empty for forward declaration

  StructDeclNode(std::string identifier, std::vector<VariableDeclNode *> fields)
      : identifier(identifier), fields(fields) {}
};