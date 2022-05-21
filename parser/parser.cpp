#include <memory>
#include <iostream>
#include <stack>
#include "parser/parser.h"
#include "ast/ast.h"
#include "token/token.h"

// Helper function to parse ast.

// NewBasicTypeNode is called for return basic type node.
shared_ptr<ast::TypeNode> NewBasicTypeNode(const token::Position& pos, token::Token tok) {
    if (tok == token::Token::CHARTK) {
        return make_shared<ast::CharTypeNode>(pos);
    } else if (tok == token::Token::INTTK) {
        return make_shared<ast::IntTypeNode>(pos);
    } else if (tok == token::Token::VOIDTK) {
        return make_shared<ast::VoidTypeNode>(pos);
    } else {
        return make_shared<ast::BadTypeNode>(pos);
    }
}

// NewArrayTypeNode is called for return array type node.
shared_ptr<ast::TypeNode> NewArrayTypeNode(const token::Position& pos, token::Token tok, const vector<int>& dimesions) {
    if (dimesions.empty() || (tok != token::CHARTK && tok != token::INTTK)) {
        return make_shared<ast::BadTypeNode>(pos);
    }

    vector<shared_ptr<ast::ArrayTypeNode>> array_type_nodes;
    for (const auto& dimension : dimesions) {
        auto array_type_node = make_shared<ast::ArrayTypeNode>(pos);
        array_type_node->size_ = dimension;
        array_type_nodes.push_back(array_type_node);
    }

    for (size_t i = 0; i < (dimesions.size() - 1); i--) {
        array_type_nodes[i]->item_ = array_type_nodes[i + 1];
    }

    array_type_nodes.back()->item_ = NewBasicTypeNode(pos, tok);

    return array_type_nodes.front();
}


Parser::Parser(
    const shared_ptr<token::File> &file,
    const string &src,
    const shared_ptr<ErrorHandler> &err,
    const shared_ptr<ec::ErrorReminder>& errors
) {
    scanner_ = make_shared<Scanner>(file, src, err);
    errors_ = errors;
    file_ = file;
    Next();
}

// Parse the source code and return the corresponding ast file tree.
shared_ptr<ast::FileNode> Parser::Parse() {
    vector<shared_ptr<ast::DeclNode>> decls;
    while (tok_ != token::END_OF_FILE) {
        decls.push_back(ParseDecl());
    }

    auto ast_file_node = make_shared<ast::FileNode>();
    ast_file_node->decl_.insert(ast_file_node->decl_.begin(), decls.begin(), decls.end());

    return ast_file_node;
}

/**
 * Error reports that the current token is unexpected.
 */
void Parser::Error(const token::Position& pos, ec::Type error_type, const string& msg) {
    errors_->Add(pos, ec::Error(pos, error_type, msg));
    exit(EXIT_SUCCESS);
}

void Parser::Expect(token::Token tok) {
    if (tok_ != tok) {
        string msg = "expect " + token::GetTokenName(tok) + ", but get " + token::GetTokenName(tok_);
        cout << "Expect: " << token::GetTokenName(tok) << endl;
        ec::Type error_type = (
            (tok == token::Token::SEMICN) ? ec::Type::SEMICNExpected :
            (tok == token::Token::RBRACK) ? ec::Type::RBRACKExpected :
            (tok == token::Token::RPARENT) ? ec::Type::RPARENTExpected :
            ec::Type::NotInHomeWork
        );
        Error(pos_, error_type, msg);
    }

    Next();

    return;
}

// Next advance to the next token.
void Parser::Next() {
    int after_next_pos;
    scanner_->Scan(&after_next_pos, &tok_, &lit_);
    pos_ = file_->GetPositionByOffset(after_next_pos);
}

// ParserDecl is called for parse decl.
// e.g. 'int a', 'int a = 1', 'int a, b, c', 'int main() { ... }';
shared_ptr<ast::DeclNode> Parser::ParseDecl() {
    auto decl_pos = pos_;
    bool is_const = false;
    if (tok_ == token::Token::CONSTTK) {
        is_const = true;
        Next();
    }

    token::Token decl_type = tok_;
    Next();
    if (decl_type != token::Token::INTTK && decl_type != token::Token::CHARTK && decl_type != token::Token::VOIDTK) {
        Error(pos_, ec::Type::NotInHomeWork, "for begin of a declear, expect int, char, or void");
        return make_shared<ast::BadDeclNode>(pos_);
    }

    auto name_pos = pos_;
    string name = lit_;
    if (tok_ != token::Token::MAINTK && tok_ != token::Token::IDENFR) {
        Error(pos_, ec::Type::NotInHomeWork, "for decl, expect <int/char/void> name");
        return make_shared<ast::BadDeclNode>(pos_);
    }
    Next();

    if (tok_ == token::Token::LPARENT) {
        if (is_const) {
            Error(pos_, ec::Type::NotInHomeWork, "const function result type not supported");
            return make_shared<ast::BadDeclNode>(pos_);
        }
        return ParseFuncDecl(decl_pos, decl_type, name_pos, name);
    } 

    return ParseVarDecl(decl_pos, is_const, decl_type, name_pos, name);
}

