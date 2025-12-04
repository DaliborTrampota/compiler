#include "fromJSON.h"

#undef NDEBUG
#include <cassert>
#include <fstream>
#include <iostream>


#include "ProgramNode.h"
#include "includeNodes.h"

namespace {

    void unknownNode(const std::string& nodeType) {
        std::cout << nodeType << "\n";
        assert(false);
    }

    template <typename NodeT>
    NodeT* parseNode(const nlohmann::json& json) {
        static_assert(false, "parseNode is not implemented for this node type");
        return nullptr;
    }

    // Forward declarations of template specializations
    template <>
    TypeNode* parseNode<TypeNode>(const nlohmann::json& json);

    template <>
    ExpressionNode* parseNode<ExpressionNode>(const nlohmann::json& json);

    template <>
    StatementNode* parseNode<StatementNode>(const nlohmann::json& json);

    template <>
    DeclarationNode* parseNode<DeclarationNode>(const nlohmann::json& json);

    template <typename ParseT, typename NodeT>
    std::vector<NodeT*> parseNodes(const nlohmann::json& json) {
        std::vector<NodeT*> nodes;
        for (const auto& node : json) {
            nodes.push_back(static_cast<NodeT*>(parseNode<ParseT>(node)));
        }
        return nodes;
    }

    template <>
    TypeNode* parseNode<TypeNode>(const nlohmann::json& json) {
        std::string nodeType = json["nodeType"].get<std::string>();

        if (nodeType == "PrimitiveType") {
            return new PrimitiveTypeNode(PrimitiveTypeNode::kindFromString(json["kind"]));
        } else if (nodeType == "NamedType") {
            return new NamedTypeNode(json["identifier"].get<std::string>());
        } else if (nodeType == "PointerType") {
            return new PointerTypeNode(parseNode<TypeNode>(json["baseType"]));
        }
        unknownNode(nodeType);
    }

    template <>
    ExpressionNode* parseNode<ExpressionNode>(const nlohmann::json& json) {
        std::string nodeType = json["nodeType"].get<std::string>();

        if (nodeType == "Literal") {
            std::string kind = json["kind"].get<std::string>();
            if (kind == "integer") {
                return new IntegerLiteralNode(json["value"].get<int>());
            } else if (kind == "double") {
                return new DoubleLiteralNode(json["value"].get<double>());
            } else if (kind == "character") {
                return new CharacterLiteralNode(json["value"].get<char>());
            } else if (kind == "string") {
                return new StringLiteralNode(json["value"].get<std::string>());
            }
        } else if (nodeType == "Identifier") {
            return new IdentifierExprNode(json["identifier"].get<std::string>());
        } else if (nodeType == "BinaryExpression") {
            return new BinaryExprNode(
                parseNode<ExpressionNode>(json["left"]),
                parseNode<ExpressionNode>(json["right"]),
                json["operator"].get<std::string>()
            );
        } else if (nodeType == "UnaryExpression") {
            return new UnaryExprNode(
                parseNode<ExpressionNode>(json["operand"]),
                json["operator"].get<std::string>(),
                json["prefix"].get<bool>()
            );
        } else if (nodeType == "CastExpression") {
            return new CastExprNode(
                parseNode<TypeNode>(json["targetType"]),
                parseNode<ExpressionNode>(json["expression"])
            );
        } else if (nodeType == "CallExpression") {
            return new CallExprNode(
                static_cast<IdentifierExprNode*>(parseNode<ExpressionNode>(json["callee"])),
                parseNodes<ExpressionNode, ExpressionNode>(json["arguments"])
            );
        } else if (nodeType == "MemberAccessExpression") {
            bool isArrow = json["kind"].get<std::string>() == "arrow";
            return new MemberExprNode(
                parseNode<ExpressionNode>(json["object"]),
                json["member"].get<std::string>(),
                isArrow
            );
        } else if (nodeType == "IndexExpression") {
            return new IndexExprNode(
                parseNode<ExpressionNode>(json["array"]), parseNode<ExpressionNode>(json["index"])
            );
        } else if (nodeType == "CommaExpression") {
            return new CommaExprNode(
                parseNodes<ExpressionNode, ExpressionNode>(json["expressions"])
            );
        }

        unknownNode(nodeType);
    }

