#pragma once

#include <string>
#include <memory>
#include <vector>
#include <map>
#include "token/token.h"

using namespace std;

namespace ast {


// There are 3 main classes of nodes: Expressions and type nodes,
// statement nodes, and declaration nodes. The node names usually
// match the corresponding spec production names to which they
// correspond. The node fields correspond to the individual parts
// of the respective productions.
//
// All nodes contain position information marking the beginning of
// the corresponding source text segment; it is accessible via the
// Pos accessor method. Nodes may contain additional position info
// for language constructs where comments may be found between parts
// of the construct (typically any larger, parenthesized subpart).
// That position information is needed to properly position comments
// when printing the construct.
enum NodeType {
    // BaseNodeTypes
    Expr, 
    Stmt,
    Decl,

    File,
    
    // Expression nodes.
    expr_beg,
    // A BadExpr node is a placeholder for an expression containing
	// syntax errors for which a correct expression node cannot be
	// created.
    BadExpr,
    // An Ident node represents an identifier.
    // e.g. x, y, z, ...
    Ident,
    // A BasicLit node represents a literal of basic type.
    // like INT, FLOAT, CHAR, or STRING.
    BasicLit,
    // A CompositeLit node represents a composite literal.
    // e.g. int a[3] = {1, 2, 3} or int b[1][2] = {{1, 2}, {3, 4}}
    CompositeLit,
    // A ParenExpr node represents a parenthesized expression.
    // e.g. (x), or (x + y).
    ParenExpr,
    // An IndexExpr node represents an expression followed by an index.
    // e.g. x[i]
    IndexExpr,
    // A CallExpr node represents an expression followed by an argument list.
    // e.g. f(1, 2, 3)
    CallExpr,
    // A UnaryExpr node represents a unary expression. 
    // e.g. '+x', '-x', '!x'
    UnaryExpr,
    // A BinaryExpr node represents a binary expression. 
    // e.g. a + b
    BinaryExpr,
    expr_end,

    // Statement nodes.
    stmt_beg,
    // A BadStmt node is a placeholder for statements containing
	// syntax errors for which no correct statement nodes can be
	// created.
    BadStmt,
    // A DeclStmt node represents a declaration in a statement list.
    // e.g. int x = 1;
    DeclStmt,
    // An EmptyStmt node represents an empty statement.
	// The "position" of the empty statement is the position
	// of the immediately following (explicit or implicit) semicolon.
    // e.g. ';'
    EmptyStmt,
    // An ExprStmt node represents a (stand-alone) expression
	// in a statement list.
	// e.g. 'x = y'
    ExprStmt,
    // An AssignStmt node represents an assignment or
	// a short variable declaration.
    // e.g. 'x = y'
    AssignStmt,
    // A ReturnStmt node represents a return statement.
    // e.g. 'return x'
    ReturnStmt,
    // A BranchStmt node represents a break, continue, goto,
	// or fallthrough statement.
    // e.g. 'break', 'continue'
    BranchStmt,
    // A BlockStmt node represents a braced statement list.
    // e.g. '{ ... }'
    BlockStmt,
    // An IfStmt node represents an if statement.
    // e.g. 'if (x) y;'
    IfStmt,
    // A CaseClause represents a case of an expression or type switch statement.
    // e.g. 'case x:'
    CaseClause,
    // A SwitchStmt node represents an expression switch statement.
    // e.g. 'switch x { ... }'
    SwitchStmt,
    stmt_end,

    decl_beg,
    // Declaration nodes.
    // A BadDecl node is a placeholder for a declaration containing
	// syntax errors for which a correct declaration node cannot be
	// created.
    BadDecl,
    // A VarDecl node (var declaration node) represents an constant
	// , variable declaration. A valid Lparen position
	// (Lparen.IsValid()) indicates a parenthesized declaration.
	//
    // e.g. 'const x = 1;'
    VarDecl,
    // A FuncDecl node represents a function declaration.
    // e.g. 'int foobar(x int)  {...}'
    FuncDecl,
    decl_end,

