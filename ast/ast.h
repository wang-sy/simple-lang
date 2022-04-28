#pragma once

#include <string>
#include <memory>
#include <vector>
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
    Type,
    Literal,

    File,

    type_beg,
    BadType,
    CharType,
    IntType,
    StringType,
    ArrayType,
    VoidType,
    type_end,

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
    // e.g. {1, 2, 3}, {{1,2,3}, {2,3,4}};
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
    CaseStmt,
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
    // e.g. 'const int x = 1, b = 2;'
    VarDecl,
    // A SingleVarDecl node is declaration for a var.
    // e.g. const int a = 10;
    SingleVarDecl,
    // A FuncDecl node represents a function declaration.
    // e.g. 'int foobar(x int)  {...}'
    FuncDecl,
    decl_end,

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
    // Get type of node.
    virtual NodeType Type() const = 0;

    virtual string ToString() const = 0;

    virtual ~Node() = default;
};

// TypeNode define type nodes.
// type nodes decl type about: 'int', 'char', 'string'.
// type nodes can also decl array.
class TypeNode: public Node {
public:
    TypeNode() = default;
    ~TypeNode() override = default;
    virtual NodeType Type() const override { return NodeType::Type; }

    virtual string ToString() const override {return "<TypeNode/>";}
};

// DeclNode All declaration nodes implement the Decl interface.
class DeclNode: public Node {
public:
    virtual ~DeclNode() override = default;

    virtual NodeType Type() const override {return NodeType::Decl;};
    
    virtual string ToString() const override {return "<DeclNode/>";}
};

// ExprNode All expression nodes implement the Expr interface.
class ExprNode : public Node {
public:
    ~ExprNode() override = default;

    NodeType Type() const override { return NodeType::Expr; };

    virtual string ToString() const override {return "<ExprNode/>";}
};

// StmtNode All statement nodes implement the Stmt interface.
class StmtNode: public Node {
public:
    ~StmtNode() override = default;

    NodeType Type() const override {return NodeType::Stmt;};

    virtual string ToString() const override {return "<StmtNode/>";}
};

// ====================================================================
// ======================Type Node=====================================
// ====================================================================
class BadTypeNode: public TypeNode {
public:
    BadTypeNode() = default;
    ~BadTypeNode() override = default;
    NodeType Type() const override { return NodeType::BadType; };
    string ToString() const override {return "<BadTypeNode/>";}
};

class VoidTypeNode : public TypeNode {
public:
    VoidTypeNode() = default;
    ~VoidTypeNode() override = default;
    NodeType Type() const override { return NodeType::VoidType; };
    string ToString() const override {return "<VoidTypeNode/>";}
};

class CharTypeNode : public TypeNode {
public:
    CharTypeNode() = default;
    ~CharTypeNode() override = default;
    NodeType Type() const override { return NodeType::CharType; };
    string ToString() const override {return "<CharTypeNode/>";}
};

class IntTypeNode : public TypeNode {
public:
    IntTypeNode() = default;
    ~IntTypeNode() override = default;
    NodeType Type() const override { return NodeType::IntType; };
    string ToString() const override {return "<IntTypeNode/>";}
};

class ArrayTypeNode : public TypeNode {
public:
    ArrayTypeNode() = default;
    ~ArrayTypeNode() override = default;
    ArrayTypeNode(const shared_ptr<TypeNode>& item, int size) :item_(item), size_(size) {};
    NodeType Type() const override { return NodeType::ArrayType; };
    string ToString() const override {
        string ret = "<ArrayTypeNode>";
        ret += "<size>" + to_string(size_) + "</size>";
        ret += "<item>" + (item_ == nullptr ? "" :item_->ToString()) + "</item>";
        ret += "</ArrayTypeNode>";
        return ret;
    }
public:
    int size_ = 0;
    shared_ptr<TypeNode> item_;
};

// ====================================================================
// ======================Expr Node=====================================
// ====================================================================

// PreDecal for FieldNode.
// IdentNode represents an identifier.
class IdentNode: public ExprNode {
public:
    IdentNode() = default;
    ~IdentNode() override = default;
    explicit IdentNode(const string& name): name_(name) {}
    NodeType Type() const override {return NodeType::Ident;};
    string ToString() const override {
        string ret = "<IdentNode>";
        ret += "<name>" + name_ + "</name>";
        ret += "</IdentNode>";
        return ret;
    }
public:
    string name_;
};