    template <>
    StatementNode* parseNode<StatementNode>(const nlohmann::json& json) {
        std::string nodeType = json["nodeType"].get<std::string>();

        if (nodeType == "ReturnStatement") {
            return new ReturnStatementNode(parseNode<ExpressionNode>(json["expression"]));
        } else if (nodeType == "BlockStatement") {
            return new BlockStatementNode(
                parseNodes<StatementNode, StatementNode>(json["statements"])
            );
        } else if (nodeType == "IfStatement") {
            nlohmann::json elseBranch = json["elseBranch"];
            return new IfStatementNode(
                parseNode<ExpressionNode>(json["condition"]),
                parseNode<StatementNode>(json["thenBranch"]),
                elseBranch.is_null() ? BlockStatementNode::Empty()
                                     : parseNode<StatementNode>(elseBranch)
            );
        } else if (nodeType == "WhileStatement") {
            return new WhileStatementNode(
                parseNode<ExpressionNode>(json["condition"]),
                parseNode<StatementNode>(json["loopBody"])
            );
        } else if (nodeType == "ForStatement") {
            nlohmann::json init = json["init"];
            ForStatementNode::InitT initT{};
            if (init.is_null()) {
                //initT = nullptr;  // TODO?
            } else {
                std::string initNodeType = init["nodeType"].get<std::string>();
                if (initNodeType.find("Expression") != std::string::npos) {
                    initT = parseNode<ExpressionNode>(init);
                } else {
                    initT = parseNode<DeclarationNode>(init);
                }
            }

            nlohmann::json update = json["update"];
            nlohmann::json condition = json["condition"];
            return new ForStatementNode(
                initT,
                update.is_null() ? nullptr : parseNode<ExpressionNode>(update),
                condition.is_null() ? nullptr : parseNode<ExpressionNode>(condition),
                parseNode<StatementNode>(json["body"])
            );
        } else if (nodeType == "ExpressionStatement") {
            std::string exprNodeType = json["expression"]["nodeType"];
            if (exprNodeType.find("Expression") != std::string::npos) {
                return new ExpressionStatementNode(parseNode<ExpressionNode>(json["expression"]));
            } else if (exprNodeType.find("Declaration") != std::string::npos) {
                return new ExpressionStatementNode(parseNode<DeclarationNode>(json["expression"]));
            }
            assert(false);
        } else if (nodeType == "DoWhileStatement") {
            return new DoWhileStatementNode(
                parseNode<ExpressionNode>(json["condition"]),
                parseNode<StatementNode>(json["loopBody"])
            );
        }
        unknownNode(nodeType);
    }

    template <>
    DeclarationNode* parseNode<DeclarationNode>(const nlohmann::json& json) {
        std::string nodeType = json["nodeType"].get<std::string>();
        if (nodeType == "VariableDeclaration") {
            nlohmann::json arraySize = json["arraySize"];
            nlohmann::json initializer = json["initializer"];

            return new VariableDeclNode(
                json["identifier"].get<std::string>(),
                parseNode<TypeNode>(json["type"]),
                arraySize.is_null() ? nullptr : parseNode<ExpressionNode>(arraySize),
                initializer.is_null() ? nullptr : parseNode<ExpressionNode>(initializer)
            );
        } else if (nodeType == "Parameter") {
            return new ParameterDeclNode(
                json["identifier"].get<std::string>(), parseNode<TypeNode>(json["type"])
            );
        } else if (nodeType == "FunctionDeclaration") {
            return new FunctionDeclNode(
                json["identifier"].get<std::string>(),
                parseNode<TypeNode>(json["returnType"]),
                parseNodes<DeclarationNode, ParameterDeclNode>(json["parameters"]),
                BlockStatementNode::Empty()
            );
        } else if (nodeType == "FunctionDefinition") {
            return new FunctionDeclNode(
                json["identifier"].get<std::string>(),
                parseNode<TypeNode>(json["returnType"]),
                parseNodes<DeclarationNode, ParameterDeclNode>(json["parameters"]),
                static_cast<BlockStatementNode*>(parseNode<StatementNode>(json["body"]))
            );
        } else if (nodeType == "FunctionPointerDeclaration") {
            return new FunctionPtrDeclNode(
                json["identifier"].get<std::string>(),
                parseNode<TypeNode>(json["returnType"]),
                parseNodes<DeclarationNode, ParameterDeclNode>(json["parameters"])
            );
        } else if (nodeType == "StructDeclaration") {
            return new StructDeclNode(json["identifier"].get<std::string>(), {});
        } else if (nodeType == "StructDefinition") {
            return new StructDeclNode(
                json["identifier"].get<std::string>(),
                parseNodes<DeclarationNode, VariableDeclNode>(json["fields"])
            );
        }

        unknownNode(nodeType);
    }
}  // namespace

