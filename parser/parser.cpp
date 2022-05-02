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
shared_ptr<ast::TypeNode> NewArrayTypeNode(token::Token tok, const vector<int>& dimesions) {
    if (dimesions.empty() || (tok != token::CHARTK && tok != token::INTTK)) {
        return make_shared<ast::BadTypeNode>();
    }

    vector<shared_ptr<ast::ArrayTypeNode>> array_type_nodes;
    for (const auto& dimension : dimesions) {
        auto array_type_node = make_shared<ast::ArrayTypeNode>();
        array_type_node->size_ = dimension;
        array_type_nodes.push_back(array_type_node);
    }

    for (size_t i = 0; i < (dimesions.size() - 1); i--) {
        array_type_nodes[i]->item_ = array_type_nodes[i + 1];
    }

    array_type_nodes.back()->item_ = NewBasicTypeNode(tok);

    return array_type_nodes.front();
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
    
    // Get Params.
    func_decl_node->params_ = ParseFieldList();
    Expect(token::Token::RPARENT);

    // GetBody.
    func_decl_node->body_ = ParseBlockStmt();
    if (tok_ != token::Token::RBRACE) {
        Error(pos_, "for end of a function, expect }");
        return make_shared<ast::BadDeclNode>();
    }
    Next();

    return func_decl_node;
}

// ParseFieldList e.g. (int a, char b, int c) for a function decl.
// When calling this function, tok_ should be '('.
// After calling this function, tok_ will be ')'.
// return nullptr for badFiledListNode.
shared_ptr<ast::FieldListNode> Parser::ParseFieldList() {
    if (tok_ != token::Token::LPARENT) {
        return nullptr;
    }
    Next();

    auto fields = make_shared<ast::FieldListNode>();
    if (tok_ == token::Token::RPARENT) {
        return fields;
    }

    while(true) {
        auto param = make_shared<ast::FieldNode>();
        fields->fields_.push_back(param);

        // Get input type.
        if (tok_ != token::Token::INTTK && tok_ != token::Token::CHARTK) {
            Error(pos_, "for paramlist, expect <int/char> indetifier");
            return nullptr;
        }
        param->type_ = NewBasicTypeNode(tok_);

        // Get input name.
        Next();
        if (tok_ != token::Token::IDENFR) {
            Error(pos_, "for paramlist, expect <int/char> indetifier");
            return nullptr;
        }
        param->name_ = make_shared<ast::IdentNode>(lit_);

        Next();

        // for ')', break.
        if (tok_ == token::Token::RPARENT) {
            break;
        }

        // not ')', expect ','.
        if (tok_ != token::Token::COMMA) {
            Error(pos_, "for paramlist for func decl, expect ',' between params");
            return nullptr;
        }
        
        // for spliter ',', do next.
        Next();
    }

    return fields;
}


// ParseVarDecl is called for parse variable decl.
// In this function, const, decl_type and name are all parsed.
// When Calling this function, tok_ should be next token of identifre.
// After Calling this function, tok_ should be next token of ';'.
// e.g. 'int a', 'int a = 1', 'int a, b, c';
shared_ptr<ast::DeclNode> Parser::ParseVarDecl(int decl_pos, bool is_const, token::Token decl_type, int name_pos, const string& name) {
    int cur_name_pos = name_pos;
    string cur_name = name;

    // scan token, until get ','.
    auto var_decl_node = make_shared<ast::VarDeclNode>();
    while (true) {
        // Parse current var, current tok_ is '=' or ',' or ';'.
        var_decl_node->decls_.push_back(ParseSingleVarDecl(is_const, decl_pos, decl_type, cur_name_pos, cur_name));

        if (tok_ == token::Token::SEMICN) {
            break;
        }

        if (tok_ != token::Token::COMMA) {
            Error(pos_, "for var decl step word, expect ',' or ';'");
            return make_shared<ast::BadDeclNode>();
        }

        // current tok_ is ',' :=> get next identfire's name.
        Expect(token::Token::COMMA);
        cur_name_pos = pos_;
        cur_name = lit_;
        Expect(token::Token::IDENFR);
    }

    Expect(token::Token::SEMICN);

    return var_decl_node;
}


shared_ptr<ast::StmtNode> Parser::ParseBlockStmt() {
    auto stmt_list = make_shared<ast::BlockStmtNode>();

    Expect(token::Token::LBRACE);

    while(tok_ != token::Token::RBRACE) {
        stmt_list->stmts_.push_back(ParseStmt());
    }

    return stmt_list;
}