// BadExprNode represents an expression containing syntax errors.
class BadExprNode: public ExprNode {
public:
    BadExprNode() = default;
    ~BadExprNode() override = default;
    NodeType Type() const override {return NodeType::BadExpr;};
    string ToString() const override {return "<BadExprNode/>";}
};


// BasicLitNode represents a literal of basic type.
class BasicLitNode: public ExprNode {
public:
    BasicLitNode() = default;
    ~BasicLitNode() override = default;
    BasicLitNode(token::Token token_type, const string& val):tok_(token_type), val_(val) {}
    NodeType Type() const override {return NodeType::BasicLit;};
    string ToString() const override {
        string ret = "<BasicLitNode>";
        ret +=  "<token>" + token::GetTokenName(tok_) + "</token>";
        ret += "<value>" + val_  + "</value>";
        ret += "</BasicLitNode>";
        return ret;
    }
public:
    string val_;
    token::Token tok_;
};

class CompositeLitNode: public ExprNode { 
public:
    CompositeLitNode() = default;
    ~CompositeLitNode() override = default;
    NodeType Type() const override {return NodeType::CompositeLit;};
    string ToString() const override {
        string ret = "<CompositeLitNode>";
        for (auto item: items_) {
            ret += "<item>" + (item == nullptr ? "" : item->ToString()) + "</item>";
        }
        ret += "</CompositeLitNode>";
        return ret;
    }
public:
    vector<shared_ptr<ExprNode>> items_;
};

// ParenExprNode represents a parenthesized expression.
class ParenExprNode: public ExprNode {
public:
    ParenExprNode() = default;
    ~ParenExprNode() override = default;
    explicit ParenExprNode(const shared_ptr<ExprNode>& expr) :expr_(expr) {};
    NodeType Type() const override {return NodeType::ParenExpr;};
    string ToString() const override {
        string ret = "<ParenExprNode>";
        ret += "<expr>" + (expr_ == nullptr ? "" : expr_->ToString()) + "</expr>";
        ret += "</ParenExprNode>";
        return ret;
    }
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
    string ToString() const override {
        string ret = "<IndexExprNode>";
        ret += "<x>" + (x_ == nullptr ? "" : x_->ToString()) + "</x>";
        ret += "<index>" + (index_ == nullptr ? "" : index_->ToString()) + "</index>";
        ret += "</IndexExprNode>";
        return ret;
    }
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
    string ToString() const override {
        string ret = "<CallExprNode>";
        ret += "<fun>" + (fun_ == nullptr ? "" : fun_->ToString()) + "</fun>";
        for (auto item: args_) {
            ret += "<arg>" + (item == nullptr ? "" : item->ToString()) + "</arg>";
        }
        ret += "</CallExprNode>";
        return ret;
    }
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
    string ToString() const override {
        string ret = "<UnaryExprNode>";
        ret += "<op>" + token::GetTokenName(op_tok_) + "</op>";
        ret += "<x>" + (x_ == nullptr ? "" : x_->ToString()) + "</x>";
        return ret;
    }
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
    string ToString() const override {
        string ret = "<BinaryExprNode>";
        ret += "<op>" + token::GetTokenName(op_tok_) + "</op>";
        ret += "<x>" + (x_ == nullptr ? "" : x_->ToString()) + "</x>";
        ret += "<y>" + (y_ == nullptr ? "" : y_->ToString()) + "</y>";
        return ret;
    }
public:
    token::Token op_tok_;
    shared_ptr<ExprNode> x_, y_;
};

// ====================================================================
// ======================Field Node=====================================
// ====================================================================

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
    FieldNode(const shared_ptr<IdentNode>& name, const shared_ptr<TypeNode>& type) :
            name_(name), type_(type) {};
    NodeType Type() const override {return NodeType::Field;};
    string ToString() const override {
        string ret = "<FieldNode>";
        ret += "<name>" + (name_ == nullptr ? "" : name_->ToString()) + "</name>";
        ret += "<type>" + (type_ == nullptr ? "" : type_->ToString()) + "</type>";
        ret += "</FieldNode>";
        return ret;
    }
public:
    shared_ptr<IdentNode> name_;
    shared_ptr<TypeNode> type_;
};