ProgramNode* fromJSON(const nlohmann::json& json) {
    ProgramNode* program =
        new ProgramNode(parseNodes<DeclarationNode, DeclarationNode>(json["declarations"]));

    return program;
}


nlohmann::json loadASTFromFile(const std::string& jsonFile) {
    std::ifstream R(jsonFile);
    if (!R.is_open()) {
        throw std::runtime_error("AST Json could not be opened\n");
    }

    // Example: Parse the input file as JSON
    try {
        nlohmann::json data = nlohmann::json::parse(R);
        std::cout << "Parsed JSON successfully!\n";
        std::cout << "Pretty print:\n" << data.dump(2) << std::endl;

        return data;
    } catch (nlohmann::json::parse_error& e) {
        throw std::runtime_error("Parse error: Not a JSON file\n");
    }
}

namespace {
    void printIndent(int indent) {
        for (int i = 0; i < indent; i++) {
            std::cout << "  ";
        }
    }

    void printType(const TypeNode* type, int indent = 0);
    void printExpression(const ExpressionNode* expr, int indent = 0);
    void printStatement(const StatementNode* stmt, int indent = 0);
    void printDeclaration(const DeclarationNode* decl, int indent = 0);

    void printType(const TypeNode* type, int indent) {
        if (!type) {
            std::cout << "<null type>";
            return;
        }

        if (auto* prim = dynamic_cast<const PrimitiveTypeNode*>(type)) {
            switch (prim->kind) {
                case PrimitiveTypeNode::Void: std::cout << "void"; break;
                case PrimitiveTypeNode::Char: std::cout << "char"; break;
                case PrimitiveTypeNode::Int: std::cout << "int"; break;
                case PrimitiveTypeNode::Double: std::cout << "double"; break;
            }
        } else if (auto* named = dynamic_cast<const NamedTypeNode*>(type)) {
            std::cout << named->identifier;
        } else if (auto* ptr = dynamic_cast<const PointerTypeNode*>(type)) {
            printType(ptr->baseType, indent);
            std::cout << "*";
        }
    }

    void printExpression(const ExpressionNode* expr, int indent) {
        if (!expr) {
            std::cout << "<null expr>";
            return;
        }

        if (auto* lit = dynamic_cast<const IntegerLiteralNode*>(expr)) {
            std::cout << lit->value;
        } else if (auto* lit = dynamic_cast<const DoubleLiteralNode*>(expr)) {
            std::cout << lit->value;
        } else if (auto* lit = dynamic_cast<const CharacterLiteralNode*>(expr)) {
            std::cout << "'" << lit->value << "'";
        } else if (auto* lit = dynamic_cast<const StringLiteralNode*>(expr)) {
            std::cout << "\"" << lit->value << "\"";
        } else if (auto* id = dynamic_cast<const IdentifierExprNode*>(expr)) {
            std::cout << id->name;
        } else if (auto* bin = dynamic_cast<const BinaryExprNode*>(expr)) {
            std::cout << "(";
            printExpression(bin->left, indent);
            std::cout << " " << bin->op << " ";
            printExpression(bin->right, indent);
            std::cout << ")";
        } else if (auto* un = dynamic_cast<const UnaryExprNode*>(expr)) {
            if (un->prefix) {
                std::cout << un->op;
                printExpression(un->operand, indent);
            } else {
                printExpression(un->operand, indent);
                std::cout << un->op;
            }
        } else if (auto* cast = dynamic_cast<const CastExprNode*>(expr)) {
            std::cout << "(";
            printType(cast->type);
            std::cout << ")";
            printExpression(cast->operand, indent);
        } else if (auto* call = dynamic_cast<const CallExprNode*>(expr)) {
            printExpression(call->callee, indent);
            std::cout << "(";
            for (size_t i = 0; i < call->arguments.size(); i++) {
                if (i > 0)
                    std::cout << ", ";
                printExpression(call->arguments[i], indent);
            }
            std::cout << ")";
        } else if (auto* member = dynamic_cast<const MemberExprNode*>(expr)) {
            printExpression(member->object, indent);
            std::cout << (member->arrowAccess ? "->" : ".") << member->member;
        } else if (auto* index = dynamic_cast<const IndexExprNode*>(expr)) {
            printExpression(index->array, indent);
            std::cout << "[";
            printExpression(index->index, indent);
            std::cout << "]";
        } else if (auto* comma = dynamic_cast<const CommaExprNode*>(expr)) {
            std::cout << "(";
            for (size_t i = 0; i < comma->expressions.size(); i++) {
                if (i > 0)
                    std::cout << ", ";
                printExpression(comma->expressions[i], indent);
            }
            std::cout << ")";
        } else {
            std::cout << "<unknown expression>";
        }
    }

