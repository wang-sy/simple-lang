#include <memory>
#include <iostream>
#include "parser/parser.h"
#include "ast/define.h"
#include "token/token.h"


Parser::Parser(const shared_ptr<token::File> &file, const string &src, const shared_ptr<ErrorHandler> &err) {
    scanner_ = make_shared<Scanner>(file, src, err);
    file_ = file;
    Next();
}

// Parse the source code and return the corresponding ast file tree.
ast::FileNode Parser::Parse() {
    vector<shared_ptr<ast::DeclNode>> decls;
    while (tok_ != token::END_OF_FILE) {
        decls.push_back(ParseDecl());
    }

    ast::FileNode ast_file_node;
    ast_file_node.decl_.insert(ast_file_node.decl_.begin(), decls.begin(), decls.end());

    return ast_file_node;
}

/**
 * Error reports that the current token is unexpected.
 */
void Parser::Error(int pos, const string& msg) {
    token::Position position = file_->GetPositionByOffset(pos);
    errors_.emplace_back(position, msg);
}

int Parser::Expect(token::Token tok) {
    int pos = pos_;
    if (tok_ != tok) {
        Error(pos, "expect " + token::GetTokenName(tok));
    }

    Next();
    return pos;
}

// Next advance to the next token.
void Parser::Next() {
    scanner_->Scan(&pos_, &tok_, &lit_);
}

// ParserDecl is called for parse decl.
// e.g. 'int a', 'int a = 1', 'int a, b, c', 'int main() { ... }';
shared_ptr<ast::DeclNode> Parser::ParseDecl() {
    int decl_pos = pos_;
    bool is_const = false;
    if (tok_ == token::Token::CONSTTK) {
        is_const = true;
        Next();
    }


    token::Token decl_type = tok_;
    Next();
    if (decl_type != token::Token::INTTK && decl_type != token::Token::CHARTK && decl_type != token::Token::VOIDTK) {
        Error(pos_, "for begin of a declear, expect int, char, or void");
        return make_shared<ast::BadDeclNode>();
    }

    int name_pos = pos_;
    string name = lit_;
    Expect(token::Token::IDENFR);

    if (tok_ == token::Token::LPARENT) {
        if (is_const) {
            Error(pos_, "const function result type not supported");
            return make_shared<ast::BadDeclNode>();
        }
        return ParseFuncDecl(decl_pos, decl_type, name_pos, name);
    }

    return ParseVarDecl(decl_pos, is_const, decl_type, name_pos, name);
}

// ParseFuncDecl is called for parse function decl.
// e.g. 'int main() { ... }';
shared_ptr<ast::DeclNode> Parser::ParseFuncDecl(int decl_pos, token::Token decl_type, int name_pos, const string& name) {
    auto func_decl_node = make_shared<ast::FuncDeclNode>();
    func_decl_node->type_ = make_shared<ast::IdentNode>(token::GetTokenName(decl_type));
    func_decl_node->name_ = make_shared<ast::IdentNode>(name);


    // parse param_list.
    auto param_list = make_shared<ast::FieldListNode>();
    func_decl_node->params_ = param_list;

    Expect(token::Token::LPARENT);
    if (tok_ != token::Token::RPARENT) {
        while(true) {
            auto param = make_shared<ast::FieldNode>();

            // Get input type.
            if (tok_ != token::Token::INTTK && tok_ != token::Token::CHARTK) {
                Error(pos_, "for paramlist, expect <int/char> indetifier");
                return make_shared<ast::BadDeclNode>(decl_pos, pos_);
            }
            param->type_ = make_shared<ast::IdentNode>(pos_, token::GetTokenName(tok_));

            // Get input name.
            Next();
            if (tok_ != token::Token::IDENFR) {
                Error(pos_, "for paramlist, expect <int/char> indetifier");
                return make_shared<ast::BadDeclNode>(decl_pos, pos_);
            }
            param->name_ = make_shared<ast::IdentNode>(pos_, lit_);

            param_list->fields_.push_back(param);

            Next();
            // Check spliter, ',' or ')'.
            if (tok_ != token::Token::COMMA && tok_ != token::Token::RPARENT) {
                Error(pos_, "for paramlist spliter, expect ',' or ')'");
                return make_shared<ast::BadDeclNode>(decl_pos, pos_);
            }

            if (tok_ == token::Token::RPARENT) {
                break;
            }
            
            // for spliter ',', do next.
            Next();
        }
    }

    // GetBody.
    Expect(token::Token::RPARENT);

    // parse stmt one by one.
    auto body_stmt_node = ParseStmtList();
    if (body_stmt_node->Type() != ast::NodeType::BlockStmt) {
        Error(pos_, "for function body, expect <block>");
        return make_shared<ast::BadDeclNode>(decl_pos, pos_);
    }
    func_decl_node->body_ = body_stmt_node;

    return func_decl_node;
}

