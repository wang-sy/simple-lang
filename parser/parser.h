#pragma once

#include "error.h"
#include "token/position.h"
#include "scanner/scanner.h"
#include "ast/ast.h"
#include <vector>

using namespace std;

// ParserError is called for parse errors.
class ParserError {
public:
    ParserError(token::Position pos, string msg) : pos(pos), message(msg) {};
    token::Position pos;
    string message;
};

// Parser parse the source code of a single source file and returns
// the corresponding ast file tree.
class Parser {
public:
    Parser(
        const shared_ptr<token::File> &file,
        const string &src,
        const shared_ptr<ErrorHandler> &err,
        const shared_ptr<ec::ErrorReminder>& errors
    );

    // Report all parse errors.
    void ReportErrors();

    // Parse the source code and return the corresponding ast file tree.
    shared_ptr<ast::FileNode> Parse();
private:
    /**
     * Next advance to the next token.
     */
    void Next();

    /**
     * Error reports that the current token is unexpected.
     */
    void Error(const token::Position& pos, ec::Type error_type, const string& msg);

    void Expect(token::Token tok);

    // ParserDecl is called for parse decl.
    // e.g. 'int a', 'int a = 1', 'int a, b, c', 'int main() { ... }';
    shared_ptr<ast::DeclNode> ParseDecl();
    
    // ParseFuncDecl is called for parse function decl.
    // e.g. 'int main() { ... }';
    shared_ptr<ast::DeclNode> ParseFuncDecl(
        const token::Position& decl_pos,
        token::Token decl_type,
        const token::Position& name_pos,
        const string& name
    );

    // ParseVarDecl is called for parse variable decl.
    // In this function, we start from token 'const' or 'int / char'.
    // e.g. 'int a', 'int a = 1', 'int a, b, c';
    shared_ptr<ast::DeclNode> ParseVarDecl();

    // ParseVarDecl is called for parse variable decl.
    // In this function, const, decl_type and name are all parsed.
    // e.g. 'int a', 'int a = 1', 'int a, b, c';
    shared_ptr<ast::DeclNode> ParseVarDecl(
        const token::Position& decl_pos,
        bool is_const,
        token::Token decl_type,
        const token::Position& name_pos,
        const string& name
    );

    // ParseSingleVarDecl Get Single Var Decl.
    // Var may be 'int', 'char', 'array' type.
    shared_ptr<ast::DeclNode> ParseSingleVarDecl(
        const token::Position& decl_pos,
        int is_const,
        token::Token decl_type,
        const token::Position& name_pos,
        const string& name
    );

    // ParseCompositeLit is called for parse composite literal.
    // @param decl_type: INTTK or CHARTK.
    // e.g. '{ 1, 2, 3 }', '{{1,2,3}, {4,5,6}}';
    shared_ptr<ast::ExprNode> ParseCompositeLit(token::Token decl_type);

    // ParseBlockStmt is called for parse statement list.
    shared_ptr<ast::StmtNode> ParseBlockStmt();

    // ParseFieldList is called for parse field list.
    // e.g. (int a, char b)
    shared_ptr<ast::FieldListNode> ParseFieldList();

    // ParseSimpleStmt is called for parse simple statement.
    shared_ptr<ast::StmtNode> ParseSimpleStmt();

    // ParseStmt is called for parse statement.
    shared_ptr<ast::StmtNode> ParseStmt();
    shared_ptr<ast::StmtNode> ParseIfStmt();
    shared_ptr<ast::StmtNode> ParseWhileStmt();
    shared_ptr<ast::StmtNode> ParseForStmt();
    shared_ptr<ast::StmtNode> ParseScanStmt();
    shared_ptr<ast::StmtNode> ParsePrintfStmt();
    shared_ptr<ast::StmtNode> ParseReturnStmt();
    shared_ptr<ast::StmtNode> ParseSwitchStmt();
    shared_ptr<ast::StmtNode> ParseCaseStmt();

    shared_ptr<ast::ExprNode> ParseExpr();
    shared_ptr<ast::ExprNode> ParseBinaryExpr(int prec);
    shared_ptr<ast::ExprNode> ParseUnaryExpr();
    shared_ptr<ast::ExprNode> ParsePrimaryExpr();
    shared_ptr<ast::ExprNode> ParseOperand();

    // ParseCallExpr is called for parse function call expression.
    // When Calling this function, tok_ should be '(' of function call.
    // After Calling this function, tok_ will be ')' of function call.
    // e.g.
    //   start calling (arg1, arg2, arg3)  :=> tok_ is '('
    //   end calling (arg1, arg2, arg3)    :=> tok_ is ')'
    shared_ptr<ast::ExprNode> ParseCallExpr(const shared_ptr<ast::ExprNode>& func_name);

    /**
     * @brief ParseIndexExpr is called for parse index expression.
     * When Calling this function, tok_ should be '['.
     * After Calling this function, tok_ will be next token of ']'.
     * e.g. 
     *  - x[1] + 2 => after call token is : '+'
     *  - x[1][3] + 2 => after call token is : '+'
     * 
     * @param array_name expect IdentNode for array name.
     * @return shared_ptr<ast::ExprNode> IndexExprNode for success, BadExprNode for fail.
     */
    shared_ptr<ast::ExprNode> ParseIndexExpr(const shared_ptr<ast::ExprNode>& array_name);

    // Datas.

    // Next token.
    token::Token tok_;
    string lit_;
    token::Position pos_;
    
    shared_ptr<Scanner> scanner_;
    shared_ptr<token::File> file_;
    shared_ptr<ec::ErrorReminder> errors_;
};
