#pragma once

#include <map>
#include <memory>
#include <string>


#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>

#include "AST/forwardDecl.h"

using namespace llvm;


class CodeGen {
  private:
    std::unique_ptr<LLVMContext> m_context;
    std::unique_ptr<Module> m_module;
    std::unique_ptr<IRBuilder<>> m_builder;

    // Symbol table for variables
    std::map<std::string, llvm::Value*> m_namedValues;

  public:
    CodeGen(const std::string& moduleName);

    LLVMContext* getContext();
    Module* getModule();
    IRBuilder<>* getBuilder();

    void generate(ProgramNode* program);

    // Visitor methods for declarations
    void visitFunctionDeclaration(FunctionDeclNode* node);
    void visitFunctionDefinition(FunctionDeclNode* node);
    void visitVariableDeclaration(VariableDeclNode* node);
    void visitStructDeclaration(StructDeclNode* node);
    void visitStructDefinition(StructDeclNode* node);

    // Visitor methods for statements
    void visitIfStatement(IfStatementNode* node);            // todo
    void visitWhileStatement(WhileStatementNode* node);      // todo
    void visitDoWhileStatement(DoWhileStatementNode* node);  // todo
    void visitForStatement(ForStatementNode* node);          // todo
    void visitReturnStatement(ReturnStatementNode* node);    // todo
    void visitCaseStatement(CaseStatementNode* node);        // todo
    void visitSwitchStatement(SwitchStatementNode* node);    // todo

    // Visitor methods for expressions (return llvm::Value*)
    llvm::Value* visitBinaryExpr(BinaryExprNode* node);
    llvm::Value* visitUnaryExpr(UnaryExprNode* node);

    llvm::Value* visitIntegerLiteral(IntegerLiteralNode* node);
    llvm::Value* visitDoubleLiteral(DoubleLiteralNode* node);
    llvm::Value* visitCharacterLiteral(CharacterLiteralNode* node);
    llvm::Value* visitStringLiteral(StringLiteralNode* node);

    llvm::Value* visitIdentifierExpr(IdentifierExprNode* node);
    llvm::Value* visitCallExpr(CallExprNode* node);
    llvm::Value* visitCastExpr(CastExprNode* node);
    llvm::Value* visitMemberExpr(MemberExprNode* node);
    llvm::Value* visitIndexExpr(IndexExprNode* node);

    // Visitor methods for types
    llvm::Type* visitPrimitiveType(PrimitiveTypeNode* node);
    llvm::Type* visitPointerType(PointerTypeNode* node);
    llvm::Type* visitNamedType(NamedTypeNode* node);
};