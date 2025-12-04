#include "CodeGen.h"

#include <iostream>
#include <string>

#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>

#include "AST/includeNodes.h"


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

    // Verify the entire module for consistency
    std::string errorMsg;
    raw_string_ostream errorStream(errorMsg);
    if (verifyModule(*m_module, &errorStream)) {
        std::cerr << "Module verification failed:\n" << errorMsg << "\n";
        throw std::runtime_error("Invalid LLVM IR generated");
    }
}

void CodeGen::print() {
    m_module->print(outs(), nullptr);
}

void CodeGen::printToFile(const std::string& filename) {
    std::error_code EC;
    raw_fd_ostream file(filename, EC);
    if (EC) {
        std::cerr << "Error opening file: " << EC.message() << "\n";
        return;
    }
    m_module->print(file, nullptr);
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
    //return function;
}

void CodeGen::visitFunctionDefinition(FunctionDeclNode* node) {
    Function* function = m_module->getFunction(node->identifier);
    if (!function) {
        visitFunctionDeclaration(node);
        function = m_module->getFunction(node->identifier);
    }

    // gen function body
    BasicBlock* entry = BasicBlock::Create(*m_context, "entry", function);
    m_builder->SetInsertPoint(entry);

    // Add parameters to symbol table
    m_scopeCtx = Context::create(std::move(m_scopeCtx));
    unsigned idx = 0;
    for (auto& arg : function->args()) {
        std::string argName = std::string(arg.getName());
        m_scopeCtx->set(argName, &arg);
        m_scopeCtx->setType(argName, node->parameters[idx]->type->accept(*this));
        m_scopeCtx->setTypeNode(argName, node->parameters[idx]->type);  // Track AST type
        idx++;
    }

    // Generate body
    visitBlockStatement(node->body);

    BasicBlock* currentBB = m_builder->GetInsertBlock();
    if (!currentBB->getTerminator()) {
        // Check if return type is void using the LLVM type
        Type* returnType = node->returnType->accept(*this);
        if (returnType->isVoidTy()) {
            m_builder->CreateRetVoid();
        } else {
            throw std::runtime_error("Missing return in non-void function: " + node->identifier);
        }
    }

    verifyFunction(*function);

    m_scopeCtx = m_scopeCtx->destroy();
}
//TODO global and constant variables
void CodeGen::visitVariableDeclaration(VariableDeclNode* node) {
    Type* type = node->type->accept(*this);
    AllocaInst* allocaInst = nullptr;
    if (node->arraySize) {
        int arrSize = dynamic_cast<IntegerLiteralNode*>(node->arraySize)->value;
        ArrayType* arrT = ArrayType::get(type, arrSize);
        allocaInst = m_builder->CreateAlloca(arrT, 0, node->identifier);

        // TODO: Array initializers not yet supported
        assert(!node->initializer && "Array initializers not implemented");

    } else {
        allocaInst = m_builder->CreateAlloca(type, 0, node->identifier);
        if (node->initializer) {
            auto* idNode = dynamic_cast<IdentifierExprNode*>(node->initializer);
            Function* func = idNode ? m_module->getFunction(idNode->name) : nullptr;
            if (func) {
                m_builder->CreateStore(func, allocaInst);
            } else {
                Value* initVal = getValueOf(node->initializer);
                m_builder->CreateStore(initVal, allocaInst);
            }
        }
    }

    m_scopeCtx->set(node->identifier, allocaInst);
    m_scopeCtx->setType(node->identifier, type);
    m_scopeCtx->setTypeNode(node->identifier, node->type);  // Track AST type for opaque pointers
}

void CodeGen::visitStructDeclaration(StructDeclNode* node) {
    StructType* type = StructType::create(*m_context, node->identifier);
    m_namedTypes[node->identifier] = type;

    StructInfo info{type, {}};
    m_structInfos[node->identifier] = info;
}