    // Type nodes.
    type_beg,
    // An ArrayType node represents an array or slice type.
    // e.g. int a[10];
    ArrayType,
    type_end,

    // Other nodes.
    // A FieldNode represents a Field declaration list in a struct type,
    // a method list in an interface type, or a parameter/result declaration
    // in a signature.
    Field,
    // A FieldList represents a list of Fields, enclosed by parentheses or braces.
    FieldList,
};

// All node types implement the Node interface.
class Node {
public:
    // Get position of first character belonging to the node.
    virtual int Pos() const = 0;

    // Get position of first character immediately after the node.
    virtual int End() const = 0;

    // Get type of node.
    virtual NodeType Type() const = 0;

    virtual ~Node() = default;
};

// ExprNode All expression nodes implement the Expr interface.
class ExprNode: public Node {
public:
    virtual ~ExprNode() override = default;

    virtual NodeType Type() const override {return NodeType::Expr;};
};

// StmtNode All statement nodes implement the Stmt interface.
class StmtNode: public Node {
public:
    virtual ~StmtNode() override = default;

    virtual NodeType Type() const override {return NodeType::Stmt;};
};

// DeclNode All declaration nodes implement the Decl interface.
class DeclNode: public Node {
public:
    virtual ~DeclNode() override = default;

    virtual NodeType Type() const override {return NodeType::Decl;};
};

// ============================================================
// ================ Field nodes ===============================
// ============================================================

// PreDecal for FieldNode.
// IdentNode represents an identifier.
class IdentNode: public ExprNode {
public:
    IdentNode() = default;
    IdentNode(int pos, const string& name): pos_(pos), name_(name) {}
    NodeType Type() const override {return NodeType::Ident;};
    int Pos() const override {return pos_;}
    int End() const override {return pos_ + name_.size();}
    string Name() const {return name_;}
public:
    string name_;
    int pos_;
};

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
    FieldNode(const shared_ptr<IdentNode>& name, const shared_ptr<ExprNode>& type) :
        name_(name), type_(type) {};
    NodeType Type() const override {return NodeType::Field;};
    int Pos() const override {return type_->Pos();}
    int End() const override {return name_->Pos();}
public:
    shared_ptr<IdentNode> name_;
    shared_ptr<ExprNode> type_;
};

// FieldListNode represents a list of Fields, enclosed by parentheses or braces.
class FieldListNode: public Node {
public:
    FieldListNode() = default;
    FieldListNode(int opening, int closing, const vector<shared_ptr<FieldNode>>& fields) :
        fields_(fields), opening_(opening), closing_(closing) {};
    NodeType Type() const override {return NodeType::FieldList;};
    int Pos() const override {
        if (opening_ != token::kNoPos) {
            return opening_;
        }
        if (!fields_.empty()) {
             return fields_.front()->Pos();
        }
        return token::kNoPos;
    }
    int End() const override {
        if (closing_ != token::kNoPos) {
            return closing_;
        }
        if (!fields_.empty()) {
             return fields_.back()->Pos();
        }
        return token::kNoPos;
    }
    int NumFields() const {return fields_.size();}
public:
    // position of opening parenthesis/brace, if any
    int opening_, closing_; 
    vector<shared_ptr<FieldNode>> fields_;
};

// ============================================================
// ================ Statement nodes ===========================
// ============================================================

// BadStmtNode represents a statement containing syntax errors for which a correct
// statement node cannot be created.
class BadStmtNode: public StmtNode {
public:
    BadStmtNode() = default;
    BadStmtNode(int from, int to) :
        from_(from), to_(to) {};
    NodeType Type() const override {return NodeType::BadStmt;};
    int Pos() const override {return from_;}
    int End() const override {return to_;}
public:
    int from_, to_;
};

// DeclStmtNode represents a declaration statement.
class DeclStmtNode: public StmtNode {
public:
    DeclStmtNode() = default;
    DeclStmtNode(const shared_ptr<DeclNode>& decl) :
        decl_(decl) {};
    NodeType Type() const override {return NodeType::DeclStmt;};
    int Pos() const override {return decl_->Pos();}
    int End() const override {return decl_->End();}
public:
    shared_ptr<DeclNode> decl_;
};

