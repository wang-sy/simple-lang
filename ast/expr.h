#pragma once

#include "ast/define.h"

namespace ast {
// ExprNode All expression nodes implement the Expr interface.
class ExprNode : public Node {
public:
    ~ExprNode() override = default;

    NodeType Type() const override { return NodeType::Expr; };
};

// PreDecal for FieldNode.
// IdentNode represents an identifier.
class IdentNode: public ExprNode {
public:
    IdentNode() = default;
    ~IdentNode() override = default;
    explicit IdentNode(const string& name): name_(name) {}

    NodeType Type() const override {return NodeType::Ident;};
public:
    string name_;
};


// BadExprNode represents an expression containing syntax errors.
class BadExprNode: public ExprNode {
public:
    BadExprNode() = default;
    ~BadExprNode() override = default;
    NodeType Type() const override {return NodeType::BadExpr;};
};


// BasicLitNode represents a literal of basic type.
class BasicLitNode: public ExprNode {
public:
    BasicLitNode() = default;
    ~BasicLitNode() override = default;
    BasicLitNode(token::Token token_type, const string& val):tok_(token_type), val_(val) {}
    NodeType Type() const override {return NodeType::BasicLit;};
public:
    string val_;
    token::Token tok_;
};

// ParenExprNode represents a parenthesized expression.
class ParenExprNode: public ExprNode {
public:
    ParenExprNode() = default;
    ~ParenExprNode() override = default;
    explicit ParenExprNode(const shared_ptr<ExprNode>& expr) :expr_(expr) {};
    NodeType Type() const override {return NodeType::ParenExpr;};
public:
    shared_ptr<ExprNode> expr_;
};

// IndexExprNode represents an index expression.
class IndexExprNode: public ExprNode {
public:
    IndexExprNode() = default;
    ~IndexExprNode() override = default;
    IndexExprNode(const shared_ptr<ExprNode>& x, const shared_ptr<ExprNode>& index) :x_(x), index_(index) {};
    NodeType Type() const override {return NodeType::IndexExpr;};
public:
    shared_ptr<ExprNode> x_, index_;
};

// CallExprNode represents a function call expression.
class CallExprNode: public ExprNode {
public:
    CallExprNode() = default;
    ~CallExprNode() override= default;
    CallExprNode(const shared_ptr<ExprNode>& fun, const vector<shared_ptr<ExprNode>>& args) :fun_(fun), args_(args) {};
    NodeType Type() const override {return NodeType::CallExpr;};
public:
    shared_ptr<ExprNode> fun_;
    vector<shared_ptr<ExprNode>> args_;
};

// UnaryExprNode represents a unary expression.
class UnaryExprNode: public ExprNode {
public:
    UnaryExprNode() = default;
    ~UnaryExprNode() override = default;
    UnaryExprNode(token::Token op_tok, const shared_ptr<ExprNode>& x) :op_tok_(op_tok), x_(x) {};
    NodeType Type() const override {return NodeType::UnaryExpr;};
public:
    token::Token op_tok_;
    shared_ptr<ExprNode> x_;
};

// BianryExprNode represents a binary expression.
class BinaryExprNode: public ExprNode {
public:
    BinaryExprNode() = default;
    ~BinaryExprNode() override = default;
    BinaryExprNode(token::Token op_tok, const shared_ptr<ExprNode>& x, const shared_ptr<ExprNode>& y) :
            op_tok_(op_tok), x_(x), y_(y) {};
    NodeType Type() const override {return NodeType::BinaryExpr;};
public:
    token::Token op_tok_;
    shared_ptr<ExprNode> x_, y_;
};

}