// FieldListNode represents a list of Fields, enclosed by parentheses or braces.
class FieldListNode: public Node {
public:
    FieldListNode() = default;
    ~FieldListNode() override = default;
    explicit FieldListNode(const vector<shared_ptr<FieldNode>>& fields) :fields_(fields) {};
    NodeType Type() const override {return NodeType::FieldList;};
    string ToString() const override {
        string ret = "<FieldListNode>";
        for (auto field: fields_) {
            ret += "<field>" + (field == nullptr ? "" : field->ToString()) + "</field>";
        }
        ret += "</FieldListNode>";
        return ret;
    }
public:
    vector<shared_ptr<FieldNode>> fields_;
};

// ====================================================================
// ======================Decl Node=====================================
// ====================================================================


// BadDeclNode is bad decl.
class BadDeclNode: public DeclNode {
public:
    BadDeclNode() = default;
    ~BadDeclNode() override = default;
    NodeType Type() const override {return NodeType::BadDecl;};
    string ToString() const override {
        return "<BadDeclNode/>";
    }
};

// FuncDeclNode represents a function declaration.
class FuncDeclNode: public DeclNode {
public:
    FuncDeclNode() = default;
    ~FuncDeclNode() override = default;
    FuncDeclNode(
            const shared_ptr<TypeNode>& type,
            const shared_ptr<IdentNode>& name,
            const shared_ptr<FieldListNode>& params,
            shared_ptr<StmtNode> body) :
            type_(type), name_(name), params_(params), body_(std::move(body)) {};
    NodeType Type() const override {return NodeType::FuncDecl;};
    string ToString() const override {
        string ret = "<FuncDeclNode>";
        ret += "<type>" + (type_ == nullptr ? "" : type_->ToString()) + "</type>";
        ret += "<name>" + (name_ == nullptr ? "" : name_->ToString()) + "</name>";
        ret += "<params>" + (params_ == nullptr ? "" : params_->ToString()) + "</params>";
        ret += "<body>" + (body_ == nullptr ? "" : body_->ToString()) + "</body>";
        return ret;
    }
public:
    shared_ptr<TypeNode> type_{};
    shared_ptr<IdentNode> name_{};
    shared_ptr<FieldListNode> params_{};
    shared_ptr<StmtNode> body_{};
};

// SingleVarDeclNode decl a single var.
class SingleVarDeclNode: public DeclNode {
public:
    SingleVarDeclNode() = default;
    ~SingleVarDeclNode() override = default;
    SingleVarDeclNode(bool is_const, const shared_ptr<TypeNode>& type, const shared_ptr<IdentNode>& name, const shared_ptr<ExprNode>& val) :
            is_const_(is_const), type_(type), name_(name), val_(val) {}
    NodeType Type() const override {return NodeType::SingleVarDecl;};
    string ToString() const override {
        string ret = "<SingleVarDeclNode>";
        ret += "<is_const>" + (is_const_ ? string("true") : string("false")) + "</is_const>";
        ret += "<type>" + (type_ == nullptr ? "" : type_->ToString()) + "</type>";
        ret += "<name>" + (name_ == nullptr ? "" : name_->ToString()) + "</name>";
        ret += "<val>" + (val_ == nullptr ? "" : val_->ToString()) + "</val>";
        return ret;
    }
public:
    bool is_const_{};
    shared_ptr<TypeNode> type_{};
    shared_ptr<IdentNode> name_{};
    shared_ptr<ExprNode> val_{};
};

// VarDeclNode represents a variable declaration.
class VarDeclNode: public DeclNode {
public:
    VarDeclNode() = default;
    ~VarDeclNode() override = default;
    explicit VarDeclNode(const vector<shared_ptr<DeclNode>>& decls) :decls_(decls) {};
    NodeType Type() const override {return NodeType::VarDecl;};
    string ToString() const override {
        string ret = "<VarDeclNode>";
        for (auto decl: decls_) {
            ret += "<decl>" + (decl == nullptr ? "" : decl->ToString()) + "</decl>";
        }
        return ret;
    }
public:
    vector<shared_ptr<DeclNode>> decls_;
};

// ====================================================================
// ======================Stmt Node=====================================
// ====================================================================

// BadStmtNode represents a statement containing syntax errors for which a correct
// statement node cannot be created.
class BadStmtNode: public StmtNode {
public:
    BadStmtNode() = default;
    ~BadStmtNode() override = default;
    NodeType Type() const override {return NodeType::BadStmt;};
    string ToString() const override {
        return "<BadStmtNode/>";
    }
};

