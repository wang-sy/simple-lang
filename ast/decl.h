#pragma

#include "ast/ast.h"
#include "ast/expr.h"
#include "ast/field.h"
#include "ast/stmt.h"

namespace ast{
// DeclNode All declaration nodes implement the Decl interface.
class DeclNode: public Node {
public:
    ~DeclNode() override = default;

    NodeType Type() const override {return NodeType::Decl;};
};

// BadDeclNode is bad decl.
class BadDeclNode: public DeclNode {
public:
    BadDeclNode() = default;
    ~BadDeclNode() override = default;
    NodeType Type() const override {return NodeType::BadDecl;};
};

// FuncDeclNode represents a function declaration.
class FuncDeclNode: public DeclNode {
public:
    FuncDeclNode() = default;
    ~FuncDeclNode() override = default;
    FuncDeclNode(
            const shared_ptr<ExprNode>& type,
            const shared_ptr<IdentNode>& name,
            const shared_ptr<FieldListNode>& params,
            shared_ptr<StmtNode> body) :
            type_(type), name_(name), params_(params), body_(std::move(body)) {};
    NodeType Type() const override {return NodeType::FuncDecl;};
public:
    shared_ptr<ExprNode> type_{};
    shared_ptr<IdentNode> name_{};
    shared_ptr<FieldListNode> params_{};
    shared_ptr<StmtNode> body_{};
};

// SingleVarDeclNode decl a single var.
class SingleVarDeclNode: public DeclNode {
public:
    SingleVarDeclNode() = default;
    ~SingleVarDeclNode() override = default;
    SingleVarDeclNode(bool is_const, const shared_ptr<ExprNode>& type, const shared_ptr<IdentNode>& name, const shared_ptr<ExprNode>& val) :
            is_const_(is_const), type_(type), name_(name), val_(val) {}
    NodeType Type() const override {return NodeType::SingleVarDecl;};
public:
    bool is_const_{};
    shared_ptr<ExprNode> type_{};
    shared_ptr<IdentNode> name_{};
    shared_ptr<ExprNode> val_{};
};

// VarDeclNode represents a variable declaration.
class VarDeclNode: public DeclNode {
public:
    VarDeclNode() = default;
    ~VarDeclNode() override = default;
    explicit VarDeclNode(const vector<shared_ptr<SingleVarDeclNode>>& decls) :decls_(decls) {};
    NodeType Type() const override {return NodeType::VarDecl;};
public:
    vector<shared_ptr<SingleVarDeclNode>> decls_;
};
}