shared_ptr<ast::StmtNode> Parser::ParseStmtList() {
    auto stmt_list = make_shared<ast::BlockStmtNode>();

    Expect(token::Token::LBRACE);

    while(tok_ != token::Token::RBRACE) {
        stmt_list->stmts_.push_back(ParseStmt());
    }

    Expect(token::Token::RBRACE);

    return stmt_list;
}

shared_ptr<ast::StmtNode> Parser::ParseStmt() {
    switch (tok_) {
        case token::Token::CONSTTK:
        case token::Token::INTTK:
        case token::Token::CHARTK:
            return make_shared<ast::DeclStmtNode>(ParseVarDecl());
        default:
            return make_shared<ast::BadStmtNode>(pos_, pos_);
    }
}

// ParseVarDecl is called for parse variable decl.
// In this function, we start from token 'const' or 'int / char'.
// e.g. 'int a', 'int a = 1', 'int a, b, c';
shared_ptr<ast::DeclNode> Parser::ParseVarDecl() {
    int decl_pos = pos_;
    bool is_const = false;
    if (tok_ == token::Token::CONSTTK) {
        is_const = true;
        Next();
    }

    token::Token decl_type = tok_;
    if (decl_type != token::Token::CHARTK && decl_type != token::Token::INTTK) {
        Error(pos_, "for begin of a declear, expect int or char");
        return make_shared<ast::BadDeclNode>(decl_pos, pos_);
    }

    Next();

    int name_pos = pos_;
    if(tok_ != token::Token::IDENFR) {
        Error(pos_, "for var decl, expect <int/char> indetifier");
        return make_shared<ast::BadDeclNode>(decl_pos, pos_);
    }
    
    return ParseVarDecl(decl_pos, is_const, decl_type, name_pos, lit_);
}

// ParseVarDecl is called for parse variable decl.
// In this function, const, decl_type and name are all parsed.
// e.g. 'int a', 'int a = 1', 'int a, b, c';
shared_ptr<ast::DeclNode> Parser::ParseVarDecl(int decl_pos, bool is_const, token::Token decl_type, int name_pos, const string& name) {
    auto var_decl_node = make_shared<ast::VarDeclNode>();

    // scan token, until get ','.
    while (tok_ != token::SEMICN) {

    }

    return make_shared<ast::BadDeclNode>();
}

// ParseSingleVarDecl parse single var.
// Calling this function, current token is IDENFR.
shared_ptr<ast::DeclNode> Parser::ParseSingleVarDecl(int is_const, int decl_pos, token::Token decl_type, int name_pos, const string &name) {
    auto single_decl_node = make_shared<ast::SingleVarDeclNode>();
    single_decl_node->type_ = make_shared<ast::IdentNode>(decl_pos, token::GetTokenName(decl_type));
    single_decl_node->name_ = make_shared<ast::IdentNode>(name_pos, name);
    single_decl_node->is_const_ = is_const;

    Expect(token::Token::IDENFR);


    // if token is '[', means var is a array.
    bool is_array = false;
    int first_dimension = - 1;
    int second_dimension = - 1;
    if (tok_ == token::Token::LBRACK) {
        Expect(token::Token::LBRACK);
        if (tok_ != token::Token::INTCON) {
            Error(pos_, "array define should be <int/char> ident[1][2];");
            return make_shared<ast::BadDeclNode>(decl_pos, pos_);
        }
        is_array = true;
        first_dimension = stoi(lit_);
        Next();
        Expect(token::Token::RBRACK);
    }

    // 第二个维度.
    if (tok_ == token::Token::LBRACK) {
        Expect(token::Token::LBRACK);
        if (tok_ != token::Token::INTCON) {
            Error(pos_, "array define should be <int/char> ident[1][2];");
            return make_shared<ast::BadDeclNode>(decl_pos, pos_);
        }
        second_dimension = stoi(lit_);
        Next();
        Expect(token::Token::RBRACK);
    }

    // update type.
    if (is_array) {

    }


    // for ';' or ',' return directly.
    if (tok_ == token::Token::SEMICN || tok_ == token::Token::COMMA) {
        return single_decl_node;
    }

    if (tok_ != token::Token::ASSIGN) {
        Error(pos_, "For SingleVarDecl, should be <int/char> ident <, / ; / = xxx>");
        return make_shared<ast::BadDeclNode>(decl_pos, pos_);
    }

    // token is assign.
    Expect(token::Token::ASSIGN);

    return shared_ptr<ast::DeclNode>();
}
