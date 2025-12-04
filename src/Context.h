#pragma once

#include <llvm/IR/Instructions.h>
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
  private:
    std::unordered_map<std::string, llvm::Value*> namedValues;
};