shared_ptr<ast::StmtNode> Parser::ParseStmt() {
    shared_ptr<ast::StmtNode> stmt_node;
    switch (tok_) {
        case token::Token::CONSTTK:
        case token::Token::INTTK:
        case token::Token::CHARTK:
            return make_shared<ast::DeclStmtNode>(ParseVarDecl());
        case token::Token::IDENFR:
            return ParseSimpleStmt();
        case token::Token::IFTK:
            return ParseIfStmt();
        case token::Token::WHILETK:
            return ParseWhileStmt();
        case token::Token::SEMICN:
            Next();
            return make_shared<ast::EmptyStmtNode>();
        case token::Token::FORTK:
            return ParseForStmt();
        case token::Token::LBRACE:
            stmt_node = ParseBlockStmt();
            Expect(token::Token::RBRACE);
            return stmt_node;
        case token::Token::PRINTFTK:
            return ParsePrintfStmt();
        case token::Token::SCANFTK:
            return ParseScanStmt();
        case token::Token::SWITCHTK:
            return ParseSwitchStmt();
        case token::Token::RETURNTK:
            return ParseReturnStmt();
        default:
            return make_shared<ast::BadStmtNode>();
    }

    return make_shared<ast::BadStmtNode>();
}

/**
 * @brief ParseSimpleStmt is called for parse simple stmt.
 * 
 * @return shared_ptr<ast::StmtNode> 
 */
shared_ptr<ast::StmtNode> Parser::ParseSimpleStmt() {
    auto x = ParseExpr();

    shared_ptr<ast::ExprNode> y;
    switch (tok_) {
        case token::Token::ASSIGN:
            Next();
            y = ParseExpr();
            return make_shared<ast::AssignStmtNode>(x, y);
        case token::Token::SEMICN:
            Next();
            return make_shared<ast::ExprStmtNode>(x);
        default:
            Error(pos_, "for simple stmt, after first expression, expect '=' or ';'");
            return make_shared<ast::BadStmtNode>();
    }
}

/**
 * @brief ParseScanStmt is called for parse scan stmt.
 * 
 * @return shared_ptr<ast::StmtNode> 
 */
shared_ptr<ast::StmtNode> Parser::ParseScanStmt() {
    if (tok_ != token::Token::SCANFTK) {
        Error(pos_, "for begin of scanf stmt, expect 'scanf'");
        return make_shared<ast::BadStmtNode>();
    }
    Next();

    if (tok_ != token::Token::LPARENT) {
        Error(pos_, "for begin of scanf stmt, expect 'scanf('");
        return make_shared<ast::BadStmtNode>();
    }
    Next();

    auto scanf_stmt = make_shared<ast::ScanStmtNode>();
    if (tok_ != token::Token::IDENFR) {
        Error(pos_, "for expr of scanf stmt, expect indetifier");
        return make_shared<ast::BadStmtNode>();
    }
    scanf_stmt->var_ = make_shared<ast::IdentNode>(lit_);

    Next();

    if (tok_ != token::Token::RPARENT) {
        Error(pos_, "for end of scanf stmt, expect ')'");
        return make_shared<ast::BadStmtNode>();
    }
    Next();

    if (tok_ != token::Token::SEMICN) {
        Error(pos_, "for end of scanf stmt, expect ';'");
        return make_shared<ast::BadStmtNode>();
    }
    Next();

    return scanf_stmt;
}

/**
 * @brief ParsePrintfStmt is called for parse printf stmt.
 * 
 * @return shared_ptr<ast::StmtNode> 
 */
shared_ptr<ast::StmtNode> Parser::ParsePrintfStmt() {
    if (tok_ != token::Token::PRINTFTK) {
        Error(pos_, "for begin of printf stmt, expect 'printf'");
        return make_shared<ast::BadStmtNode>();
    }
    Next();

    if (tok_ != token::Token::LPARENT) {
        Error(pos_, "for begin of printf stmt, expect 'printf('");
        return make_shared<ast::BadStmtNode>();
    }
    Next();

    auto printf_stmt = make_shared<ast::PrintfStmtNode>();
    if (tok_ != token::Token::STRCON) {
        Error(pos_, "for expr of printf stmt, expect string");
        return make_shared<ast::BadStmtNode>();
    }
    printf_stmt->fmt_ = lit_;

    Next();

    // Parse other args.
    while (tok_ == token::Token::COMMA) {
        Next();
        printf_stmt->args_.push_back(ParseExpr());
    }

    if (tok_ != token::Token::RPARENT) {
        Error(pos_, "for end of printf stmt, expect ')'");
        return make_shared<ast::BadStmtNode>();
    }
    Next();

    if (tok_ != token::Token::SEMICN) {
        Error(pos_, "for end of printf stmt, expect ';'");
        return make_shared<ast::BadStmtNode>();
    }
    Next();

    return printf_stmt;
}

