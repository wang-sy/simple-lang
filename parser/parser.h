#pragma once

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
    Parser(const shared_ptr<token::File> &file, const string &src, const shared_ptr<ErrorHandler> &err);

    // Report all parse errors.
    void ReportErrors();

    // Parse the source code and return the corresponding ast file tree.
    ast::FileNode Parse();
private:
    /**
     * Next advance to the next token.
     */
    void Next();

    /**
     * Error reports that the current token is unexpected.
     */
    void Error(int pos, const string& msg);

    int Expect(token::Token tok);

    // ParserDecl is called for parse decl.
    // e.g. 'int a', 'int a = 1', 'int a, b, c', 'int main() { ... }';
    shared_ptr<ast::DeclNode> ParseDecl();
    
    // ParseFuncDecl is called for parse function decl.
    // e.g. 'int main() { ... }';
    shared_ptr<ast::DeclNode> ParseFuncDecl(int decl_pos, token::Token decl_type, int name_pos, const string& name);

    // ParseVarDecl is called for parse variable decl.
    // In this function, we start from token 'const' or 'int / char'.
    // e.g. 'int a', 'int a = 1', 'int a, b, c';
    shared_ptr<ast::DeclNode> ParseVarDecl();

    // ParseVarDecl is called for parse variable decl.
    // In this function, const, decl_type and name are all parsed.
    // e.g. 'int a', 'int a = 1', 'int a, b, c';
    shared_ptr<ast::DeclNode> ParseVarDecl(int decl_pos, bool is_const, token::Token decl_type, int name_pos, const string& name);

    // ParseSingleVarDecl Get Single Var Decl.
    // Var may be 'int', 'char', 'array' type.
    shared_ptr<ast::DeclNode> ParseSingleVarDecl(int is_const, int decl_pos, token::Token decl_type, int name_pos, const string& name);

    // ParseCompositeLit is called for parse composite literal.
    // @param decl_type: INTTK or CHARTK.
    // e.g. '{ 1, 2, 3 }', '{{1,2,3}, {4,5,6}}';
    shared_ptr<ast::ExprNode> ParseCompositeLit(token::Token decl_type);

    // ParseBlockStmt is called for parse statement list.
    shared_ptr<ast::StmtNode> ParseBlockStmt();

    // ParseFieldList is called for parse field list.
    // e.g. (int a, char b)
    shared_ptr<ast::FieldListNode> ParseFieldList();

    // ParseStmt is called for parse statement.
    shared_ptr<ast::StmtNode> ParseStmt();

    shared_ptr<ast::StmtNode> ParseExprStmt();

    // Datas.

    // Next token.
    token::Token tok_;
    string lit_;
    int pos_;
    
    shared_ptr<Scanner> scanner_;
    shared_ptr<token::File> file_;
    vector<ParserError> errors_;
};