// EmptyStmtNode represents an empty statement.
class EmptyStmtNode: public StmtNode {
public:
    EmptyStmtNode() = default;
    EmptyStmtNode(int pos) :
        pos_(pos) {};
    NodeType Type() const override {return NodeType::EmptyStmt;};
    int Pos() const override {return pos_;}
    int End() const override {return pos_;}
public:
    int pos_;
};

// ExprStmtNode represents an expression statement.
class ExprStmtNode: public StmtNode {
public:
    ExprStmtNode() = default;
    ExprStmtNode(const shared_ptr<ExprNode>& expr) :
        expr_(expr) {};
    NodeType Type() const override {return NodeType::ExprStmt;};
    int Pos() const override {return expr_->Pos();}
    int End() const override {return expr_->End();}
public:
    shared_ptr<ExprNode> expr_;
};

// AssignStmtNode represents an assignment statement.
class AssignStmtNode: public StmtNode {
public:
    AssignStmtNode() = default;
    AssignStmtNode(const shared_ptr<ExprNode>& lhs, const shared_ptr<ExprNode>& rhs) :
        lhs_(lhs), rhs_(rhs) {};
    NodeType Type() const override {return NodeType::AssignStmt;};
    int Pos() const override {return lhs_->Pos();}
    int End() const override {return rhs_->End();}
public:
    shared_ptr<ExprNode> lhs_, rhs_;
};

// ReturnStmtNode represents a return statement.
class ReturnStmtNode: public StmtNode {
public:
    ReturnStmtNode() = default;
    ReturnStmtNode(int return_pos, const shared_ptr<ExprNode>& results) :
        return_pos_(return_pos), results_(results) {};
    NodeType Type() const override {return NodeType::ReturnStmt;};
    int Pos() const override {return return_pos_;}
    int End() const override {return results_->End();}
public:
    int return_pos_;
    shared_ptr<ExprNode> results_;
};

// BranchStmtNode represents a branch statement.    
class BranchStmtNode: public StmtNode {
public:
    BranchStmtNode() = default;
    BranchStmtNode(int pos, token::Token tok) :
        tok_pos_(pos), tok_(tok) {};
    NodeType Type() const override {return NodeType::BranchStmt;};
    int Pos() const override {return tok_pos_;}
    int End() const override {return tok_pos_;} 
public:
    int tok_pos_;
    token::Token tok_;
};

// BlockStmtNode represents a block statement.
class BlockStmtNode: public StmtNode {
public:
    BlockStmtNode() = default;
    BlockStmtNode(int lbrace, int rbrace, const vector<shared_ptr<StmtNode>>& stmts) :
        stmts_(stmts), lbrace_(lbrace), rbrace_(rbrace) {};
    NodeType Type() const override {return NodeType::BlockStmt;};
    int Pos() const override {return lbrace_;}
    int End() const override {return rbrace_;}
public:
    int lbrace_, rbrace_;
    vector<shared_ptr<StmtNode>> stmts_;
};

// IfStmtNode represents an if statement.
class IfStmtNode: public StmtNode {
public:
    IfStmtNode() = default;
    IfStmtNode(int if_pos_, const shared_ptr<ExprNode>& cond, const shared_ptr<BlockStmtNode>& body, const shared_ptr<StmtNode>& else_stmt) :
        if_pos_(if_pos_), cond_(cond), body_(body), else_(else_stmt) {};
    NodeType Type() const override {return NodeType::BlockStmt;};
    int Pos() const override {return if_pos_;}
    int End() const override {
        if (else_ != nullptr) {
            return else_->End();
        }

        return body_->End();
    }
public:
    int if_pos_;
    shared_ptr<ExprNode> cond_;
    shared_ptr<BlockStmtNode> body_;
    shared_ptr<StmtNode> else_;
};

