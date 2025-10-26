#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "Context.h"


#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>

#include "AST/forwardDecl.h"

using namespace llvm;


class CodeGen {
  private:
    std::unique_ptr<Context> m_scopeCtx = Context::create(nullptr, Context::Global);

    std::unique_ptr<LLVMContext> m_context;
    std::unique_ptr<Module> m_module;
    std::unique_ptr<IRBuilder<>> m_builder;

    // Symbol table for variables
    std::unordered_map<std::string, llvm::Type*> m_namedTypes;

    //llvm::Value* m_lastValue

  public:
    CodeGen(const std::string& moduleName);

    LLVMContext* getContext();
    Module* getModule();
    IRBuilder<>* getBuilder();

    void generate(ProgramNode* program);

    // Visitor methods for declarations
    void visitFunctionDeclaration(FunctionDeclNode* node);
    void visitFunctionDefinition(FunctionDeclNode* node);
    void visitFunctionPtrDeclaration(FunctionPtrDeclNode* node);
    void visitVariableDeclaration(VariableDeclNode* node);
    void visitStructDeclaration(StructDeclNode* node);
    void visitStructDefinition(StructDeclNode* node);

    // Visitor methods for statements
    void visitBlockStatement(BlockStatementNode* node);
    // TODO
    void visitIfStatement(IfStatementNode* node);
    void visitWhileStatement(WhileStatementNode* node);
    void visitDoWhileStatement(DoWhileStatementNode* node);
    void visitForStatement(ForStatementNode* node);
    void visitReturnStatement(ReturnStatementNode* node);
    void visitCaseStatement(CaseStatementNode* node);
    void visitSwitchStatement(SwitchStatementNode* node);

    // Visitor methods for expressions
    llvm::Value* visitBinaryExpr(BinaryExprNode* node);
    llvm::Value* visitUnaryExpr(UnaryExprNode* node);

    // Helpers for getting addresses and values
    llvm::Value* getAddressOf(ExpressionNode* node);  // Get pointer (for l-values)
    llvm::Value* getValueOf(ExpressionNode* node);    // Get loaded value (for r-values)

    llvm::Value* visitIntegerLiteral(IntegerLiteralNode* node);
    llvm::Value* visitDoubleLiteral(DoubleLiteralNode* node);
    llvm::Value* visitCharacterLiteral(CharacterLiteralNode* node);
    llvm::Value* visitStringLiteral(StringLiteralNode* node);
    // TODO
    llvm::Value* visitIdentifierExpr(IdentifierExprNode* node);
    llvm::Value* visitCallExpr(CallExprNode* node);
    llvm::Value* visitCastExpr(CastExprNode* node);
    llvm::Value* visitMemberExpr(MemberExprNode* node);
    llvm::Value* visitIndexExpr(IndexExprNode* node);
    llvm::Value* visitCommaExpr(CommaExprNode* node);

    // Visitor methods for types
    llvm::Type* visitPrimitiveType(PrimitiveTypeNode* node);
    llvm::Type* visitPointerType(PointerTypeNode* node);
    llvm::Type* visitNamedType(NamedTypeNode* node);
};