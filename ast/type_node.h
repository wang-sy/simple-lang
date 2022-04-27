#pragma once

#include "ast/ast.h"

#include <memory>

namespace ast {
// TypeNode define type nodes.
// type nodes decl type about: 'int', 'char', 'string'.
// type nodes can also decl array.
class TypeNode : public Node {
public:
    TypeNode() = default;
    ~TypeNode() override = default;
    NodeType Type() const override { return NodeType::Type }
};

class CharTypeNode : public TypeNode {
public:
    CharTypeNode() = default;
    ~CharTypeNode() override = default;
    NodeType Type() const override { return NodeType::CharType; };
};

class IntTypeNode : public TypeNode {
public:
    IntTypeNode() = default;
    ~IntTypeNode() override = default;
    NodeType Type() const override { return NodeType::IntType; };
};

class ArrayTypeNode : public TypeNode {
public:
    IntTypeNode() = default;
    ~IntTypeNode() override = default;
    explicit TypeNode(const shared_ptr<TypeNode>& item) :item_(item) {};
    NodeType Type() const override { return NodeType::IntType; };
public:
    shared_ptr<TypeNode> item_;
};

}