/**
 * @brief ParseReturnStmt is called for parse return stmt.
 * 
 * @return shared_ptr<ast::StmtNode> 
 */
shared_ptr<ast::StmtNode> Parser::ParseReturnStmt() {
    if (tok_ != token::Token::RETURNTK) {
        Error(pos_, "for begin of return stmt, expect 'return'");
        return make_shared<ast::BadStmtNode>();
    }
    Next();

    if (tok_ != token::Token::SEMICN) {
        auto expr = ParseExpr();
        if (tok_ != token::Token::SEMICN) {
            Error(pos_, "for end of return stmt, expect ';'");
            return make_shared<ast::BadStmtNode>();
        }
        Next();
        return make_shared<ast::ReturnStmtNode>(expr);
    }

    if (tok_ != token::Token::SEMICN) {
        Error(pos_, "for end of return stmt, expect ';'");
        return make_shared<ast::BadStmtNode>();
    }
    Next();
    return make_shared<ast::ReturnStmtNode>();
}

shared_ptr<ast::StmtNode> Parser::ParseSwitchStmt() {

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
    string name = lit_;
    if(tok_ != token::Token::IDENFR) {
        Error(pos_, "for var decl, expect <int/char> indetifier");
        return make_shared<ast::BadDeclNode>();
    }

    Expect(token::Token::IDENFR);
    
    return ParseVarDecl(decl_pos, is_const, decl_type, name_pos, name);
}

