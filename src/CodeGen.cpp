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

void* LogError(const char* msg) {
    std::cerr << "Error: " << msg << "\n";
    return nullptr;
}

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
    Type* returnType = visitType(node->returnType);

    std::vector<Type*> paramTypes;
    for (auto* param : node->parameters) {
        paramTypes.push_back(visitType(param->type));
    }

    FunctionType* funcType = FunctionType::get(returnType, paramTypes, false);
    Function* function =
        Function::Create(funcType, Function::ExternalLinkage, node->identifier, m_module.get());

    // Set parameter names
    unsigned idx = 0;
    for (auto& arg : function->args()) {
        arg.setName(node->parameters[idx++]->identifier);
    }
    return function;
}

void CodeGen::visitFunctionDefinition(FunctionDeclNode* node) {
    Function* function = m_module->getFunction(node->identifier);
    if (!function)
        function = visitFunctionDeclaration(node);

    if (!function)
        return LogError("Unknown function: " + node->identifier);

    // If there's a body, generate it
    //if (node->body && !node->body->body.empty()) {
    BasicBlock* entry = BasicBlock::Create(*m_context, "entry", function);
    m_builder->SetInsertPoint(entry);

    // Add parameters to symbol table
    m_scopeCtx = Context::create(std::move(m_scopeCtx));
    for (auto& arg : function->args()) {
        m_scopeCtx->namedValues[std::string(arg.getName())] = &arg;
    }

    // Generate body
    Value* returnValue = visitBlockStatement(node->body);

    if (node->returnType->kind == PrimitiveTypeNode::Void)
        m_builder->CreateRetVoid();
    else
        m_builder->CreateRet(returnValue);

    verifyFunction(*function);

    m_scopeCtx = m_scopeCtx->destroy();
}

void CodeGen::visitVariableDeclaration(VariableDeclNode* node) {
    Type* type = visitType(node->type);
    if (node->arraySize) {
        int arrSize = dynamic_cast<IntegerLiteralNode*>(node->arraySize)->value;
        ArrayType* arrT = ArrayType::get(type, arrSize);
        m_builder->CreateAlloca(arrT, 0, node->identifier);

        if (node->initializer) {
            LogError("Array initializers not supported");
            return;
        }


    } else {
        m_builder->CreateAlloca(type, 0, node->identifier);
        if (node->initializer) {
            Value* initVal = visitExpression(node->initializer);
            m_builder->CreateStore(initVal, alloca);
        }
    }


    m_scopeCtx->namedValues[node->identifier] = alloca;
}

void CodeGen::visitStructDeclaration(StructDeclNode* node) {
    StructType* type = StructType::create(m_context, node->identifier);
    m_namedTypes[node->identifier] = type;
}

void CodeGen::visitStructDefinition(StructDeclNode* node) {
    StructType* type = m_namedTypes[node->identifier];
    if (!type) {
        visitStructDeclaration(node);
        type = m_namedTypes[node->identifier];
    }

    std::vector<Type*> fieldTypes;
    for (VariableDeclNode* field : node->fields) {
        Type* fieldType = visitType(field->type);
        fieldTypes.push_back(fieldType);
    }

    type->setBody(fieldTypes);
}

void CodeGen::visitFunctionPtrDeclaration(FunctionPtrDeclNode* node) {
    Type* returnType = visitType(node->returnType);
    std::vector<Type*> paramTypes;
    for (auto* param : node->parameters) {
        paramTypes.push_back(visitType(param->type));
    }
    FunctionType* funcType = FunctionType::get(returnType, paramTypes, false);
    Function* function =
        Function::Create(funcType, Function::ExternalLinkage, node->identifier, m_module.get());
    //TODO
}

// ==== Statement visitors ====
Value* CodeGen::visitBlockStatement(BlockStatementNode* node) {
    Value* lastValue = nullptr;
    for (auto* stmt : node->body) {
        lastValue = stmt->accept(*this);
    }
    return lastValue;
}

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
        Value* retVal = visitExpression(node->returnValue);
        m_builder->CreateRet(retVal);
    } else {
        m_builder->CreateRetVoid();
    }
}