    void printStatement(const StatementNode* stmt, int indent) {
        if (!stmt) {
            printIndent(indent);
            std::cout << "<null statement>\n";
            return;
        }

        if (auto* block = dynamic_cast<const BlockStatementNode*>(stmt)) {
            printIndent(indent);
            std::cout << "{\n";
            for (auto* s : block->body) {
                printStatement(s, indent + 1);
            }
            printIndent(indent);
            std::cout << "}\n";
        } else if (auto* ifStmt = dynamic_cast<const IfStatementNode*>(stmt)) {
            printIndent(indent);
            std::cout << "if (";
            printExpression(ifStmt->condition);
            std::cout << ")\n";
            printStatement(ifStmt->thenBody, indent + 1);
            if (ifStmt->elseBody) {
                // Check if it's an empty block, if so skip printing else
                bool isEmptyBlock = false;
                if (auto* elseBlock = dynamic_cast<const BlockStatementNode*>(ifStmt->elseBody)) {
                    if (elseBlock->body.empty()) {
                        isEmptyBlock = true;
                    }
                }
                if (!isEmptyBlock) {
                    // Check if it's an "else if" - format it nicely on same line
                    if (auto* elseIf = dynamic_cast<const IfStatementNode*>(ifStmt->elseBody)) {
                        printIndent(indent);
                        std::cout << "else ";
                        // Don't add newline, recursively print the if statement without indent
                        std::cout << "if (";
                        printExpression(elseIf->condition);
                        std::cout << ")\n";
                        printStatement(elseIf->thenBody, indent + 1);
                        // Recursively handle any further else/else-if
                        if (elseIf->elseBody) {
                            // Re-use the same logic by treating this as a new if statement's else
                            const IfStatementNode* current = elseIf;
                            while (current->elseBody) {
                                bool skipEmpty = false;
                                if (auto* eb = dynamic_cast<const BlockStatementNode*>(
                                        current->elseBody
                                    )) {
                                    if (eb->body.empty()) {
                                        skipEmpty = true;
                                        break;
                                    }
                                }
                                if (skipEmpty)
                                    break;

                                if (auto* nextElseIf =
                                        dynamic_cast<const IfStatementNode*>(current->elseBody)) {
                                    printIndent(indent);
                                    std::cout << "else if (";
                                    printExpression(nextElseIf->condition);
                                    std::cout << ")\n";
                                    printStatement(nextElseIf->thenBody, indent + 1);
                                    current = nextElseIf;
                                } else {
                                    printIndent(indent);
                                    std::cout << "else\n";
                                    printStatement(current->elseBody, indent + 1);
                                    break;
                                }
                            }
                        }
                    } else {
                        printIndent(indent);
                        std::cout << "else\n";
                        printStatement(ifStmt->elseBody, indent + 1);
                    }
                }
            }
        } else if (auto* whileStmt = dynamic_cast<const WhileStatementNode*>(stmt)) {
            printIndent(indent);
            std::cout << "while (";
            printExpression(whileStmt->condition);
            std::cout << ")\n";
            printStatement(whileStmt->loopBody, indent + 1);
        } else if (auto* doWhile = dynamic_cast<const DoWhileStatementNode*>(stmt)) {
            printIndent(indent);
            std::cout << "do\n";
            printStatement(doWhile->loopBody, indent + 1);
            printIndent(indent);
            std::cout << "while (";
            printExpression(doWhile->condition);
            std::cout << ");\n";
        } else if (auto* forStmt = dynamic_cast<const ForStatementNode*>(stmt)) {
            printIndent(indent);
            std::cout << "for (";
            if (auto* initExpr = std::get_if<ExpressionNode*>(&forStmt->init)) {
                if (*initExpr)
                    printExpression(*initExpr);
            } else if (auto* initDecl = std::get_if<DeclarationNode*>(&forStmt->init)) {
                if (*initDecl) {
                    if (auto* varDecl = dynamic_cast<const VariableDeclNode*>(*initDecl)) {
                        printType(varDecl->type);
                        std::cout << " " << varDecl->identifier;
                        if (varDecl->initializer) {
                            std::cout << " = ";
                            printExpression(varDecl->initializer);
                        }
                    }
                }
            }
            std::cout << "; ";
            if (forStmt->condition)
                printExpression(forStmt->condition);
            std::cout << "; ";
            if (forStmt->update)
                printExpression(forStmt->update);
            std::cout << ")\n";
            printStatement(forStmt->body, indent + 1);
        } else if (auto* exprStmt = dynamic_cast<const ExpressionStatementNode*>(stmt)) {
            printIndent(indent);
            if (std::holds_alternative<ExpressionNode*>(exprStmt->expression))
                printExpression(std::get<ExpressionNode*>(exprStmt->expression));
            else
                printDeclaration(std::get<DeclarationNode*>(exprStmt->expression));
            std::cout << ";\n";
        } else if (auto* retStmt = dynamic_cast<const ReturnStatementNode*>(stmt)) {
            printIndent(indent);
            std::cout << "return";
            if (retStmt->returnValue) {
                std::cout << " ";
                printExpression(retStmt->returnValue);
            }
            std::cout << ";\n";
        } else {
            printIndent(indent);
            std::cout << "<unknown statement>\n";
        }
    }