// CaseStmtNode represents a case statement.
class CaseStmtNode: public StmtNode {
public:
    CaseStmtNode() = default;
    CaseStmtNode(int case_pos, int colon_pos, const shared_ptr<ExprNode>& cond, const vector<shared_ptr<StmtNode>>& body) :
        case_pos_(case_pos), colon_pos_(colon_pos), cond_(cond), body_(body) {};
    NodeType Type() const override {return NodeType::BlockStmt;};
    int Pos() const override {return case_pos_;}
    int End() const override {
        if (!body_.empty()) {
            return body_.back()->End();
        }

        return colon_pos_;
    }
public:
    int case_pos_, colon_pos_;
    shared_ptr<ExprNode> cond_;
    vector<shared_ptr<StmtNode>> body_;
};

// SwitchStmtNode represents a switch statement.
class SwitchStmtNode: public StmtNode {
public:
    SwitchStmtNode() = default;
    SwitchStmtNode(int switch_pos, const shared_ptr<ExprNode>& cond, const shared_ptr<BlockStmtNode>& body) :
        switch_pos_(switch_pos), cond_(cond), body_(body) {};
    NodeType Type() const override {return NodeType::BlockStmt;};
    int Pos() const override {return switch_pos_;}
    int End() const override {return body_->End();}
public:
    int switch_pos_;
    shared_ptr<ExprNode> cond_;
    shared_ptr<BlockStmtNode> body_; // case only.
};


// ============================================================
// ================ Expression nodes ==========================
// ============================================================

// BadExprNode represents an expression containing syntax errors.
class BadExprNode: public ExprNode {
public:
    BadExprNode() = default;
    BadExprNode(int from, int to): from_(from), to_(to) {}
    NodeType Type() const override {return NodeType::BadExpr;};
    int Pos() const override {return from_;}
    int End() const override {return to_;}
public:
    int from_, to_;
};


// BasicLitNode represents a literal of basic type.
class BasicLitNode: public ExprNode {
public:
    BasicLitNode() = default;
    BasicLitNode(int pos, token::Token token_type, const string& val):
        pos_(pos), tok_(token_type), val_(val) {}
    NodeType Type() const override {return NodeType::BasicLit;};
    int Pos() const override {return pos_;}
    int End() const override {return pos_ + val_.size();}
    token::Token TokenType() const {return tok_;}
    string Value() const {return val_;}
public:
    int pos_;
    string val_;
    token::Token tok_;
};

// CompositeLitNode represents a composite literal.
class CompositeLitNode: public ExprNode {
public:
    CompositeLitNode() = default;
    CompositeLitNode(int lbrace, int rbrace, const shared_ptr<ExprNode>& type, const vector<shared_ptr<ExprNode>>& elts) :
        lbrace_(lbrace), rbrace_(rbrace), type_(type), elts_(elts) {};
    NodeType Type() const override {return NodeType::CompositeLit;};
    int Pos() const override {return type_->Pos();}
    int End() const override {
        if (!elts_.empty()) {
            return elts_.back()->End();
        }

        return token::kNoPos;
    }
public:
    shared_ptr<ExprNode> type_;
    int lbrace_, rbrace_;
    vector<shared_ptr<ExprNode>> elts_;
};

// ParenExprNode represents a parenthesized expression.
class ParenExprNode: public ExprNode {
public:
    ParenExprNode() = default;
    ParenExprNode(int lparen, int rparen, const shared_ptr<ExprNode>& expr) :
        lparen_(lparen), rparen_(rparen), expr_(expr) {};
    NodeType Type() const override {return NodeType::ParenExpr;};
    int Pos() const override {return lparen_;}
    int End() const override {return rparen_;}
public:
    int lparen_, rparen_;
    shared_ptr<ExprNode> expr_;
};

// IndexExprNode represents an index expression.
class IndexExprNode: public ExprNode {
public:
    IndexExprNode() = default;
    IndexExprNode(int lbrack, int rbrack, const shared_ptr<ExprNode>& x, const shared_ptr<ExprNode>& index) :
        lbrack_(lbrack), rbrack_(rbrack), x_(x), index_(index) {};
    NodeType Type() const override {return NodeType::IndexExpr;};
    int Pos() const override {return x_->Pos();}
    int End() const override {return rbrack_;}
public:
    shared_ptr<ExprNode> x_, index_;
    int lbrack_, rbrack_;
};

