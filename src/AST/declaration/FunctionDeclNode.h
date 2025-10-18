#pragma once

#include <string>
#include <vector>
#include "../statement/BlockStatementNode.h"
#include "../type/TypeNode.h"
#include "DeclarationNode.h"
#include "ParameterDeclNode.h"


// FunctionDeclaration for forward declaration
// FunctionDefinition for full definition
struct FunctionDeclNode : public DeclarationNode {
    std::string identifier;
    TypeNode* returnType;
    std::vector<ParameterDeclNode*> parameters;
    BlockStatementNode* body;  // null for forward declaration

    FunctionDeclNode(
        std::string identifier,
        TypeNode* returnType,
        std::vector<ParameterDeclNode*> parameters,
        BlockStatementNode* body
    )
        : identifier(identifier),
          returnType(returnType),
          parameters(parameters),
          body(body) {}

    void accept(CodeGen& codeGen) override {
        if (body)
            codeGen.visitFunctionDefinition(this);
        else
            codeGen.visitFunctionDeclaration(this);
    }
};