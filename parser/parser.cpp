#include <memory>
#include <iostream>
#include <stack>
#include "parser/parser.h"
#include "ast/ast.h"
#include "token/token.h"

// Helper function to parse ast.

// NewBasicTypeNode is called for return basic type node.
shared_ptr<ast::TypeNode> NewBasicTypeNode(token::Token tok) {
    if (tok == token::Token::CHARTK) {
        return make_shared<ast::CharTypeNode>();
    } else if (tok == token::Token::INTTK) {
        return make_shared<ast::IntTypeNode>();
    } else if (tok == token::Token::VOIDTK) {
        return make_shared<ast::VoidTypeNode>();
    } else {
        return make_shared<ast::BadTypeNode>();
    }
}

// NewArrayTypeNode is called for return array type node.
shared_ptr<ast::TypeNode> NewArrayTypeNode(token::Token tok, int first_dimension, int second_dimension) {
    if (tok != token::CHARTK && tok != token::INTTK) {
        return make_shared<ast::BadTypeNode>();
    }

    if (first_dimension == -1) {
        return make_shared<ast::BadTypeNode>();
    }

    auto array_type_node = make_shared<ast::ArrayTypeNode>();
    array_type_node->size_ = first_dimension;
    if (second_dimension == -1) {
        array_type_node->item_ = NewBasicTypeNode(tok);
    } else {
        array_type_node->item_ = make_shared<ast::ArrayTypeNode>(NewBasicTypeNode(tok), second_dimension);
    }

    return array_type_node;
}


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
        cout << "ERROR:" << "expect" << token::GetTokenName(tok) << endl;;
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
    if (tok_ != token::Token::MAINTK && tok_ != token::Token::IDENFR) {
        Error(pos_, "for decl, expect <int/char/void> name");
        return make_shared<ast::BadDeclNode>();
    }
    Next();

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
    func_decl_node->type_ = NewBasicTypeNode(decl_type);
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
                return make_shared<ast::BadDeclNode>();
            }
            param->type_ = NewBasicTypeNode(tok_);

            // Get input name.
            Next();
            if (tok_ != token::Token::IDENFR) {
                Error(pos_, "for paramlist, expect <int/char> indetifier");
                return make_shared<ast::BadDeclNode>();
            }
            param->name_ = make_shared<ast::IdentNode>(lit_);

            param_list->fields_.push_back(param);

            Next();
            // Check spliter, ',' or ')'.
            if (tok_ != token::Token::COMMA && tok_ != token::Token::RPARENT) {
                Error(pos_, "for paramlist spliter, expect ',' or ')'");
                return make_shared<ast::BadDeclNode>();
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
        return make_shared<ast::BadDeclNode>();
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
            return make_shared<ast::BadStmtNode>();
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
        return make_shared<ast::BadDeclNode>();
    }

    Next();

    int name_pos = pos_;
    if(tok_ != token::Token::IDENFR) {
        Error(pos_, "for var decl, expect <int/char> indetifier");
        return make_shared<ast::BadDeclNode>();
    }

    Expect(token::Token::IDENFR);
    
    return ParseVarDecl(decl_pos, is_const, decl_type, name_pos, lit_);
}

// ParseVarDecl is called for parse variable decl.
// In this function, const, decl_type and name are all parsed.
// e.g. 'int a', 'int a = 1', 'int a, b, c';
shared_ptr<ast::DeclNode> Parser::ParseVarDecl(int decl_pos, bool is_const, token::Token decl_type, int name_pos, const string& name) {
    int cur_name_pos = name_pos;
    string cur_name = name;

    // scan token, until get ','.
    auto var_decl_node = make_shared<ast::VarDeclNode>();
    do {
        // Parse current var, current tok_ is '=' or ',' or ';'.
        var_decl_node->decls_.push_back(ParseSingleVarDecl(decl_pos, is_const, decl_type, cur_name_pos, cur_name));

        if (tok_ == token::Token::SEMICN) {
            break;
        }

        if (tok_ != token::Token::COMMA) {
            Error(pos_, "for var decl step word, expect ',' or ';'");
            return make_shared<ast::BadDeclNode>();
        }

        // current tok_ is ',' :=> get next identfire's name.
        Expect(token::Token::COMMA);
        cout << "Run ASSIGN " << pos_ << "=> cur_name_pos" << " && " <<  lit_ << "=> cur_name" << endl;
        cur_name_pos = pos_;
        cur_name = lit_;
        cout << "AFTER ASSIGN " << cur_name_pos << "=> cur_name_pos" << " && " <<  cur_name << "=> cur_name" << endl;
        Expect(token::Token::IDENFR);
        cout << "AFTER Expect " << cur_name_pos << "=> cur_name_pos" << " && " <<  cur_name << "=> cur_name" << endl;
    } while (tok_ != token::SEMICN);

    Expect(token::Token::SEMICN);

    return var_decl_node;
}