// DeclStmtNode represents a declaration statement.
class DeclStmtNode: public StmtNode {
public:
    DeclStmtNode() = default;
    ~DeclStmtNode() override = default;
    explicit DeclStmtNode(const shared_ptr<DeclNode>& decl) :decl_(decl) {};
    NodeType Type() const override {return NodeType::DeclStmt;};
    string ToString() const override {
        string ret = "<DeclStmtNode>";
        ret += "<decl>" + (decl_ == nullptr ? "" : decl_->ToString()) + "</decl>";
        ret += "</DeclStmtNode>";
        return ret;
    }
public:
    shared_ptr<DeclNode> decl_{};
};

// EmptyStmtNode represents an empty statement.
class EmptyStmtNode: public StmtNode {
public:
    EmptyStmtNode() = default;
    ~EmptyStmtNode() override = default;
    NodeType Type() const override {return NodeType::EmptyStmt;};
    string ToString() const override {
        return "<EmptyStmtNode/>";
    }
};

// ExprStmtNode represents an expression statement.
class ExprStmtNode: public StmtNode {
public:
    ExprStmtNode() = default;
    ~ExprStmtNode() override = default;
    explicit ExprStmtNode(const shared_ptr<ExprNode>& expr) :expr_(expr) {};
    NodeType Type() const override {return NodeType::ExprStmt;};
    string ToString() const override {
        string ret = "<ExprStmtNode>";
        ret += "<expr>" + (expr_ == nullptr ? "" : expr_->ToString()) + "</expr>";
        ret += "</ExprStmtNode>";
        return ret;
    }
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
    string ToString() const override {
        string ret = "<AssignStmtNode>";
        ret += "<lhs>" + (lhs_ == nullptr ? "" : lhs_->ToString()) + "</lhs>";
        ret += "<rhs>" + (rhs_ == nullptr ? "" : rhs_->ToString()) + "</rhs>";
        ret += "</AssignStmtNode>";
        return ret;
    }
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
    string ToString() const override {
        string ret = "<ReturnStmtNode>";
        ret += "<results>" + (results_ == nullptr ? "" : results_->ToString()) + "</results>";
        ret += "</ReturnStmtNode>";
        return ret;
    }
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
    string ToString() const override {
        string ret = "<BranchStmtNode>";
        ret += "<tok>" + token::GetTokenName(tok_) + "</tok>";
        ret += "</BranchStmtNode>";
        return ret;
    }
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
    string ToString() const override {
        string ret = "<BlockStmtNode>";
        for (auto stmt: stmts_) {
            ret += "<stmt>" + (stmt == nullptr ? "" : stmt->ToString()) + "</stmt>";
        }
        ret += "</BlockStmtNode>";
        return ret;
    }
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
    string ToString() const override {
        string ret = "<IfStmtNode>";
        ret += "<cond>" + (cond_ == nullptr ? "" : cond_->ToString()) + "</cond>";
        ret += "<body>" + (body_ == nullptr ? "" : body_->ToString()) + "</body>";
        ret += "<else>" + (else_ == nullptr ? "" : else_->ToString()) + "</else>";
        ret += "</IfStmtNode>";
        return ret;
    }
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
    string ToString() const override {
        string ret = "<CaseStmtNode>";
        ret += "<cond>" + (cond_ == nullptr ? "" : cond_->ToString()) + "</cond>";
        for (auto stmt: body_) {
            ret += "<body>" + (stmt == nullptr ? "" : stmt->ToString()) + "</body>";
        }
        ret += "</CaseStmtNode>";
        return ret;
    }
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
    string ToString() const override {
        string ret = "<SwitchStmtNode>";
        ret += "<cond>" + (cond_ == nullptr ? "" : cond_->ToString()) + "</cond>";
        ret += "<body>" + (body_ == nullptr ? "" : body_->ToString()) + "</body>";
        ret += "</SwitchStmtNode>";
        return ret;
    }
public:
    shared_ptr<ExprNode> cond_{};
    shared_ptr<BlockStmtNode> body_; // case only.
};

// ====================================================================
// ======================File Node=====================================
// ====================================================================

// FileNode represents a Go source file.
class FileNode {
public:
    shared_ptr <IdentNode> name_{};
    vector <shared_ptr<DeclNode>> decl_{};
    string ToString() const {
        string ret = "<FileNode>";
        ret += "<name>" + (name_ == nullptr ? "" : name_->ToString()) + "</name>";
        for (auto decl: decl_) {
            ret += "<decl>" + (decl == nullptr ? "" : decl->ToString()) + "</decl>";
        }
        ret += "</FileNode>";
        return ret;
    }
    // map<string, Node> scope_;
};

}// namespace ast