// CallExprNode represents a function call expression.
class CallExprNode: public ExprNode {
public:
    CallExprNode() = default;
    CallExprNode(int lparen, int rparen, const shared_ptr<ExprNode>& fun, const vector<shared_ptr<ExprNode>>& args) :
        lparen_(lparen), rparen_(rparen), fun_(fun), args_(args) {};
    NodeType Type() const override {return NodeType::CallExpr;};
    int Pos() const override {return fun_->Pos();}
    int End() const override {return rparen_;}
public:
    int lparen_, rparen_;
    shared_ptr<ExprNode> fun_;
    vector<shared_ptr<ExprNode>> args_;
};

// UnaryExprNode represents a unary expression.
class UnaryExprNode: public ExprNode {
public:
    UnaryExprNode() = default;
    UnaryExprNode(int op_pos, token::Token op_tok, const shared_ptr<ExprNode>& x) :
        op_pos_(op_pos), op_tok_(op_tok), x_(x) {};
    NodeType Type() const override {return NodeType::UnaryExpr;};
    int Pos() const override {return op_pos_;}
    int End() const override {return x_->End();}
public:
    int op_pos_;
    token::Token op_tok_;
    shared_ptr<ExprNode> x_;
};

// BianryExprNode represents a binary expression.
class BinaryExprNode: public ExprNode {
public:
    BinaryExprNode() = default;
    BinaryExprNode(int op_pos, token::Token op_tok, const shared_ptr<ExprNode>& x, const shared_ptr<ExprNode>& y) :
        op_pos_(op_pos), op_tok_(op_tok), x_(x), y_(y) {};
    NodeType Type() const override {return NodeType::BinaryExpr;};
    int Pos() const override {return op_pos_;}
    int End() const override {return y_->End();}
public:
    int op_pos_;
    token::Token op_tok_;
    shared_ptr<ExprNode> x_, y_;
};


// ============================================================
// ================ Declaration nodes =========================
// ============================================================

class BadDeclNode: public DeclNode {
public:
    BadDeclNode() = default;
    BadDeclNode(int pos, int end): pos_(pos), end_(end) {};
    NodeType Type() const override {return NodeType::BadDecl;};
    int Pos() const override {return pos_;}
    int End() const override {return end_;}
public:
    int pos_, end_;
};

// FuncDeclNode represents a function declaration.
class FuncDeclNode: public DeclNode {
public:
    FuncDeclNode() = default;
    FuncDeclNode(
        const shared_ptr<ExprNode>& type, 
        const shared_ptr<IdentNode>& name, 
        const shared_ptr<FieldListNode> params,
        shared_ptr<StmtNode> body) :
        type_(type), name_(name), params_(params), body_(body) {};
    NodeType Type() const override {return NodeType::FuncDecl;};
    int Pos() const override {return type_->Pos();}
    int End() const override {return body_->End();}
public:
    shared_ptr<ExprNode> type_;
    shared_ptr<IdentNode> name_;
    shared_ptr<FieldListNode> params_;
    shared_ptr<StmtNode> body_;
};

// VarDeclNode represents a variable declaration.
class VarDeclNode: public DeclNode {
public:
    VarDeclNode() = default;
    VarDeclNode(const shared_ptr<ExprNode>& type, const vector<shared_ptr<IdentNode>>& names, const vector<shared_ptr<ExprNode>>& vals) :
        type_(type), names_(names), vals_(vals) {};
    NodeType Type() const override {return NodeType::VarDecl;};
    int Pos() const override {return type_->Pos();}
    int End() const override {
        if (!vals_.empty()) {
            return vals_.back()->End();
        }

        return token::kNoPos;
    }
public:
    shared_ptr<ExprNode> type_;
    vector<shared_ptr<IdentNode>> names_;
    vector<shared_ptr<ExprNode>> vals_;
};

// FileNode represents a Go source file.
class FileNode {
public:
    shared_ptr<IdentNode> name_;
    vector<shared_ptr<DeclNode>> decl_;
    // map<string, Node> scope_;
};

}// namespace ast