// ParseFuncDecl is called for parse function decl.
// e.g. 'int main() { ... }';
shared_ptr<ast::DeclNode> Parser::ParseFuncDecl(
    const token::Position& decl_pos,
    token::Token decl_type,
    const token::Position& name_pos,
    const string& name
) {
    auto func_type = NewBasicTypeNode(decl_pos, decl_type);
    auto func_name = make_shared<ast::IdentNode>(name_pos, name);

    // Parse func param list.
    auto func_params = ParseFieldList();
    Expect(token::Token::RPARENT);

    // Parse func body.
    auto func_body = ParseBlockStmt();
    Expect(token::Token::RBRACE);

    return make_shared<ast::FuncDeclNode>(decl_pos, func_type, func_name, func_params, func_body);
}

// ParseFieldList e.g. (int a, char b, int c) for a function decl.
// When calling this function, tok_ should be '('.
// After calling this function, tok_ will be ')'.
// return nullptr for badFiledListNode.
shared_ptr<ast::FieldListNode> Parser::ParseFieldList() {
    auto fields = make_shared<ast::FieldListNode>(pos_);

    Expect(token::Token::LPARENT);
    if (tok_ == token::Token::RPARENT) {
        return fields;
    }

    while(true) {
        // Get input type.
        if (tok_ != token::Token::INTTK && tok_ != token::Token::CHARTK) {
            Error(pos_, ec::Type::NotInHomeWork, "for paramlist, expect <int/char> indetifier");
        }
        auto param_type = NewBasicTypeNode(pos_, tok_);
        Next();

        // Get param name.
        auto param_name = make_shared<ast::IdentNode>(pos_, lit_);
        Expect(token::Token::IDENFR);

        fields->fields_.push_back(make_shared<ast::FieldNode>(param_type->Pos(), param_type, param_name));

        // for not comma, break.
        if (tok_ != token::Token::COMMA) {
            break;
        }
        Expect(token::Token::COMMA);
    }

    return fields;
}