void CodeGen::visitStructDefinition(StructDeclNode* node) {
    StructType* type = dyn_cast_or_null<StructType>(m_namedTypes[node->identifier]);
    if (!type) {
        visitStructDeclaration(node);
        type = cast<StructType>(m_namedTypes[node->identifier]);
    }

    std::vector<Type*> fieldTypes;
    StructInfo& info = m_structInfos[node->identifier];
    for (VariableDeclNode* field : node->fields) {
        Type* fieldType = field->type->accept(*this);
        fieldTypes.push_back(fieldType);
        unsigned fieldIndex = static_cast<unsigned>(fieldTypes.size() - 1);
        info.fields[field->identifier] = {fieldIndex, fieldType};
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

    // For opaque pointers, just store the FunctionType (not PointerType)
    // When creating variables with this type, they'll be pointers automatically
    m_namedTypes[node->identifier] = funcType;
}

// ==== Statement visitors ====
void CodeGen::visitBlockStatement(BlockStatementNode* node) {
    for (auto* stmt : node->body) {
        stmt->accept(*this);
    }
}

Value* CodeGen::convertToBoolean(Value* condValue) {
    if (condValue->getType()->isIntegerTy()) {
        // Integer: compare with 0
        return m_builder->CreateICmpNE(
            condValue, ConstantInt::get(condValue->getType(), 0), "ifcond"
        );
    } else if (condValue->getType()->isFloatingPointTy()) {
        // Float: compare with 0.0
        return m_builder->CreateFCmpONE(
            condValue, ConstantFP::get(condValue->getType(), 0.0), "ifcond"
        );
    } else if (condValue->getType()->isPointerTy()) {
        // Pointer: compare with null
        return m_builder->CreateICmpNE(
            condValue, ConstantPointerNull::get(cast<PointerType>(condValue->getType())), "ifcond"
        );
    } else {
        // Already a boolean (i1)
        return condValue;
    }
    return nullptr;
}

void CodeGen::visitIfStatement(IfStatementNode* node) {
    Value* condValue = getValueOf(node->condition);
    assert(condValue != nullptr && "Condition is nullptr");
    // Convert condition to boolean (i1)
    Value* condition = convertToBoolean(condValue);
    assert(condition != nullptr && "Condition is nullptr");

    Function* func = m_builder->GetInsertBlock()->getParent();

    // Create basic blocks
    BasicBlock* thenBB = BasicBlock::Create(*m_context, "if.then", func);
    BasicBlock* mergeBB = BasicBlock::Create(*m_context, "if.end");
    BasicBlock* elseBB = node->elseBody ? BasicBlock::Create(*m_context, "if.else") : mergeBB;

    // Branch based on condition
    m_builder->CreateCondBr(condition, thenBB, elseBB);

    // Emit then block
    m_builder->SetInsertPoint(thenBB);
    node->thenBody->accept(*this);
    if (!m_builder->GetInsertBlock()->getTerminator()) {
        m_builder->CreateBr(mergeBB);
    }

    // Emit else block if it exists
    if (node->elseBody) {
        func->insert(func->end(), elseBB);
        m_builder->SetInsertPoint(elseBB);
        node->elseBody->accept(*this);
        if (!m_builder->GetInsertBlock()->getTerminator()) {
            m_builder->CreateBr(mergeBB);
        }
    }

    // Emit merge block
    func->insert(func->end(), mergeBB);
    m_builder->SetInsertPoint(mergeBB);
}

void CodeGen::visitWhileStatement(WhileStatementNode* node) {
    Function* func = m_builder->GetInsertBlock()->getParent();

    BasicBlock* condBB = BasicBlock::Create(*m_context, "while.cond", func);
    BasicBlock* bodyBB = BasicBlock::Create(*m_context, "while.body");
    BasicBlock* endBB = BasicBlock::Create(*m_context, "while.end");

    m_builder->CreateBr(condBB);
    m_builder->SetInsertPoint(condBB);
    if (node->condition) {
        Value* condValue = getValueOf(node->condition);
        assert(condValue != nullptr && "Condition is nullptr");
        Value* condition = convertToBoolean(condValue);
        assert(condition != nullptr && "Condition is nullptr");
        m_builder->CreateCondBr(condition, bodyBB, endBB);
    } else {
        m_builder->CreateBr(bodyBB);
    }

    func->insert(func->end(), bodyBB);
    m_builder->SetInsertPoint(bodyBB);
    node->loopBody->accept(*this);
    if (!m_builder->GetInsertBlock()->getTerminator()) {
        m_builder->CreateBr(condBB);
    }

    func->insert(func->end(), endBB);
    m_builder->SetInsertPoint(endBB);
}

void CodeGen::visitDoWhileStatement(DoWhileStatementNode* node) {
    Function* func = m_builder->GetInsertBlock()->getParent();

    BasicBlock* bodyBB = BasicBlock::Create(*m_context, "do.while.body");
    BasicBlock* condBB = BasicBlock::Create(*m_context, "do.while.cond");
    BasicBlock* endBB = BasicBlock::Create(*m_context, "do.while.end");

    m_builder->CreateBr(bodyBB);
    func->insert(func->end(), bodyBB);
    m_builder->SetInsertPoint(bodyBB);
    node->loopBody->accept(*this);
    if (!m_builder->GetInsertBlock()->getTerminator()) {
        m_builder->CreateBr(condBB);
    }

    func->insert(func->end(), condBB);
    m_builder->SetInsertPoint(condBB);
    if (node->condition) {
        Value* condValue = getValueOf(node->condition);
        assert(condValue != nullptr && "Condition is nullptr");
        Value* condition = convertToBoolean(condValue);
        assert(condition != nullptr && "Condition is nullptr");
        m_builder->CreateCondBr(condition, bodyBB, endBB);
    } else {
        m_builder->CreateBr(bodyBB);
    }

    func->insert(func->end(), endBB);
    m_builder->SetInsertPoint(endBB);
}

void CodeGen::visitForStatement(ForStatementNode* node) {
    Function* func = m_builder->GetInsertBlock()->getParent();

    // initialize loop variable
    BasicBlock* preheaderBB = BasicBlock::Create(*m_context, "for.init", func);
    m_builder->CreateBr(preheaderBB);
    m_builder->SetInsertPoint(preheaderBB);

    if (std::holds_alternative<ExpressionNode*>(node->init)) {
        std::get<ExpressionNode*>(node->init)->accept(*this);
    } else {
        std::get<DeclarationNode*>(node->init)->accept(*this);
    }


    BasicBlock* condBB = BasicBlock::Create(*m_context, "for.cond", func);
    BasicBlock* bodyBB = BasicBlock::Create(*m_context, "for.body");
    BasicBlock* incBB = BasicBlock::Create(*m_context, "for.increment");
    BasicBlock* endBB = BasicBlock::Create(*m_context, "for.end");

    m_builder->CreateBr(condBB);
    m_builder->SetInsertPoint(condBB);
    if (node->condition) {
        Value* condValue = getValueOf(node->condition);
        assert(condValue != nullptr && "Condition is nullptr");
        Value* condition = convertToBoolean(condValue);
        assert(condition != nullptr && "Condition is nullptr");
        m_builder->CreateCondBr(condition, bodyBB, endBB);
    } else {
        m_builder->CreateBr(bodyBB);
    }

    func->insert(func->end(), bodyBB);
    m_builder->SetInsertPoint(bodyBB);
    node->body->accept(*this);
    if (!m_builder->GetInsertBlock()->getTerminator()) {
        m_builder->CreateBr(incBB);
    }

    func->insert(func->end(), incBB);
    m_builder->SetInsertPoint(incBB);
    if (node->update) {
        node->update->accept(*this);
    }
    m_builder->CreateBr(condBB);

    func->insert(func->end(), endBB);
    m_builder->SetInsertPoint(endBB);
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
    if (auto* idNode = dynamic_cast<IdentifierExprNode*>(node)) {
        Value* ptr = m_scopeCtx->get(idNode->name);
        if (!ptr) {
            std::cerr << "Unknown variable name: " << idNode->name << "\n";
            return nullptr;
        }
        return ptr;
    }
    if (auto* memberNode = dynamic_cast<MemberExprNode*>(node)) {
        return visitMemberExpr(memberNode);
    }
    if (auto* indexNode = dynamic_cast<IndexExprNode*>(node)) {
        return visitIndexExpr(indexNode);
    }

    if (auto* unaryNode = dynamic_cast<UnaryExprNode*>(node)) {
        if (unaryNode->op == "*") {
            return getValueOf(unaryNode->operand);
        }
    }

    llvm_unreachable("Cannot get address of this expression");
}

// Helper to get the value (loaded) for r-values
Value* CodeGen::getValueOf(ExpressionNode* node) {
    if (auto* idNode = dynamic_cast<IdentifierExprNode*>(node)) {
        Value* ptr = m_scopeCtx->get(idNode->name);
        if (!ptr) {
            std::cerr << "Unknown variable name: " << idNode->name << "\n";
            return nullptr;
        }
        Type* type = m_scopeCtx->getType(idNode->name);
        return m_builder->CreateLoad(type, ptr, idNode->name);
    }
    if (auto* memberNode = dynamic_cast<MemberExprNode*>(node)) {
        Value* ptr = visitMemberExpr(memberNode);
        // Get field type from struct info
        IdentifierExprNode* objId = dynamic_cast<IdentifierExprNode*>(memberNode->object);
        TypeNode* objTypeNode = m_scopeCtx->getTypeNode(objId->name);

        StructType* structType = nullptr;
        if (memberNode->arrowAccess) {
            PointerTypeNode* ptrType = dynamic_cast<PointerTypeNode*>(objTypeNode);
            structType = cast<StructType>(ptrType->baseType->accept(*this));
        } else {
            structType = cast<StructType>(objTypeNode->accept(*this));
        }

        std::string structName = structType->getName().str();
        Type* fieldType = m_structInfos[structName].fields[memberNode->member].second;
        return m_builder->CreateLoad(fieldType, ptr, "member");
    }

    if (auto* indexNode = dynamic_cast<IndexExprNode*>(node)) {
        Value* ptr = visitIndexExpr(indexNode);
        // Get element type from array type
        IdentifierExprNode* arrId = dynamic_cast<IdentifierExprNode*>(indexNode->array);
        Type* arrayType = m_scopeCtx->getType(arrId->name);

        // arrayType is [N x T], get T
        Type* elemType = cast<ArrayType>(arrayType)->getElementType();
        return m_builder->CreateLoad(elemType, ptr, "elem");
    }

    // For everything else (literals, expressions, calls, etc.)
    return node->accept(*this);
}

Value* CodeGen::visitBinaryExpr(BinaryExprNode* node) {
    if (node->op == "=") {
        Value* leftAddr = getAddressOf(node->left);  // Get address for l-value
        Value* rightVal = getValueOf(node->right);   // Get value for r-value
        m_builder->CreateStore(rightVal, leftAddr);
        return rightVal;
    }

    // Both sides as r-values
    Value* left = getValueOf(node->left);
    Value* right = getValueOf(node->right);

    if (node->op == "+") {
        return m_builder->CreateAdd(left, right, "add");
    } else if (node->op == "-") {
        return m_builder->CreateSub(left, right, "sub");
    } else if (node->op == "*") {
        return m_builder->CreateMul(left, right, "mul");
    } else if (node->op == "/") {
        return m_builder->CreateSDiv(left, right, "div");
    } else if (node->op == "<") {
        return m_builder->CreateICmpSLT(left, right, "cmp_lt");
    } else if (node->op == ">") {
        return m_builder->CreateICmpSGT(left, right, "cmp_gt");
    } else if (node->op == "==") {
        return m_builder->CreateICmpEQ(left, right, "cmp_eq");
    } else if (node->op == "!=") {
        return m_builder->CreateICmpNE(left, right, "cmp_ne");
    }

    llvm_unreachable("Unknown binary operator");
}

Value* CodeGen::visitUnaryExpr(UnaryExprNode* node) {
    if (node->op == "++" || node->op == "--") {
        Value* addr = getAddressOf(node->operand);
        // Get the type for the load
        Type* type = nullptr;
        if (auto* idNode = dynamic_cast<IdentifierExprNode*>(node->operand)) {
            type = m_scopeCtx->getType(idNode->name);
        } else {
            // For member/index expressions, get from allocaInst
            if (auto* allocaInst = dyn_cast<AllocaInst>(addr)) {
                type = allocaInst->getAllocatedType();
            } else {
                llvm_unreachable("Cannot determine type for increment/decrement");
            }
        }
        Value* oldVal = m_builder->CreateLoad(type, addr);
        Value* newVal = nullptr;

        if (node->op == "++") {
            newVal = m_builder->CreateAdd(oldVal, m_builder->getInt32(1), "inc");
        } else {
            newVal = m_builder->CreateSub(oldVal, m_builder->getInt32(1), "dec");
        }

        m_builder->CreateStore(newVal, addr);
        return node->prefix ? newVal : oldVal;
    }

    if (node->op == "-") {
        Value* operand = getValueOf(node->operand);
        return m_builder->CreateNeg(operand, "neg");
    } else if (node->op == "!") {
        Value* operand = getValueOf(node->operand);
        return m_builder->CreateNot(operand, "not");
    } else if (node->op == "&") {
        return getAddressOf(node->operand);
    } else if (node->op == "*") {
        Value* ptr = getValueOf(node->operand);
        // For dereference, we need the pointee type
        // Get from the TypeNode - the operand should have a pointer type
        if (auto* idNode = dynamic_cast<IdentifierExprNode*>(node->operand)) {
            TypeNode* astType = m_scopeCtx->getTypeNode(idNode->name);
            PointerTypeNode* ptrType = dynamic_cast<PointerTypeNode*>(astType);
            Type* pointeeType = ptrType->baseType->accept(*this);
            return m_builder->CreateLoad(pointeeType, ptr, "deref");
        }
        llvm_unreachable("Dereference on complex expressions not yet supported");
    }

    llvm_unreachable("Unknown unary operator");
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
    llvm_unreachable(
        "visitIdentifierExpr called directly instead of getAddressOf() or getValueOf()"
    );
}

Value* CodeGen::visitCallExpr(CallExprNode* node) {
    std::vector<Value*> args;
    for (auto* arg : node->arguments) {
        args.push_back(getValueOf(arg));
    }

    Function* func = m_module->getFunction(node->callee->name);
    if (func) {
        // Direct function call: foo(args)
        // Only give it a name if it returns a value (not void)
        const char* name = func->getReturnType()->isVoidTy() ? "" : "direct_call";
        return m_builder->CreateCall(func, args, name);
    }

    // Indirect call through function pointer
    Value* funcPtr = getValueOf(node->callee);

    // For opaque pointers, we need the FunctionType
    // The variable's type is the function type (we stored it when creating the variable)
    // For function pointer variables, the stored type should be the function type
    Type* varType = m_scopeCtx->getType(node->callee->name);
    FunctionType* funcType = cast<FunctionType>(varType);

    // Only give it a name if it returns a value (not void)
    const char* name = funcType->getReturnType()->isVoidTy() ? "" : "indirect_call";
    return m_builder->CreateCall(funcType, funcPtr, args, name);
}

Value* CodeGen::visitCastExpr(CastExprNode* node) {
    // TODO: Implement casts
    std::cout << "Generating cast expression\n";
    return nullptr;
}

// This is mostly ai work, i implemented it myself with llvm docs but it was not working on llvm ir v22 due to opaque pointers stuff i dont understand. my solution was 5 lines using getPointerElementType, now i need to track all types
Value* CodeGen::visitMemberExpr(MemberExprNode* node) {
    Value* structPtr = node->arrowAccess ? getValueOf(node->object) : getAddressOf(node->object);

    // For opaque pointers (LLVM 15+), get the type from the value itself
    Type* structType = nullptr;

    if (node->arrowAccess) {
        // obj->field: structPtr is a loaded pointer value
        // For a pointer loaded from an allocaInst, we need to know what it points to
        // Use the stored type from Context
        if (auto* idNode = dynamic_cast<IdentifierExprNode*>(node->object)) {
            Type* varType = m_scopeCtx->getType(idNode->name);
            // varType should be the struct type (stored when variable was created)
            structType = varType;
        }
    } else {
        // obj.field: structPtr is a pointer to struct (AllocaInst or Argument)
        if (auto* allocaInst = dyn_cast<AllocaInst>(structPtr)) {
            // Use getAllocatedType() as per LLVM opaque pointer migration guide
            structType = allocaInst->getAllocatedType();
        } else if (isa<Argument>(structPtr)) {
            // For arguments, we need to use stored type
            if (auto* idNode = dynamic_cast<IdentifierExprNode*>(node->object)) {
                structType = m_scopeCtx->getType(idNode->name);
            }
        }
    }

    StructType* st = cast<StructType>(structType);
    std::string structName = st->getName().str();
    unsigned fieldIndex = m_structInfos[structName].fields[node->member].first;

    return m_builder->CreateStructGEP(st, structPtr, fieldIndex, "field_ptr");
}

Value* CodeGen::visitIndexExpr(IndexExprNode* node) {
    Value* arrayPtr = getAddressOf(node->array);
    Value* index = getValueOf(node->index);

    // For opaque pointers, we need the array/element type
    Type* arrayType = nullptr;
    if (auto* allocaInst = dyn_cast<AllocaInst>(arrayPtr)) {
        // Use getAllocatedType() for allocas (LLVM migration guide)
        arrayType = allocaInst->getAllocatedType();
    } else if (auto* idNode = dynamic_cast<IdentifierExprNode*>(node->array)) {
        // For arguments or other cases, use stored type
        arrayType = m_scopeCtx->getType(idNode->name);
    } else {
        llvm_unreachable("Array indexing on complex expressions requires type tracking");
    }

    // For arrays allocated with allocaInst [N x type], use indices [0, index]
    // The first 0 gets to the array start, the second is the actual index
    std::vector<Value*> indices = {m_builder->getInt32(0), index};

    // For opaque pointers, CreateGEP needs: (Type, ptr, indices)
    return m_builder->CreateGEP(arrayType, arrayPtr, indices, "elem_ptr");
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
    return m_namedTypes[node->identifier];
}
