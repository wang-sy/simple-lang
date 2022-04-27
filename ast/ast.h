#pragma once

#include <string>
#include <memory>
#include <vector>
#include <map>
#include "ast/type_node.h"
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
    CharType,
    IntType,
    StringType,
    ArrayType,
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

    virtual ~Node() = default;
};

}// namespace ast