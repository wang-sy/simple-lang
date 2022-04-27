#pragma once

#include "ast/define.h"

namespace ast{

// A FieldNode represents a Field declaration list in a struct type,
// a method list in an interface type, or a parameter/result declaration
// in a signature.
// Field.Names is nil for unnamed parameters (parameter lists which only contain types)
// and embedded struct fields. In the latter case, the field name is the type name.
// Field.Names contains a single name "type" for elements of interface type lists.
// Types belonging to the same type list share the same "type" identifier which also
// records the position of that keyword.
class FieldNode: public Node {
public:
    FieldNode() = default;
    ~FieldNode() override = default;
    FieldNode(const shared_ptr<IdentNode>& name, const shared_ptr<ExprNode>& type) :
            name_(name), type_(type) {};
    NodeType Type() const override {return NodeType::Field;};
public:
    shared_ptr<IdentNode> name_;
    shared_ptr<ExprNode> type_;
};


// FieldListNode represents a list of Fields, enclosed by parentheses or braces.
class FieldListNode: public Node {
public:
    FieldListNode() = default;
    ~FieldListNode() override = default;
    explicit FieldListNode(const vector<shared_ptr<FieldNode>>& fields) :fields_(fields) {};
    NodeType Type() const override {return NodeType::FieldList;};
public:
    vector<shared_ptr<FieldNode>> fields_;
};
}