// ParseVarDecl is called for parse variable decl.
// In this function, const, decl_type and name are all parsed.
// When Calling this function, tok_ should be next token of identifre.
// After Calling this function, tok_ should be next token of ';'.
// e.g. 'int a', 'int a = 1', 'int a, b, c';
shared_ptr<ast::DeclNode> Parser::ParseVarDecl(
    const token::Position& decl_pos,
    bool is_const,
    token::Token decl_type,
    const token::Position& name_pos,
    const string& name
) {
    auto var_decl_node = make_shared<ast::VarDeclNode>(decl_pos);

    auto cur_name_pos = name_pos;
    auto cur_name = name;
    // scan token, until get ','.
    while (true) {
        // Parse current var, current tok_ is '=' or ',' or ';'.
        var_decl_node->decls_.push_back(ParseSingleVarDecl(decl_pos, is_const, decl_type, cur_name_pos, cur_name));

        if (tok_ == token::Token::SEMICN) {
            break;
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
    auto stmt_list = make_shared<ast::BlockStmtNode>(pos_);

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
            return make_shared<ast::DeclStmtNode>(pos_, ParseVarDecl());
        case token::Token::IDENFR:
            stmt_node = ParseSimpleStmt();
            Expect(token::Token::SEMICN);
            return stmt_node;
        case token::Token::IFTK:
            return ParseIfStmt();
        case token::Token::WHILETK:
            return ParseWhileStmt();
        case token::Token::SEMICN:
            stmt_node = make_shared<ast::EmptyStmtNode>(pos_);
            Next();
            return stmt_node;
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
            return make_shared<ast::BadStmtNode>(pos_);
    }
}

/**
 * @brief ParseSimpleStmt is called for parse simple stmt.
 * After process, tok_ should be ';'.
 * @return shared_ptr<ast::StmtNode> 
 */
shared_ptr<ast::StmtNode> Parser::ParseSimpleStmt() {
    auto x = ParseExpr();

    shared_ptr<ast::ExprNode> y;
    switch (tok_) {
        case token::Token::ASSIGN:
            Next();
            y = ParseExpr();
            return make_shared<ast::AssignStmtNode>(x->Pos(), x, y);
        case token::Token::SEMICN:
            return make_shared<ast::ExprStmtNode>(x->Pos(), x);
        default:
            Error(pos_, ec::Type::NotInHomeWork, "for simple stmt, after first expression, expect '=' or ';'");
            return make_shared<ast::BadStmtNode>(x->Pos());
    }
}

/**
 * @brief ParseScanStmt is called for parse scan stmt.
 * 
 * @return shared_ptr<ast::StmtNode> 
 */
shared_ptr<ast::StmtNode> Parser::ParseScanStmt() {
    auto scanf_stmt = make_shared<ast::ScanStmtNode>(pos_);

    Expect(token::Token::SCANFTK);
    Expect(token::Token::LPARENT);

    if (tok_ != token::Token::IDENFR) {
        Error(pos_, ec::Type::NotInHomeWork, "for expr of scanf stmt, expect indetifier");
        scanf_stmt->var_ = make_shared<ast::BadExprNode>(pos_);
    } else {
        scanf_stmt->var_ = make_shared<ast::IdentNode>(pos_, lit_);
    }
    Next();

    Expect(token::Token::RPARENT);
    Expect(token::Token::SEMICN);

    return scanf_stmt;
}

/**
 * @brief ParsePrintfStmt is called for parse printf stmt.
 * 
 * @return shared_ptr<ast::StmtNode> 
 */
shared_ptr<ast::StmtNode> Parser::ParsePrintfStmt() {
    auto printf_stmt = make_shared<ast::PrintfStmtNode>(pos_);

    Expect(token::Token::PRINTFTK);
    Expect(token::Token::LPARENT);

    while (tok_ != token::Token::RPARENT) {
        printf_stmt->args_.push_back(ParseExpr());

        if (tok_ == token::Token::COMMA) {
            Next();
        } else {
            break;
        }
    }

    Expect(token::Token::RPARENT);
    Expect(token::Token::SEMICN);

    return printf_stmt;
}

/**
 * @brief ParseReturnStmt is called for parse return stmt.
 * 
 * @return shared_ptr<ast::StmtNode> 
 */
shared_ptr<ast::StmtNode> Parser::ParseReturnStmt() {
    auto return_stmt = make_shared<ast::ReturnStmtNode>(pos_);

    Expect(token::Token::RETURNTK);

    if (tok_ != token::Token::SEMICN) {
        return_stmt->results_ = ParseExpr();
    }

    Expect(token::Token::SEMICN);
    return return_stmt;
}

/**
 * @brief ParseSwitchStmt is called for parse switch stmt.
 * 
 * @return shared_ptr<ast::StmtNode> 
 */
shared_ptr<ast::StmtNode> Parser::ParseSwitchStmt() {
    auto switch_stmt = make_shared<ast::SwitchStmtNode>(pos_);

    Expect(token::Token::SWITCHTK);
    Expect(token::Token::LPARENT);

    switch_stmt->cond_ = ParseExpr();

    Expect(token::Token::RPARENT);
    Expect(token::Token::LBRACE);
    
    while (tok_ == token::Token::CASETK || tok_ == token::Token::DEFAULTTK) {
        switch_stmt->cases_.push_back(ParseCaseStmt());
    }

    Expect(token::Token::RBRACE);

    return switch_stmt;
}

/**
 * @brief ParseCaseStmt is called for parse case stmt.
 * 
 * @return shared_ptr<ast::StmtNode> 
 */
shared_ptr<ast::StmtNode> Parser::ParseCaseStmt() {
    auto case_stmt_node = make_shared<ast::CaseStmtNode>(pos_);

    if (tok_ == token::Token::CASETK) {
        Next();
        case_stmt_node->cond_ = ParseExpr();
    } else if (tok_ == token::Token::DEFAULTTK) {
        Next();
        case_stmt_node->cond_ = nullptr;
    } else {
        Error(pos_, ec::Type::NotInHomeWork, "for begin of case stmt, expect <case/default>");
        // FIXME: what todo?
    }

    Expect(token::Token::COLON);

    while (tok_ != token::Token::CASETK && tok_ != token::Token::DEFAULTTK && tok_ != token::RBRACE) {
        case_stmt_node->body_.push_back(ParseStmt());
    }

    return case_stmt_node;
}

// ParseVarDecl is called for parse variable decl.
// In this function, we start from token 'const' or 'int / char'.
// e.g. 'int a', 'int a = 1', 'int a, b, c';
shared_ptr<ast::DeclNode> Parser::ParseVarDecl() {
    auto decl_pos = pos_;
    bool is_const = false;
    if (tok_ == token::Token::CONSTTK) {
        is_const = true;
        Next();
    }

    token::Token decl_type = tok_;
    if (decl_type != token::Token::CHARTK && decl_type != token::Token::INTTK) {
        Error(pos_, ec::Type::NotInHomeWork, "for begin of a declear, expect int or char");
        return make_shared<ast::BadDeclNode>(pos_);
    }
    Next();

    auto name_pos = pos_;
    string name = lit_;

    Expect(token::Token::IDENFR);
    
    return ParseVarDecl(decl_pos, is_const, decl_type, name_pos, name);
}

// ParseSingleVarDecl parse single var.
// When Calling this function, tok_ is next of IDENFR.
// After Calling this function, tok_ is ',' or ';'. 
shared_ptr<ast::DeclNode> Parser::ParseSingleVarDecl(
    const token::Position& decl_pos,
    int is_const,
    token::Token decl_type,
    const token::Position& name_pos,
    const string& name
) {
    auto single_decl_node = make_shared<ast::SingleVarDeclNode>(name_pos);

    single_decl_node->type_ = NewBasicTypeNode(decl_pos, decl_type);
    single_decl_node->name_ = make_shared<ast::IdentNode>(name_pos, name);
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
                Error(pos_, ec::Type::NotInHomeWork, "for array dimension, expect [int]");
                return 1;
            }

            Expect(token::Token::RBRACK);
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
            return make_shared<ast::BadDeclNode>(name_pos);
        }
        single_decl_node->type_ = NewArrayTypeNode(name_pos, decl_type, dimesions);
        
        if (tok_ == token::Token::SEMICN || tok_ == token::Token::COMMA) {
            return single_decl_node;
        }

        Expect(token::Token::ASSIGN);

        single_decl_node->val_ = ParseCompositeLit(decl_type);
        return single_decl_node;
    }

    // token is assign.
    Expect(token::Token::ASSIGN);

    single_decl_node->val_ = ParseExpr();

    return single_decl_node;
}