// ParseSingleVarDecl parse single var.
// Calling this function, current token is IDENFR.
shared_ptr<ast::DeclNode> Parser::ParseSingleVarDecl(int is_const, int decl_pos, token::Token decl_type, int name_pos, const string name) {
    cout << "ParseSingleVar: get name => " << name << endl;
    auto single_decl_node = make_shared<ast::SingleVarDeclNode>();
    single_decl_node->type_ = NewBasicTypeNode(decl_type);
    single_decl_node->name_ = make_shared<ast::IdentNode>(name);
    single_decl_node->is_const_ = is_const;

    // if token is '[', means var is a array.
    bool is_array = false;
    int first_dimension = - 1;
    int second_dimension = - 1;
    if (tok_ == token::Token::LBRACK) {
        Expect(token::Token::LBRACK);
        if (tok_ != token::Token::INTCON) {
            Error(pos_, "array define should be <int/char> ident[1][2];");
            return make_shared<ast::BadDeclNode>();
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
            return make_shared<ast::BadDeclNode>();
        }
        second_dimension = stoi(lit_);
        Next();
        Expect(token::Token::RBRACK);
    }

    // update type.
    if (is_array) {
        single_decl_node->type_ = NewArrayTypeNode(decl_type, first_dimension, second_dimension);
    }


    // for ';' or ',' return directly.
    if (tok_ == token::Token::SEMICN || tok_ == token::Token::COMMA) {
        return single_decl_node;
    }

    // token is assign.
    Expect(token::Token::ASSIGN);

    if (!is_array) {
        if (tok_ == token::Token::INTCON || token::Token::CHARCON) {
            single_decl_node->val_ = make_shared<ast::BasicLitNode>(tok_, lit_);
        } else if (tok_ == token::Token::IDENFR) {
            single_decl_node->val_ = make_shared<ast::IdentNode>(lit_);
        } else {
            Error(pos_, "var define should be <int/char> ident = <int/char/identfr>;");
            return make_shared<ast::BadDeclNode>();
        }
        
        // Point to next token.
        Next();
        return single_decl_node;
    }

    // Parse array value.
    auto composite_lit_node = make_shared<ast::CompositeLitNode>();
    single_decl_node->val_ = composite_lit_node;
    stack<shared_ptr<ast::CompositeLitNode>> composite_lit_stack;
    composite_lit_stack.push(composite_lit_node);

    auto current_composite_lit_node = composite_lit_node;
    Expect(token::Token::LBRACE);
    while (!composite_lit_stack.empty()) {
        switch (tok_) {
            case token::Token::INTCON:
            case token::Token::CHARCON:
                cout << "GetConstLit" << lit_ << endl;
                current_composite_lit_node->items_.push_back(make_shared<ast::BasicLitNode>(tok_, lit_));
                break;
            case token::Token::IDENFR:
                cout << "GetINDETLit" << lit_ << endl;
                current_composite_lit_node->items_.push_back(make_shared<ast::IdentNode>(lit_));
                break;
            case token::Token::COMMA:
                break;
            case token::LBRACE:
                composite_lit_stack.push(current_composite_lit_node);
                current_composite_lit_node = make_shared<ast::CompositeLitNode>();
                break;
            case token::RBRACE:
                composite_lit_stack.pop();
                if (!composite_lit_stack.empty()) {
                    current_composite_lit_node = composite_lit_stack.top();
                }
                break;
            default:
                Error(pos_, "array define should be <int/char> ident = <int/char/identfr>;");
                return make_shared<ast::BadDeclNode>();
        }
        // Point to next token.
        Next();
    }

    cout << "ParseSingleVar ok, current lit_ is :=> " << lit_ << "token_is -> " << token::GetTokenName(tok_) << endl;

    return single_decl_node;
}

// Report all parse errors.
void Parser::ReportErrors() {
    for (auto &err : errors_) {
        cerr << err.pos.line << ":" << err.pos.column << ": " << err.message << endl;
    }
}
