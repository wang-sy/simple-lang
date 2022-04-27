#pragma once

#include "ast/decl.h"
#include "ast/expr.h"
#include "ast/stmt.h"

#include <memory>

using namespace std;

namespace ast{
// StmtNode All statement nodes implement the Stmt interface.
class StmtNode: public Node {
public:
    ~StmtNode() override = default;

    NodeType Type() const override {return NodeType::Stmt;};
};

// BadStmtNode represents a statement containing syntax errors for which a correct
// statement node cannot be created.
class BadStmtNode: public StmtNode {
public:
    BadStmtNode() = default;
    ~BadStmtNode() override = default;
    NodeType Type() const override {return NodeType::BadStmt;};
};

// DeclStmtNode represents a declaration statement.
class DeclStmtNode: public StmtNode {
public:
    DeclStmtNode() = default;
    ~DeclStmtNode() override = default;
    explicit DeclStmtNode(const shared_ptr<DeclNode>& decl) :decl_(decl) {};
    NodeType Type() const override {return NodeType::DeclStmt;};
public:
    shared_ptr<DeclNode> decl_{};
};

// EmptyStmtNode represents an empty statement.
class EmptyStmtNode: public StmtNode {
public:
    EmptyStmtNode() = default;
    ~EmptyStmtNode() override = default;
    NodeType Type() const override {return NodeType::EmptyStmt;};
};

// ExprStmtNode represents an expression statement.
class ExprStmtNode: public StmtNode {
public:
    ExprStmtNode() = default;
    ~ExprStmtNode() override = default;
    explicit ExprStmtNode(const shared_ptr<ExprNode>& expr) :expr_(expr) {};
    NodeType Type() const override {return NodeType::ExprStmt;};
public:
    shared_ptr<ExprNode> expr_{};
};

// AssignStmtNode represents an assignment statement.
class AssignStmtNode: public StmtNode {
public:
    AssignStmtNode() = default;
    ~AssignStmtNode() override = default;
    explicit AssignStmtNode(const shared_ptr<ExprNode>& lhs, const shared_ptr<ExprNode>& rhs) :lhs_(lhs), rhs_(rhs) {};
    NodeType Type() const override {return NodeType::AssignStmt;};
public:
    shared_ptr<ExprNode> lhs_{}, rhs_{};
};

// ReturnStmtNode represents a return statement.
class ReturnStmtNode: public StmtNode {
public:
    ReturnStmtNode() = default;
    ~ReturnStmtNode() override = default;
    explicit ReturnStmtNode(const shared_ptr<ExprNode>& results) :results_(results) {};
    NodeType Type() const override {return NodeType::ReturnStmt;};
public:
    shared_ptr<ExprNode> results_{};
};

// BranchStmtNode represents a branch statement.
class BranchStmtNode: public StmtNode {
public:
    BranchStmtNode() = default;
    ~BranchStmtNode() override = default;
    explicit BranchStmtNode(token::Token tok) :tok_(tok) {};
    NodeType Type() const override {return NodeType::BranchStmt;};
public:
    token::Token tok_;
};

// BlockStmtNode represents a block statement.
class BlockStmtNode: public StmtNode {
public:
    BlockStmtNode() = default;
    ~BlockStmtNode() override = default;
    explicit BlockStmtNode(const vector<shared_ptr<StmtNode>>& stmts) :stmts_(stmts) {};
    NodeType Type() const override {return NodeType::BlockStmt;};
public:
    vector<shared_ptr<StmtNode>> stmts_;
};

// IfStmtNode represents an if statement.
class IfStmtNode: public StmtNode {
public:
    IfStmtNode() = default;
    ~IfStmtNode() override = default;
    IfStmtNode(const shared_ptr<ExprNode>& cond, const shared_ptr<BlockStmtNode>& body, const shared_ptr<StmtNode>& else_stmt) :
            cond_(cond), body_(body), else_(else_stmt) {};
    NodeType Type() const override {return NodeType::BlockStmt;};
public:
    shared_ptr<ExprNode> cond_{};
    shared_ptr<BlockStmtNode> body_;
    shared_ptr<StmtNode> else_;
};

// CaseStmtNode represents a case statement.
class CaseStmtNode: public StmtNode {
public:
    CaseStmtNode() = default;
    ~CaseStmtNode() override = default;
    CaseStmtNode(const shared_ptr<ExprNode>& cond, const vector<shared_ptr<StmtNode>>& body) :
            cond_(cond), body_(body) {};
    NodeType Type() const override {return NodeType::CaseStmt;};
public:
    shared_ptr<ExprNode> cond_{};
    vector<shared_ptr<StmtNode>> body_;
};

// SwitchStmtNode represents a switch statement.
class SwitchStmtNode: public StmtNode {
public:
    SwitchStmtNode() = default;
    ~SwitchStmtNode() override = default;
    SwitchStmtNode(const shared_ptr<ExprNode>& cond, const shared_ptr<BlockStmtNode>& body) :
            cond_(cond), body_(body) {};
    NodeType Type() const override {return NodeType::SwitchStmt;};
public:
    shared_ptr<ExprNode> cond_{};
    shared_ptr<BlockStmtNode> body_; // case only.
};

}