// ParseCompositeLit is called for parse composite literal.
// @param decl_type: INTTK or CHARTK.
// e.g. '{ 1, 2, 3 }', '{{1,2,3}, {4,5,6}}';
shared_ptr<ast::ExprNode> Parser::ParseCompositeLit(token::Token decl_type) {
    auto composite_lit_node = make_shared<ast::CompositeLitNode>(pos_);
    stack<shared_ptr<ast::CompositeLitNode>> composite_lit_stack;
    composite_lit_stack.push(composite_lit_node);
    auto current_composite_lit_node = composite_lit_node;
    shared_ptr<ast::UnaryExprNode> unary_expr_node;
    shared_ptr<ast::CompositeLitNode> sub_composite_lit_node;
    Expect(token::Token::LBRACE);
    auto get_item = [&]() -> shared_ptr<ast::ExprNode> {
        if (tok_ == token::Token::INTCON || tok_ == token::Token::CHARCON) {
            return make_shared<ast::BasicLitNode>(pos_, tok_, lit_);
        }

        if (tok_ == token::Token::IDENFR) {
            return make_shared<ast::IdentNode>(pos_, lit_);
        }

        if (tok_ != token::Token::PLUS && tok_ != token::Token::MINU) {
            return make_shared<ast::BadExprNode>(pos_);
        }
        
        // get signed lit.
        unary_expr_node = make_shared<ast::UnaryExprNode>(pos_, tok_, nullptr);
        Next();

        if (tok_ == token::Token::INTCON) {
            unary_expr_node->x_ = make_shared<ast::BasicLitNode>(pos_, tok_, lit_);
            return unary_expr_node;
        }
        if (tok_ == token::Token::IDENFR) {
            unary_expr_node->x_ = make_shared<ast::IdentNode>(pos_, lit_);
            return unary_expr_node;
        }


        Error(pos_, ec::Type::NotInHomeWork, "for unary expr, expect <int/char>");
        return make_shared<ast::BadExprNode>(pos_);
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
                sub_composite_lit_node = make_shared<ast::CompositeLitNode>(pos_);
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
                Error(pos_, ec::Type::NotInHomeWork, "array define should be <int/char> ident = <int/char/identfr>;");
                return make_shared<ast::BadExprNode>(pos_);
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
    auto ret_if_stmt_node = make_shared<ast::IfStmtNode>(pos_);

    Expect(token::Token::IFTK);
    Expect(token::Token::LPARENT);

    ret_if_stmt_node->cond_ = ParseExpr();

    Expect(token::Token::RPARENT);

    ret_if_stmt_node->body_ = ParseStmt();

    if (tok_ == token::Token::ELSETK) {
        Next();
        ret_if_stmt_node->else_ = ParseStmt();
    }

    return ret_if_stmt_node;
}

shared_ptr<ast::StmtNode> Parser::ParseWhileStmt() {
    auto ret_while_stmt_node = make_shared<ast::WhileStmtNode>(pos_);

    Expect(token::Token::WHILETK);
    Expect(token::Token::LPARENT);

    ret_while_stmt_node->cond_ = ParseExpr();

    Expect(token::Token::RPARENT);

    ret_while_stmt_node->body_ = ParseStmt();

    return ret_while_stmt_node;
}

shared_ptr<ast::StmtNode> Parser::ParseForStmt() {
    auto ret_for_stmt_node = make_shared<ast::ForStmtNode>(pos_);

    Expect(token::Token::FORTK);
    Expect(token::Token::LPARENT);

    if (tok_ != token::Token::SEMICN) {
        ret_for_stmt_node->init_ = ParseSimpleStmt();
        Expect(token::Token::SEMICN);
    }

    if (tok_ != token::Token::SEMICN) {
        ret_for_stmt_node->cond_ = ParseSimpleStmt();
        Expect(token::Token::SEMICN);
    }

    if (tok_ != token::Token::RPARENT) {
        ret_for_stmt_node->step_ = ParseSimpleStmt();
    }
    Expect(token::Token::RPARENT);

    ret_for_stmt_node->body_ = ParseStmt();

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
        left_expr_node = make_shared<ast::BinaryExprNode>(left_expr_node->Pos(), op_token, left_expr_node, right_expr_node);
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
        auto op_position = pos_;
        token::Token op = tok_;
        Next();
        return make_shared<ast::UnaryExprNode>(op_position, op, ParseUnaryExpr());
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
            ret = make_shared<ast::IdentNode>(pos_, lit_);
            Next();
            return ret;
        case token::Token::INTCON:
        case token::Token::CHARCON:
        case token::Token::STRCON:
            ret = make_shared<ast::BasicLitNode>(pos_, tok_, lit_);
            Next();
            return ret;
        case token::Token::LPARENT:
            Next();
            ret = make_shared<ast::ParenExprNode>(pos_, ParseExpr());
            Expect(token::Token::RPARENT);
            return ret;
        default:
            Error(pos_, ec::Type::NotInHomeWork, "in operand, expect <int/char/idenfr/string/'('>");
            return make_shared<ast::BadExprNode>(pos_);
    }
}

