# Simplified CodeGen with Type Checker

## Philosophy

**If type checker validates everything, CodeGen should just generate code.**

No need for defensive error checking - that's the type checker's job!

## Example: Member Access

### Without Type Checker (defensive):
```cpp
Value* CodeGen::visitMemberExpr(MemberExprNode* node) {
    Value* structPtr;
    
    if (node->arrowAccess) {
        structPtr = getValueOf(node->object);
    } else {
        structPtr = getAddressOf(node->object);
    }
    
    // Lots of defensive checks...
    Type* ptrType = structPtr->getType();
    if (!ptrType->isPointerTy()) {
        return LogError("Expected pointer type");
    }
    
    Type* structTypeRaw = ptrType->getPointerElementType();
    StructType* structType = dyn_cast<StructType>(structTypeRaw);
    if (!structType) {
        return LogError("Member access on non-struct");
    }
    
    std::string structName = structType->getName().str();
    auto it = m_structInfos.find(structName);
    if (it == m_structInfos.end()) {
        return LogError("Unknown struct");
    }
    
    auto fieldIt = info.fields.find(node->member);
    if (fieldIt == info.fields.end()) {
        return LogError("Unknown field");
    }
    
    unsigned fieldIndex = fieldIt->second.first;
    return m_builder->CreateStructGEP(structType, structPtr, fieldIndex, "member_ptr");
}
```

### With Type Checker (clean):
```cpp
Value* CodeGen::visitMemberExpr(MemberExprNode* node) {
    // Type checker already validated everything!
    Value* structPtr = node->arrowAccess 
        ? getValueOf(node->object)
        : getAddressOf(node->object);
    
    // Get struct type - we KNOW it's valid
    Type* structTypeRaw = structPtr->getType()->getPointerElementType();
    StructType* structType = cast<StructType>(structTypeRaw);
    
    // Get field index - we KNOW it exists
    std::string structName = structType->getName().str();
    unsigned fieldIndex = m_structInfos[structName].fields[node->member].first;
    
    return m_builder->CreateStructGEP(structType, structPtr, fieldIndex, "member_ptr");
}
```

Much cleaner! 🎉

## When to Keep Checks

### Keep checks for:
1. **Internal assertions** (programmer errors in CodeGen itself)
   ```cpp
   assert(structType && "CodeGen bug: expected struct type");
   ```

2. **LLVM API requirements** (things LLVM will crash on)
   ```cpp
   if (!ptr) {
       llvm_unreachable("Invalid pointer in CodeGen");
   }
   ```

3. **Debug builds** (use `#ifdef DEBUG` or assertions)
   ```cpp
   #ifndef NDEBUG
   if (!m_structInfos.count(structName)) {
       llvm::errs() << "CodeGen bug: missing struct info\n";
   }
   #endif
   ```

### Remove checks for:
1. **Type errors** (type checker's job)
2. **Undefined variables** (type checker's job)
3. **Invalid operations** (type checker's job)
4. **Semantic errors** (type checker's job)

## Benefits

✅ **Simpler code** - less clutter
✅ **Faster compilation** - no redundant checks
✅ **Clearer separation** - each phase has one job
✅ **Better error messages** - type checker gives better context

## The Contract

**Type Checker Promise:**
> "I will only pass you valid, well-typed ASTs"

**CodeGen Promise:**
> "I will generate correct LLVM IR for valid ASTs"

**Result:** Clean separation of concerns!