// ==== Expression visitors ====
Value* CodeGen::visitBinaryExpr(BinaryExprNode* node) {
    Value* left = visitExpression(node->left);
    Value* right = visitExpression(node->right);

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
    } else if (node->op == "=") {
        m_builder->CreateStore(right, left);  // Left is identifier or index expression;
        return right;
    }

    std::cerr << "Unknown binary operator: " << node->op << "\n";
    return nullptr;
}

Value* CodeGen::visitUnaryExpr(UnaryExprNode* node) {
    Value* operand = visitExpression(node->operand);
    Value* oldVal = m_builder->CreateLoad(operand);
    Value* newVal = nullptr;

    if (node->op == "++") {
        newVal = m_builder->CreateAdd(oldVal, m_builder->getInt32(1));
        m_builder->CreateStore(newVal, operand);
    } else if (node->op == "--") {
        newVal = m_builder->CreateSub(oldVal, m_builder->getInt32(1));
        m_builder->CreateStore(newVal, operand);
    } else {
        return LogError("Unknown unary operator: " + node->op);
    }
    return node->prefix ? newVal : oldVal;
}

Value* CodeGen::visitIntegerLiteral(IntegerLiteralNode* node) {
    return ConstantInt::get(*m_context, APInt(32, node->value, true));
}

Value* CodeGen::visitDoubleLiteral(DoubleLiteralNode* node) {
    return ConstantFP::get(*m_context, APFloat(node->value));
}

Value* CodeGen::visitCharacterLiteral(CharacterLiteralNode* node) {
    return ConstantInt::get(*m_context, APInt(8, node->value, false));
}

Value* CodeGen::visitStringLiteral(StringLiteralNode* node) {
    // TODO: Implement string literal
    std::cout << "Generating string literal\n";
    return nullptr;
}

Value* CodeGen::visitIdentifierExpr(IdentifierExprNode* node) {
    Value* val = m_scopeCtx->get(node->name);
    if (!val) {
        std::cerr << "Unknown variable name: " << node->name << "\n";
        return nullptr;
    }
    return val;
}

Value* CodeGen::visitCallExpr(CallExprNode* node) {
    // TODO: Implement function calls
    std::cout << "Generating call expression\n";
    return nullptr;
}

Value* CodeGen::visitCastExpr(CastExprNode* node) {
    // TODO: Implement casts
    std::cout << "Generating cast expression\n";
    return nullptr;
}

Value* CodeGen::visitMemberExpr(MemberExprNode* node) {
    // TODO: Implement member access
    std::cout << "Generating member expression\n";
    return nullptr;
}

Value* CodeGen::visitIndexExpr(IndexExprNode* node) {
    // TODO: Implement array indexing
    std::cout << "Generating index expression\n";
    return nullptr;
}

// ==== Type visitors ====
Type* CodeGen::visitPrimitiveType(PrimitiveTypeNode* node) {
    switch (node->kind) {
        case PrimitiveTypeNode::Void: return Type::getVoidTy(*m_context);
        case PrimitiveTypeNode::Char: return Type::getInt8Ty(*m_context);
        case PrimitiveTypeNode::Int: return Type::getInt32Ty(*m_context);
        case PrimitiveTypeNode::Double: return Type::getDoubleTy(*m_context);
        default: return nullptr;
    }
}

Type* CodeGen::visitPointerType(PointerTypeNode* node) {
    Type* baseType = visitType(node->baseType);
    return PointerType::get(baseType, 0);
}

Type* CodeGen::visitNamedType(NamedTypeNode* node) {
    Type* type = m_namedTypes[node->identifier];
    if (!type) {
        return LogError("Unknown named type: " + node->identifier);
    }
    return type;
}