// ParseCallExpr is called for parse function call expression.
// When Calling this function, tok_ should be '(' of function call.
// After Calling this function, tok_ will be ')' of function call.
// e.g.
//   start calling (arg1, arg2, arg3)  :=> tok_ is '('
//   end calling (arg1, arg2, arg3)    :=> tok_ is ')'
shared_ptr<ast::ExprNode> Parser::ParseCallExpr(const shared_ptr<ast::ExprNode>& func_name) {
    auto func_call_expr_node = make_shared<ast::CallExprNode>(pos_);
    Expect(token::Token::LPARENT);

    func_call_expr_node->fun_ = func_name;

    while (tok_ != token::Token::RPARENT) {
        func_call_expr_node->args_.push_back(ParseExpr());

        if (tok_ != token::Token::COMMA) {
            break;
        }
        Next();
    }

    Expect(token::Token::RPARENT);

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
    Expect(token::Token::LBRACK);

    vector<shared_ptr<ast::ExprNode>> index_expr_nodes;
    while (true) {
        index_expr_nodes.push_back(ParseExpr());
        Expect(token::Token::RBRACK);

        if (tok_ != token::Token::LBRACK) {
            break;
        }
        Next();
    }

    shared_ptr<ast::ExprNode> current_node = array_name;
    for (auto& index_expr_node : index_expr_nodes) {
        current_node = make_shared<ast::IndexExprNode>(current_node->Pos(), current_node, index_expr_node);
    }

    return current_node;
}

// Report all parse errors.
void Parser::ReportErrors() {
    cerr << errors_->ToString() << endl;
}