    void printDeclaration(const DeclarationNode* decl, int indent) {
        if (!decl) {
            printIndent(indent);
            std::cout << "<null declaration>\n";
            return;
        }

        if (auto* varDecl = dynamic_cast<const VariableDeclNode*>(decl)) {
            printIndent(indent);
            printType(varDecl->type);
            std::cout << " " << varDecl->identifier;
            if (varDecl->arraySize) {
                std::cout << "[";
                printExpression(varDecl->arraySize);
                std::cout << "]";
            }
            if (varDecl->initializer) {
                std::cout << " = ";
                printExpression(varDecl->initializer);
            }
            std::cout << ";\n";
        } else if (auto* funcDecl = dynamic_cast<const FunctionDeclNode*>(decl)) {
            printIndent(indent);
            printType(funcDecl->returnType);
            std::cout << " " << funcDecl->identifier << "(";
            for (size_t i = 0; i < funcDecl->parameters.size(); i++) {
                if (i > 0)
                    std::cout << ", ";
                auto* param = funcDecl->parameters[i];
                printType(param->type);
                std::cout << " " << param->identifier;
            }
            std::cout << ")";
            if (funcDecl->body && !funcDecl->body->body.empty()) {
                std::cout << "\n";
                printStatement(funcDecl->body, indent);
            } else {
                std::cout << ";\n";
            }
        } else if (auto* funcPtr = dynamic_cast<const FunctionPtrDeclNode*>(decl)) {
            printIndent(indent);
            printType(funcPtr->returnType);
            std::cout << " (*" << funcPtr->identifier << ")(";
            for (size_t i = 0; i < funcPtr->parameters.size(); i++) {
                if (i > 0)
                    std::cout << ", ";
                auto* param = funcPtr->parameters[i];
                printType(param->type);
                std::cout << " " << param->identifier;
            }
            std::cout << ");\n";
        } else if (auto* structDecl = dynamic_cast<const StructDeclNode*>(decl)) {
            printIndent(indent);
            std::cout << "struct " << structDecl->identifier;
            if (structDecl->fields.empty()) {
                std::cout << ";\n";
            } else {
                std::cout << " {\n";
                for (auto* field : structDecl->fields) {
                    printDeclaration(field, indent + 1);
                }
                printIndent(indent);
                std::cout << "};\n";
            }
        } else {
            printIndent(indent);
            std::cout << "<unknown declaration>\n";
        }
    }
}  // namespace

void printAST(const ProgramNode* ast) {
    if (!ast) {
        std::cout << "Empty AST (null pointer)\n";
        return;
    }

    std::cout << "\n=== AST Dump ===\n\n";
    for (auto* decl : ast->declarations) {
        printDeclaration(decl, 0);
    }
    std::cout << "\n=== End of AST ===\n\n";
}