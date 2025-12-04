#pragma once

#include <llvm/IR/Instructions.h>
#include <llvm/IR/Value.h>
#include <memory>
#include <string>
#include <unordered_map>

// Forward declaration
struct TypeNode;

struct Context {
    enum Type {
        Global = 1,
        Function = 2,
        Scope = 3,
    };

    std::unique_ptr<Context> parent = nullptr;
    Type type;


    Context(std::unique_ptr<Context> parent, Type type = Function)
        : parent(std::move(parent)),
          type(type) {}

    static std::unique_ptr<Context> create(std::unique_ptr<Context> parent, Type type = Function) {
        return std::make_unique<Context>(std::move(parent), type);
    }

    std::unique_ptr<Context> destroy() { return std::move(parent); }

    llvm::Value* get(const std::string& name) {
        if (namedValues.count(name)) {
            return namedValues[name];
        }
        if (parent) {
            return parent->get(name);
        }
        return nullptr;
    }

    void set(const std::string& name, llvm::Value* value) { namedValues[name] = value; }

    // Get the type of a variable (for opaque pointers)
    llvm::Type* getType(const std::string& name) {
        if (namedTypes.count(name)) {
            return namedTypes[name];
        }
        if (parent) {
            return parent->getType(name);
        }
        return nullptr;
    }

    void setType(const std::string& name, llvm::Type* type) { namedTypes[name] = type; }

    // For opaque pointers: also track AST TypeNode to preserve full type info
    TypeNode* getTypeNode(const std::string& name) {
        if (astTypes.count(name)) {
            return astTypes[name];
        }
        if (parent) {
            return parent->getTypeNode(name);
        }
        return nullptr;
    }

    void setTypeNode(const std::string& name, TypeNode* typeNode) { astTypes[name] = typeNode; }


  private:
    std::unordered_map<std::string, llvm::Value*> namedValues;
    std::unordered_map<std::string, llvm::Type*> namedTypes;
    std::unordered_map<std::string, TypeNode*> astTypes;  // Track AST types for opaque pointers
};