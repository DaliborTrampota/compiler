#pragma once

#include <llvm/IR/Value.h>
#include <memory>
#include <string>
#include <unordered_map>


struct Context {
    enum Type {
        Global = 1,
        Function = 2,
        Scope = 3,
    };

    Context* parent = nullptr;
    Type type;
    std::unordered_map<std::string, llvm::AllocaInst*> namedValues;

    static std::unique_ptr<Context> create(std::unique_ptr<Context> parent, Type type = Function) {
        return std::make_unique<Context>(std::move(parent), type);
    }

    std::unique_ptr<Context> destroy() { return std::move(parent); }

    llvm::AllocaInst* get(const std::string& name) {
        if (namedValues.count(name)) {
            return namedValues[name];
        }
        if (parent) {
            return parent->get(name);
        }
        return nullptr;
    }
};