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
    Type* returnType = node->returnType->accept(*this);

    std::vector<Type*> paramTypes;
    for (auto* param : node->parameters) {
        paramTypes.push_back(param->type->accept(*this));
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
    if (node->body->body.empty()) {
        //TODO warn empty function body?
    }

    BasicBlock* entry = BasicBlock::Create(*m_context, "entry", function);
    m_builder->SetInsertPoint(entry);

    // Add parameters to symbol table
    m_scopeCtx = Context::create(std::move(m_scopeCtx));
    for (auto& arg : function->args()) {
        m_scopeCtx->namedValues[std::string(arg.getName())] = &arg;
    }

    // Generate body
    visitBlockStatement(node->body);

    BasicBlock* currentBB = m_builder->GetInsertBlock();
    if (!currentBB->getTerminator()) {
        if (node->returnType->kind == PrimitiveTypeNode::Void) {
            m_builder->CreateRetVoid();
        } else {
            throw std::runtime_error("Missing return in non-void function: " + node->identifier);
        }
    }

    verifyFunction(*function);

    m_scopeCtx = m_scopeCtx->destroy();
}

void CodeGen::visitVariableDeclaration(VariableDeclNode* node) {
    Type* type = node->type->accept(*this);
    AllocaInst* alloca = nullptr;
    if (node->arraySize) {
        int arrSize = dynamic_cast<IntegerLiteralNode*>(node->arraySize)->value;
        ArrayType* arrT = ArrayType::get(type, arrSize);
        alloca = m_builder->CreateAlloca(arrT, 0, node->identifier);

        if (node->initializer) {
            return LogError("Array initializers not supported");
        }

    } else {
        alloca = m_builder->CreateAlloca(type, 0, node->identifier);
        if (node->initializer) {
            Value* initVal = getValueOf(node->initializer);
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
        Type* fieldType = field->type->accept(*this);
        fieldTypes.push_back(fieldType);
    }

    type->setBody(fieldTypes);
}

void CodeGen::visitFunctionPtrDeclaration(FunctionPtrDeclNode* node) {
    Type* returnType = node->returnType->accept(*this);
    std::vector<Type*> paramTypes;
    for (auto* param : node->parameters) {
        paramTypes.push_back(param->type->accept(*this));
    }
    FunctionType* funcType = FunctionType::get(returnType, paramTypes, false);
    Function* function =
        Function::Create(funcType, Function::ExternalLinkage, node->identifier, m_module.get());
    //TODO?
}

// ==== Statement visitors ====
void CodeGen::visitBlockStatement(BlockStatementNode* node) {
    for (auto* stmt : node->body) {
        stmt->accept(*this);
    }
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
        Value* retVal = getValueOf(node->returnValue);
        m_builder->CreateRet(retVal);
    } else {
        m_builder->CreateRetVoid();
    }
}

// ==== Expression visitors ====

// Helper to get the address (pointer) for l-values
Value* CodeGen::getAddressOf(ExpressionNode* node) {
    // For identifiers, return the pointer without loading
    if (auto* idNode = dynamic_cast<IdentifierExprNode*>(node)) {
        Value* ptr = m_scopeCtx->get(idNode->name);
        if (!ptr) {
            std::cerr << "Unknown variable name: " << idNode->name << "\n";
            return nullptr;
        }
        return ptr;
    }
    // TODO: Handle array indexing (arr[i]), member access (obj.field), dereferencing (*ptr), etc.
    std::cerr << "Cannot get address of this expression\n";
    return nullptr;
}

// Helper to get the value (loaded) for r-values
Value* CodeGen::getValueOf(ExpressionNode* node) {
    // For identifiers, load the value from the pointer
    if (auto* idNode = dynamic_cast<IdentifierExprNode*>(node)) {
        Value* ptr = m_scopeCtx->get(idNode->name);
        if (!ptr) {
            std::cerr << "Unknown variable name: " << idNode->name << "\n";
            return nullptr;
        }
        return m_builder->CreateLoad(ptr, idNode->name);  // Load the value
    }

    // For everything else (literals, expressions, calls, etc.), use the visitor
    return node->accept(*this);
}

Value* CodeGen::visitBinaryExpr(BinaryExprNode* node) {
    // Special handling for assignment operator
    if (node->op == "=") {
        Value* leftAddr = getAddressOf(node->left);  // Get address for l-value
        Value* rightVal = getValueOf(node->right);   // Get value for r-value
        m_builder->CreateStore(rightVal, leftAddr);
        return rightVal;
    }

    // For all other operators, evaluate both sides as r-values
    Value* left = getValueOf(node->left);
    Value* right = getValueOf(node->right);

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

Value* CodeGen::visitUnaryExpr(UnaryExprNode* node) {
    if (node->op == "++" || node->op == "--") {
        Value* addr = getAddressOf(node->operand);
        Value* oldVal = m_builder->CreateLoad(addr);
        Value* newVal = nullptr;

        if (node->op == "++") {
            newVal = m_builder->CreateAdd(oldVal, m_builder->getInt32(1));
        } else {
            newVal = m_builder->CreateSub(oldVal, m_builder->getInt32(1));
        }

        m_builder->CreateStore(newVal, addr);
        return node->prefix ? newVal : oldVal;
    }

    if (node->op == "-") {
        Value* operand = getValueOf(node->operand);
        return m_builder->CreateNeg(operand, "negtmp");
    } else if (node->op == "!") {
        Value* operand = getValueOf(node->operand);
        return m_builder->CreateNot(operand, "nottmp");
    } else if (node->op == "&") {
        return getAddressOf(node->operand);
    } else if (node->op == "*") {
        Value* ptr = getValueOf(node->operand);
        return m_builder->CreateLoad(ptr, "dereftmp");
    }

    return LogError("Unknown unary operator: " + node->op);
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
    return ConstantDataArray::getString(*m_context, node->value);
}

Value* CodeGen::visitIdentifierExpr(IdentifierExprNode* node) {
    throw std::runtime_error(
        "visitIdentifierExpr should not be called directly, use getAddressOf() or getValueOf() "
        "instead"
    );
    return nullptr;
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

Value* CodeGen::visitCommaExpr(CommaExprNode* node) {
    Value* lastValue = nullptr;
    for (auto* expr : node->expressions) {
        lastValue = expr->accept(*this);
    }
    return lastValue;
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
    Type* baseType = node->baseType->accept(*this);
    return PointerType::get(baseType, 0);
}

Type* CodeGen::visitNamedType(NamedTypeNode* node) {
    Type* type = m_namedTypes[node->identifier];
    if (!type) {
        return LogError("Unknown named type: " + node->identifier);
    }
    return type;
}
