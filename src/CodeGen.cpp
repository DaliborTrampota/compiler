#include "CodeGen.h"

#include <iostream>
#include <string>


#include "AST/ProgramNode.h"
#include "AST/declaration/DeclarationNode.h"
#include "AST/declaration/FunctionDeclNode.h"
#include "AST/declaration/ParameterDeclNode.h"
#include "AST/declaration/StructDeclNode.h"
#include "AST/declaration/VariableDeclNode.h"


#include "AST/statement/BlockStatementNode.h"
#include "AST/statement/DoWhileStatementNode.h"
#include "AST/statement/ExpressionStatementNode.h"
#include "AST/statement/ForStatementNode.h"
#include "AST/statement/IfStatementNode.h"
#include "AST/statement/ReturnStatementNode.h"
#include "AST/statement/StatementNode.h"
#include "AST/statement/WhileStatementNode.h"


#include "AST/expression/BinaryExprNode.h"
#include "AST/expression/CallExprNode.h"
#include "AST/expression/CastExprNode.h"
#include "AST/expression/CommaExprNode.h"
#include "AST/expression/ExpressionNode.h"
#include "AST/expression/IdentifierExprNode.h"
#include "AST/expression/IndexExprNode.h"
#include "AST/expression/LiteralExprNode.h"
#include "AST/expression/MemberExprNode.h"
#include "AST/expression/UnaryExprNode.h"


#include "AST/type/NamedTypeNode.h"
#include "AST/type/PointerTypeNode.h"
#include "AST/type/PrimitiveTypeNode.h"
#include "AST/type/TypeNode.h"


using namespace llvm;

CodeGen::CodeGen(const std::string& moduleName)
    : m_context(std::make_unique<LLVMContext>()),
      m_module(std::make_unique<Module>(moduleName, *m_context)),
      m_builder(std::make_unique<IRBuilder<>>(*m_context)) {}

void CodeGen::generate(ProgramNode* program) {
    for (auto* declaration : program->declarations) {
        declaration->accept(*this);
    }
}

// ==== Accessor methods ====
LLVMContext* CodeGen::getContext() {
    return m_context.get();
}

Module* CodeGen::getModule() {
    return m_module.get();
}

IRBuilder<>* CodeGen::getBuilder() {
    return m_builder.get();
}

// ==== Declaration visitors ====
void CodeGen::visitFunctionDeclaration(FunctionDeclNode* node) {
    // Example: Create function signature
    llvm::Type* returnType = visitType(node->returnType);

    std::vector<llvm::Type*> paramTypes;
    for (auto* param : node->parameters) {
        paramTypes.push_back(visitType(param->type));
    }

    llvm::FunctionType* funcType = llvm::FunctionType::get(returnType, paramTypes, false);
    llvm::Function* function = llvm::Function::Create(
        funcType, llvm::Function::ExternalLinkage, node->identifier, m_module.get()
    );

    // Set parameter names
    unsigned idx = 0;
    for (auto& arg : function->args()) {
        arg.setName(node->parameters[idx++]->identifier);
    }

    // If there's a body, generate it
    if (node->body && !node->body->body.empty()) {
        llvm::BasicBlock* entry = llvm::BasicBlock::Create(*m_context, "entry", function);
        m_builder->SetInsertPoint(entry);

        // Add parameters to symbol table
        m_namedValues.clear();
        for (auto& arg : function->args()) {
            m_namedValues[std::string(arg.getName())] = &arg;
        }

        // Generate body
        visitBlockStatement(node->body);
    }
}

void CodeGen::visitVariableDeclaration(VariableDeclNode* node) {
    // TODO: Implement global variable declaration
    std::cout << "Generating variable: " << node->identifier << "\n";
}

void CodeGen::visitStructDeclaration(StructDeclNode* node) {
    // TODO: Implement struct type creation
    std::cout << "Generating struct: " << node->identifier << "\n";
}

void CodeGen::visitStructDefinition(StructDeclNode* node) {
    // TODO: Implement struct definition
    std::cout << "Generating struct: " << node->identifier << "\n";
}

// ==== Statement visitors ====
void CodeGen::visitIfStatement(IfStatementNode* node) {
    // TODO: Implement if statement
    std::cout << "Generating if statement\n";
}

void CodeGen::visitWhileStatement(WhileStatementNode* node) {
    // TODO: Implement while loop
    std::cout << "Generating while statement\n";
}

void CodeGen::visitDoWhileStatement(DoWhileStatementNode* node) {
    // TODO: Implement do-while loop
    std::cout << "Generating do-while statement\n";
}