// ParseSingleVarDecl parse single var.
// When Calling this function, tok_ is next of IDENFR.
// After Calling this function, tok_ is ',' or ';'. 
shared_ptr<ast::DeclNode> Parser::ParseSingleVarDecl(int is_const, int decl_pos, token::Token decl_type, int name_pos, const string& name) {
    auto single_decl_node = make_shared<ast::SingleVarDeclNode>();
    single_decl_node->type_ = NewBasicTypeNode(decl_type);
    single_decl_node->name_ = make_shared<ast::IdentNode>(name);
    single_decl_node->is_const_ = is_const;

    // get_array_dimension is called for parse array dimension.
    // dimesions is a vector of dimension.
    // return int for ErrorCode.
    // When Calling this function, tok_ is next of '['.
    // After Calling this function, tok_ is next of ']'.
    auto get_array_dimension = [&](vector<int>& dimesions) -> int {
        while (tok_ == token::Token::LBRACK) {
            Next();
            if (tok_ == token::Token::INTCON) {
                dimesions.push_back(atoi(lit_.c_str()));
                Next();
            } else {
                Error(pos_, "for array dimension, expect [int]");
                return 1;
            }

            if (tok_ != token::Token::RBRACK) {
                Error(pos_, "for array dimension, expect ]");
                return 1;
            }
            Next();
        }

        return 0;
    };

    // only var name.
    if (tok_ == token::Token::SEMICN || tok_ == token::Token::COMMA) {
        return single_decl_node;
    }

    // is array.
    if (tok_ == token::Token::LBRACK) {
        vector<int> dimesions;
        if (get_array_dimension(dimesions) != 0) {
            return make_shared<ast::BadDeclNode>();
        }
        single_decl_node->type_ = NewArrayTypeNode(decl_type, dimesions);
        
        Expect(token::Token::ASSIGN);

        single_decl_node->val_ = ParseCompositeLit(decl_type);
        return single_decl_node;
    }

    // token is assign.
    if (tok_ != token::Token::ASSIGN) {
        Error(pos_, "for var decl, expect '=' or ';' or ',' After identifr");
        return make_shared<ast::BadDeclNode>();
    }
    Next();

    if (tok_ == token::Token::INTCON || tok_ == token::Token::CHARCON) {
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

// ParseCompositeLit is called for parse composite literal.
// @param decl_type: INTTK or CHARTK.
// e.g. '{ 1, 2, 3 }', '{{1,2,3}, {4,5,6}}';
shared_ptr<ast::ExprNode> Parser::ParseCompositeLit(token::Token decl_type) {
    auto composite_lit_node = make_shared<ast::CompositeLitNode>();
    stack<shared_ptr<ast::CompositeLitNode>> composite_lit_stack;
    composite_lit_stack.push(composite_lit_node);
    auto current_composite_lit_node = composite_lit_node;
    shared_ptr<ast::UnaryExprNode> unary_expr_node;
    shared_ptr<ast::CompositeLitNode> sub_composite_lit_node;
    Expect(token::Token::LBRACE);
    auto get_item = [&]() -> shared_ptr<ast::ExprNode> {
        if (tok_ == token::Token::INTCON || tok_ == token::Token::CHARCON) {
            return make_shared<ast::BasicLitNode>(tok_, lit_);
        }

        if (tok_ == token::Token::IDENFR) {
            return make_shared<ast::IdentNode>(lit_);
        }

        if (tok_ != token::Token::PLUS && tok_ != token::Token::MINU) {
            return make_shared<ast::BadExprNode>();
        }
        
        // get signed lit.
        unary_expr_node = make_shared<ast::UnaryExprNode>(tok_, nullptr);
        Next();

        if (tok_ == token::Token::INTCON) {
            unary_expr_node->x_ = make_shared<ast::BasicLitNode>(tok_, lit_);
            return unary_expr_node;
        }
        if (tok_ == token::Token::IDENFR) {
            unary_expr_node->x_ = make_shared<ast::IdentNode>(lit_);
            return unary_expr_node;
        }


        Error(pos_, "for unary expr, expect <int/char>");
        return make_shared<ast::BadExprNode>();
    };
    while (!composite_lit_stack.empty()) {
        switch (tok_) {
            // 无符号.
            case token::Token::INTCON:
            case token::Token::CHARCON:
            case token::Token::PLUS:
            case token::Token::MINU:
            case token::Token::IDENFR:
                current_composite_lit_node->items_.push_back(get_item());
                break;
            case token::Token::COMMA:
                break;
            case token::LBRACE:
                sub_composite_lit_node = make_shared<ast::CompositeLitNode>();
                current_composite_lit_node->items_.push_back(sub_composite_lit_node);
                composite_lit_stack.push(current_composite_lit_node);
                current_composite_lit_node = sub_composite_lit_node;
                break;
            case token::RBRACE:
                composite_lit_stack.pop();
                if (!composite_lit_stack.empty()) {
                    current_composite_lit_node = composite_lit_stack.top();
                }
                break;
            default:
                Error(pos_, "array define should be <int/char> ident = <int/char/identfr>;");
                return make_shared<ast::BadExprNode>();
        }
        // Point to next token.
        Next();
    }

    return composite_lit_node;
}

// ParseIfStmt is called for parse if statement.
// When Calling this function, tok_ should be IFTK.
// After Calling this function, tok_ will be next token of '}'.
shared_ptr<ast::StmtNode> Parser::ParseIfStmt() {
    auto ret_if_stmt_node = make_shared<ast::IfStmtNode>();
    if (tok_ != token::Token::IFTK) {
        Error(pos_, "for begin of if statement, expect if");
        return make_shared<ast::BadStmtNode>();
    }
    Next();

    if (tok_ != token::Token::LPARENT) {
        Error(pos_, "for begin of if stament, expect if (");
        return make_shared<ast::BadStmtNode>();
    }
    Next();

    ret_if_stmt_node->cond_ = ParseExpr();

    if (tok_ != token::Token::RPARENT) {
        Error(pos_, "for begin of if statement, expect '('");
        return make_shared<ast::BadStmtNode>();
    }
    Next();

    ret_if_stmt_node->body_ = ParseBlockStmt();
    if (tok_ != token::Token::RBRACE) {
        Error(pos_, "for end of if statement, expect '}'");
        return make_shared<ast::BadStmtNode>();
    }
    Next();

    if (tok_ == token::Token::ELSETK) {
        Next();
        if (tok_ == token::Token::IFTK) {
            ret_if_stmt_node->else_ = ParseIfStmt();
        } else {
            ret_if_stmt_node->else_ = ParseBlockStmt();
            Expect(token::Token::RBRACE);
        }
    }

    return ret_if_stmt_node;
}

shared_ptr<ast::StmtNode> Parser::ParseWhileStmt() {
    auto ret_while_stmt_node = make_shared<ast::WhileStmtNode>();
    if (tok_ != token::Token::WHILETK) {
        Error(pos_, "for begin of while statement, expect while");
        return make_shared<ast::BadStmtNode>();
    }
    Next();

    if (tok_ != token::Token::LPARENT) {
        Error(pos_, "for begin of while statement, expect '('");
        return make_shared<ast::BadStmtNode>();
    }
    Next();

    ret_while_stmt_node->cond_ = ParseExpr();

    if (tok_ != token::Token::RPARENT) {
        Error(pos_, "for begin of while statement, expect ')'");
        return make_shared<ast::BadStmtNode>();
    }
    Next();

    ret_while_stmt_node->body_ = ParseBlockStmt();
    if (tok_ != token::Token::RBRACE) {
        Error(pos_, "for end of while statement, expect '}'");
        return make_shared<ast::BadStmtNode>();
    }
    Next();

    return ret_while_stmt_node;
}

shared_ptr<ast::StmtNode> Parser::ParseForStmt() {
    auto ret_for_stmt_node = make_shared<ast::ForStmtNode>();
    if (tok_ != token::Token::FORTK) {
        Error(pos_, "for begin of for statement, expect for");
        return make_shared<ast::BadStmtNode>();
    }
    Next();

    if (tok_ != token::Token::LPARENT) {
        Error(pos_, "for begin of for statement, expect '('");
        return make_shared<ast::BadStmtNode>();
    }
    Next();

    if (tok_ != token::Token::SEMICN) {
        ret_for_stmt_node->init_ = ParseStmt();
    }

    if (tok_ != token::Token::SEMICN) {
        ret_for_stmt_node->cond_ = ParseStmt();
    }

    if (tok_ != token::Token::RPARENT) {
        ret_for_stmt_node->step_ = ParseStmt();
    }
    Expect(token::Token::RPARENT);

    ret_for_stmt_node->body_ = ParseBlockStmt();
    if (tok_ != token::Token::RBRACE) {
        Error(pos_, "for end of for statement, expect '}'");
        return make_shared<ast::BadStmtNode>();
    }
    Next();

    return ret_for_stmt_node;
}

// ParseExpr is called for parse expression.
// Cause those tokens will only used in <for/while/if> cond_.
// When Calling this function, tok_ should be first token of expression.
// After Calling this function, tok_ will be next token of expression.
// After Calling tok_ e.g.
//  - if (expr == expr')  :=> '=='
//  - fake = foo + bar;   :=> ';'
shared_ptr<ast::ExprNode> Parser::ParseExpr() {
    return ParseBinaryExpr(token::kLowestPrecedence + 1);
}

/**
 * @brief ParseBinaryExpr is called for parse binary expression.
 * When Calling this function, tok_ should be first token of binary expression.
 * After Calling this function, tok_ will be next token of binary expression.
 * 
 * @param prec is prec of current token.
 * @return shared_ptr<ast::ExprNode> BinaryExprNode for success, BadExprNode for fail.
 */
shared_ptr<ast::ExprNode> Parser::ParseBinaryExpr(int prec) {
    auto left_expr_node = ParseUnaryExpr();

    while (true) {
        int tok_prec = token::GetPrecedence(tok_);
        if (tok_prec < prec) {
            return left_expr_node;
        }

        auto op_token = tok_;
        Next();

        auto right_expr_node = ParseBinaryExpr(tok_prec + 1);
        left_expr_node = make_shared<ast::BinaryExprNode>(op_token, left_expr_node, right_expr_node);
    }
}

/**
 * @brief ParseUnaryExpr is called for parse unary expression.
 * When Calling this function, tok_ should be first token of unary expression.
 * After Calling this function, tok_ will be next token of unary expression.
 * 
 * @return shared_ptr<ast::ExprNode> UnaryExprNode for success, BadExprNode for fail.
 */
shared_ptr<ast::ExprNode> Parser::ParseUnaryExpr() {
    if (tok_ == token::Token::PLUS || tok_ == token::Token::MINU) {
        token::Token op = tok_;
        Next();
        return make_shared<ast::UnaryExprNode>(op, ParseUnaryExpr());
    }

    return ParsePrimaryExpr();
}

/**
 * @brief ParsePrimaryExpr is called for parse primary expression.
 * When Calling this function, tok_ should be first token of primary expression.
 * After Calling this function, tok_ will be next token of primary expression.
 * 
 * @return shared_ptr<ast::ExprNode> <BaiscLitNode/IdentNode/CallExprNode> for success, BadExprNode for fail.
 */
shared_ptr<ast::ExprNode> Parser::ParsePrimaryExpr() {
    auto x = ParseOperand();

    // '(' function call.
    if (tok_ == token::Token::LPARENT) {
        return ParseCallExpr(x);
    }

    // '[' => index.
    if (tok_ == token::Token::LBRACK) {
        return ParseIndexExpr(x);
    }

    return x;
}

/**
 * @brief ParseOperand is called for parse operand.
 * When Calling this function, tok_ should be first token of operand.
 * After Calling this function, tok_ will be next token of operand.
 * e.g.
 *  - (1 + 2 + 3) => after call token is : next token of ')'
 *  - 1 + 2       => after call token is : '+'
 *  - ident + 1   => after call token is : '+'
 * 
 * @return shared_ptr<ast::ExprNode> 
 */
shared_ptr<ast::ExprNode> Parser::ParseOperand() {
    shared_ptr<ast::ExprNode> ret; 
    switch (tok_) {
        case token::Token::IDENFR:
            ret = make_shared<ast::IdentNode>(lit_);
            Next();
            return ret;
        case token::Token::INTCON:
        case token::Token::CHARCON:
        case token::Token::STRCON:
            ret = make_shared<ast::BasicLitNode>(tok_, lit_);
            Next();
            return ret;
        case token::Token::LPARENT:
            Next();
            ret = make_shared<ast::ParenExprNode>(ParseExpr());
            if (tok_ != token::Token::RPARENT) {
                Error(pos_, "in operand start with '(', expect end with ')'");
                return make_shared<ast::BadExprNode>();
            }
            Next();
            return ret;
        default:
            Error(pos_, "in operand, expect <int/char/idenfr/string/'('>");
            return make_shared<ast::BadExprNode>();
    }
}

// ParseCallExpr is called for parse function call expression.
// When Calling this function, tok_ should be '(' of function call.
// After Calling this function, tok_ will be ')' of function call.
// e.g.
//   start calling (arg1, arg2, arg3)  :=> tok_ is '('
//   end calling (arg1, arg2, arg3)    :=> tok_ is ')'
shared_ptr<ast::ExprNode> Parser::ParseCallExpr(const shared_ptr<ast::ExprNode>& func_name) {
    if (tok_ != token::Token::LPARENT) {
        Error(pos_, "in function call, expect '('");
        return make_shared<ast::BadExprNode>();
    }
    Next();

    auto func_call_expr_node = make_shared<ast::CallExprNode>();
    func_call_expr_node->fun_ = func_name;

    while (tok_ != token::Token::RPARENT) {
        auto arg_expr_node = ParseExpr();
        func_call_expr_node->args_.push_back(arg_expr_node);

        if (tok_ != token::Token::COMMA) {
            break;
        }
        Next();
    }

    if (tok_ != token::Token::RPARENT) {
        Error(pos_, "in function call, expect ')'");
        return make_shared<ast::BadExprNode>();
    }
    Next();

    return func_call_expr_node;
}

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
shared_ptr<ast::ExprNode> Parser::ParseIndexExpr(const shared_ptr<ast::ExprNode>& array_name) {
    if (tok_ == token::Token::RBRACK) {
        Error(pos_, "for index expr, [] is not valid");
        return make_shared<ast::BadExprNode>();
    }
    Next();

    vector<shared_ptr<ast::ExprNode>> index_expr_nodes;
    while (true) {
        index_expr_nodes.push_back(ParseExpr());

        if (tok_ != token::Token::RBRACK) {
            Error(pos_, "for end of array index, expect ]");
            return make_shared<ast::BadExprNode>();
        }
        Next();

        if (tok_ != token::Token::LBRACK) {
            break;
        }
        Next();
    }

    shared_ptr<ast::ExprNode> current_node = array_name;
    for (auto& index_expr_node : index_expr_nodes) {
        current_node = make_shared<ast::IndexExprNode>(current_node, index_expr_node);
    }

    return current_node;
}



// Report all parse errors.
void Parser::ReportErrors() {
    for (auto &err : errors_) {
        cerr << err.pos.line << ":" << err.pos.column << ": " << err.message << endl;
    }
}