void CodeGen::visitForStatement(ForStatementNode* node) {
    // TODO: Implement for loop
    std::cout << "Generating for statement\n";
}

void CodeGen::visitReturnStatement(ReturnStatementNode* node) {
    if (node->returnValue) {
        llvm::Value* retVal = visitExpression(node->returnValue);
        m_builder->CreateRet(retVal);
    } else {
        m_builder->CreateRetVoid();
    }
}

// ==== Expression visitors ====
llvm::Value* CodeGen::visitBinaryExpr(BinaryExprNode* node) {
    llvm::Value* left = visitExpression(node->left);
    llvm::Value* right = visitExpression(node->right);

    if (node->op == "+") {
        return m_builder->CreateAdd(left, right, "addtmp");
    } else if (node->op == "-") {
        return m_builder->CreateSub(left, right, "subtmp");
    } else if (node->op == "*") {
        return m_builder->CreateMul(left, right, "multmp");
    } else if (node->op == "/") {
        return m_builder->CreateSDiv(left, right, "divtmp");
    } else if (node->op == "<") {
        return m_builder->CreateICmpSLT(left, right, "cmptmp");
    } else if (node->op == ">") {
        return m_builder->CreateICmpSGT(left, right, "cmptmp");
    } else if (node->op == "==") {
        return m_builder->CreateICmpEQ(left, right, "cmptmp");
    } else if (node->op == "!=") {
        return m_builder->CreateICmpNE(left, right, "cmptmp");
    }

    std::cerr << "Unknown binary operator: " << node->op << "\n";
    return nullptr;
}

llvm::Value* CodeGen::visitUnaryExpr(UnaryExprNode* node) {
    // TODO: Implement unary operators
    std::cout << "Generating unary expression\n";
    return nullptr;
}

llvm::Value* CodeGen::visitIntegerLiteral(IntegerLiteralNode* node) {
    return llvm::ConstantInt::get(*m_context, llvm::APInt(32, node->value, true));
}

llvm::Value* CodeGen::visitDoubleLiteral(DoubleLiteralNode* node) {
    return llvm::ConstantFP::get(*m_context, llvm::APFloat(node->value));
}

llvm::Value* CodeGen::visitCharacterLiteral(CharacterLiteralNode* node) {
    return llvm::ConstantInt::get(*m_context, llvm::APInt(8, node->value, false));
}

llvm::Value* CodeGen::visitStringLiteral(StringLiteralNode* node) {
    // TODO: Implement string literal
    std::cout << "Generating string literal\n";
    return nullptr;
}

llvm::Value* CodeGen::visitIdentifierExpr(IdentifierExprNode* node) {
    llvm::Value* val = m_namedValues[node->name];
    if (!val) {
        std::cerr << "Unknown variable name: " << node->name << "\n";
        return nullptr;
    }
    return val;
}

llvm::Value* CodeGen::visitCallExpr(CallExprNode* node) {
    // TODO: Implement function calls
    std::cout << "Generating call expression\n";
    return nullptr;
}

llvm::Value* CodeGen::visitCastExpr(CastExprNode* node) {
    // TODO: Implement casts
    std::cout << "Generating cast expression\n";
    return nullptr;
}

llvm::Value* CodeGen::visitMemberExpr(MemberExprNode* node) {
    // TODO: Implement member access
    std::cout << "Generating member expression\n";
    return nullptr;
}

llvm::Value* CodeGen::visitIndexExpr(IndexExprNode* node) {
    // TODO: Implement array indexing
    std::cout << "Generating index expression\n";
    return nullptr;
}

// ==== Type visitors ====
llvm::Type* CodeGen::visitPrimitiveType(PrimitiveTypeNode* node) {
    switch (node->kind) {
        case PrimitiveTypeNode::Void: return llvm::Type::getVoidTy(*m_context);
        case PrimitiveTypeNode::Char: return llvm::Type::getInt8Ty(*m_context);
        case PrimitiveTypeNode::Int: return llvm::Type::getInt32Ty(*m_context);
        case PrimitiveTypeNode::Double: return llvm::Type::getDoubleTy(*m_context);
        default: return nullptr;
    }
}

llvm::Type* CodeGen::visitPointerType(PointerTypeNode* node) {
    llvm::Type* baseType = visitType(node->baseType);
    return llvm::PointerType::get(baseType, 0);
}

llvm::Type* CodeGen::visitNamedType(NamedTypeNode* node) {
    // TODO: Look up struct type from module
    std::cout << "Looking up named type: " << node->identifier << "\n";
    return nullptr;
}
