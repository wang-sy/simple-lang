#include <queue>

#include "check/check.h"
#include "token/position.h"

using namespace std;

namespace check {
Checker::Checker(const shared_ptr<ast::FileNode>& file_node, const shared_ptr<ec::ErrorReminder>& error_reminder)
    : ast_(file_node), errors_(error_reminder) {
    var_table_ = make_shared<VarTable>();
}

shared_ptr<ast::TypeNode> CreateBasicTypeNodeByNodeType(const ast::NodeType& typ) {
    if (typ == ast::IntType) {
        return make_shared<ast::IntTypeNode>();
    } else if (typ == ast::CharType) {
        return make_shared<ast::CharTypeNode>();
    } else if (typ == ast::StringType) {
        return make_shared<ast::StringTypeNode>();
    }

    return make_shared<ast::BadTypeNode>();
}

void Checker::Check() {
    for (const auto& decl: ast_->decl_) {
        if (decl->Type() == ast::VarDecl) {
            CheckVarDeclNode(dynamic_pointer_cast<ast::VarDeclNode>(decl));
        } else if (decl->Type() == ast::FuncDecl) {
            CheckFuncDeclNode(dynamic_pointer_cast<ast::FuncDeclNode>(decl));
        } else {
            errors_->Emplace(decl->Pos(), ec::NotInHomeWork, "for root decl, expect var or func decl");
            return;
        }
    }
}


/**
 * @brief CheckVarDeclNode check var decl node in any position.
 * 
 * @param decl var decl node. 
 */
void Checker::CheckVarDeclNode(const shared_ptr<ast::VarDeclNode>& decl) {
    if (decl == nullptr || decl->Type() != ast::VarDecl) {
        errors_->Emplace(token::npos, ec::Type::NotInHomeWork, "CheckVarDeclNode: decl is nullptr or not var decl node.");
        return;
    }

    for (auto& single_decl: decl->decls_) {
        if (single_decl->Type() == ast::SingleVarDecl) {
            CheckSingleVarDeclNode(dynamic_pointer_cast<ast::SingleVarDeclNode>(single_decl));
        } else {
            errors_->Emplace(single_decl->Pos(), ec::NotInHomeWork, "for var decl, expect single var decl");
        }
    }
}

/**
 * @brief CheckSingleVarDeclNode check single var decl node.
 *
 * @param decl single var decl node.
 */
void Checker::CheckSingleVarDeclNode(const shared_ptr<ast::SingleVarDeclNode> &decl) {
    if (decl == nullptr || decl->Type() != ast::SingleVarDecl) {
        errors_->Emplace(token::npos, ec::Type::NotInHomeWork, "CheckSingleVarDeclNode: decl is nullptr or not single var decl node.");
        return;
    }

    switch (decl->type_->Type()) {
        case ast::IntType:
        case ast::CharType:
            CheckBasicVarDeclNode(decl);
            return;
        case ast::ArrayType:
            CheckArrayVarDeclNode(decl);
            return;
        default:
            errors_->Emplace(decl->Pos(), ec::NotInHomeWork, "for single var decl, expect basic type or array type");
            return;
    }
}


/**
 * @brief CheckFuncDeclNode check func decl node in root.
 *
 * @param decl func decl node.
 */
void Checker::CheckFuncDeclNode(const shared_ptr<ast::FuncDeclNode>& decl) {
    if (decl == nullptr || decl->Type() != ast::FuncDecl) {
        errors_->Emplace(decl->Pos(), ec::Type::NotInHomeWork, "for funcdecl node, node type error");
        return;
    }

    if (decl->name_ == nullptr || var_table_->IsVarExistedInCurrentCodeBlock(decl->name_->name_)) {
        errors_->Emplace(decl->Pos(), ec::Type::Redefine, "for funcdecl, func name already defined");
        return;
    }

    var_table_->AddFunc(decl->name_->name_, decl);
    var_table_->CreateCodeBlock();

    // check func params.
    if (decl->params_ == nullptr || decl->params_->Type() != ast::FieldList) {
        errors_->Emplace(decl->Pos(), ec::Type::NotInHomeWork, "for funcdecl params, expect field_list");
        return;
    }

    for (auto& field: decl->params_->fields_) {
        if (field == nullptr || field->Type() != ast::Field) {
            errors_->Emplace(field->Pos(), ec::Type::NotInHomeWork, "for funcdecl field_list, expect field");
            return;
        }
        
        auto field_decl = dynamic_pointer_cast<ast::FieldNode>(field);
        if (field_decl->name_ == nullptr || var_table_->IsVarExistedInCurrentCodeBlock(field_decl->name_->name_)) {
            errors_->Emplace(field_decl->Pos(), ec::Type::Redefine, "for funcdecl field, var name already defined");
            return;
        }
        var_table_->AddVar(field_decl->name_->name_, CreateBasicTypeNodeByNodeType(field_decl->type_->Type()));
    }

    // check body.
    if (decl->body_ == nullptr || decl->body_->Type() != ast::BlockStmt) {
        errors_->Emplace(decl->Pos(), ec::Type::NotInHomeWork, "for funcdecl body, expect block_stmt");
        return;
    }
    auto block_stmt_body = dynamic_pointer_cast<ast::BlockStmtNode>(decl->body_);
    
    // return type check.
    bool have_return = false;
    for (const auto& stmt_node:block_stmt_body->stmts_) {
        if(stmt_node == nullptr || stmt_node->Type() != ast::ReturnStmt) {
            continue;
        }

        auto return_stmt = dynamic_pointer_cast<ast::ReturnStmtNode>(stmt_node);
        if (return_stmt->Type() != decl->type_->Type()) {
            errors_->Emplace(return_stmt->Pos(), ec::Type::NotInHomeWork, "for funcdecl return type, expect return type");
            return;
        }

        have_return = true;
    }

    if (decl->type_->Type() != ast::VoidType && !have_return) {
        cout << decl->type_->Type() << endl;
        errors_->Emplace(decl->Pos(), ec::Type::NotInHomeWork, "for funcdecl return type, expect void or return stmt");
        return;
    }

    // check body.
    CheckBlockStmt(dynamic_pointer_cast<ast::BlockStmtNode>(decl->body_));

    var_table_->DestroyCodeBlock();
}

void Checker::CheckArrayVarDeclNode(const shared_ptr<ast::SingleVarDeclNode> &decl) {
    auto get_demissions_and_basic_token = [&](
        const shared_ptr<ast::TypeNode>& arr,
        vector<int>* demissions,
        shared_ptr<ast::TypeNode>* typ
    ) {
        shared_ptr<ast::TypeNode> cur_demission = arr;
        while(true) {
            if (cur_demission->Type() == ast::ArrayType) {
                auto cur_demission_arr = dynamic_pointer_cast<ast::ArrayTypeNode>(cur_demission);
                demissions->push_back(cur_demission_arr->size_);
                cur_demission = cur_demission_arr->item_;
            } else if (cur_demission->Type() == ast::BadType) {
                errors_->Emplace(decl->Pos(), ec::NotInHomeWork, "for array var decl, expect array type");
                return;
            } else {
                *typ = CreateBasicTypeNodeByNodeType(cur_demission->Type());
                break;
            }
        }
    };

    if (decl == nullptr || decl->Type() != ast::SingleVarDecl) {
        errors_->Emplace(token::npos, ec::Type::NotInHomeWork, "CheckArrayVarDeclNode: decl is nullptr or not single var decl node.");
        return;
    }

    if (decl->type_->Type() != ast::ArrayType) {
        errors_->Emplace(decl->Pos(), ec::NotInHomeWork, "for array var decl, expect array type");
        return;
    }

    // check var name is not duplicate in current code block.
    auto decl_name = dynamic_pointer_cast<ast::IdentNode>(decl->name_);
    if (var_table_->IsVarExistedInCurrentCodeBlock(decl_name->name_)) {
        errors_->Emplace(decl->name_->Pos(), ec::Redefine, "in single var decl, var name is duplicate");
        return;
    }
    var_table_->AddVar(decl_name->name_, decl->type_);

    if (decl->val_ == nullptr) {
        return;
    }

    if (decl->val_->Type() != ast::CompositeLit) {
        errors_->Emplace(decl->val_->Pos(), ec::NotInHomeWork, "for array var decl, expect array val");
        return;
    }

    // check decl val.
    shared_ptr<ast::TypeNode> composite_lit_type;
    CheckCompositeLitNodeAndGetType(dynamic_pointer_cast<ast::CompositeLitNode>(decl->val_), &composite_lit_type);

    // check if composite lit is equal to array type.
    if (composite_lit_type->Type() != ast::ArrayType) {
        errors_->Emplace(decl->val_->Pos(), ec::CompositeLitSizeError, "for array var decl, expect array val");
        return;
    }

    vector<int> decl_demissions, composite_lit_demissions;
    shared_ptr<ast::TypeNode> decl_basic_token, composite_lit_basic_token;

    get_demissions_and_basic_token(decl->type_, &decl_demissions, &decl_basic_token);
    get_demissions_and_basic_token(composite_lit_type, &composite_lit_demissions, &composite_lit_basic_token);

    if (decl_basic_token->Type() != composite_lit_basic_token->Type()) {
        errors_->Emplace(decl->val_->Pos(), ec::CompositeLitSizeError, "for array var decl, decl basic type and composite lit type neq");
        return;
    }

    if (decl_demissions != composite_lit_demissions) {
        errors_->Emplace(
            decl->val_->Pos(),
            ec::CompositeLitSizeError,
            "for array var decl, decl demissions and composite lit demissions neq"
        );
        return;
    }
}

void Checker::CheckBasicVarDeclNode(const shared_ptr<ast::SingleVarDeclNode> &decl) {
    // check decl type.
    if (decl->type_->Type() != ast::IntType && decl->type_->Type() != ast::CharType) {
        errors_->Emplace(decl->type_->Pos(), ec::NotInHomeWork, "for basic var decl type, expect int or char type");
        return;
    }

    // check var name type.
    if (decl->name_->Type() != ast::Ident) {
        errors_->Emplace(decl->name_->Pos(), ec::NotInHomeWork, "for basic var decl name, expect identifier");
        return;
    }

    // check var name is not duplicate in current code block.
    auto decl_name = dynamic_pointer_cast<ast::IdentNode>(decl->name_);
    if (var_table_->IsVarExistedInCurrentCodeBlock(decl_name->name_)) {
        errors_->Emplace(decl->name_->Pos(), ec::Redefine, "in single var decl, var name is duplicate");
        return;
    }
    var_table_->AddVar(decl_name->name_, decl->type_);

    if (decl->val_ == nullptr) {
        return;
    }

    // check var init.
    auto init_lit_type = make_shared<ast::TypeNode>();
    CheckExprAndGetType(decl->val_, &init_lit_type);

    if (init_lit_type->Type() != decl->type_->Type()) {
        errors_->Emplace(decl->val_->Pos(), ec::Type::NotInHomeWork, "for single var decl init value, type not equal");
        return;
    }
}

void Checker::CheckExprAndGetType(const shared_ptr<ast::ExprNode> &expr, shared_ptr<ast::TypeNode> *typ) {
    if (expr == nullptr) {
        errors_->Emplace(token::npos, ec::Type::NotInHomeWork, "CheckExprAndGetType: expr is nullptr");
        return;
    }

    switch (expr->Type()) {
        case ast::Ident:
            CheckIdentExprNodeAndGetType(dynamic_pointer_cast<ast::IdentNode>(expr), typ);
            return;
        case ast::BasicLit:
            CheckBasicLitNodeAndGetType(dynamic_pointer_cast<ast::BasicLitNode>(expr), typ);
            return;
        case ast::CompositeLit:
            CheckCompositeLitNodeAndGetType(dynamic_pointer_cast<ast::CompositeLitNode>(expr), typ);
            return;
        case ast::IndexExpr:
            CheckIndexExprNodeAndGetType(dynamic_pointer_cast<ast::IndexExprNode>(expr), typ);
            return;
        case ast::CallExpr:
            CheckCallExprNodeAndGetType(dynamic_pointer_cast<ast::CallExprNode>(expr), typ);
            return;
        case ast::UnaryExpr:
            CheckUnaryExprNodeAndGetType(dynamic_pointer_cast<ast::UnaryExprNode>(expr), typ);
            return;
        case ast::BinaryExpr:
            CheckBinaryExprNodeAndGetType(dynamic_pointer_cast<ast::BinaryExprNode>(expr), typ);
            return;
        default:
            errors_->Emplace(expr->Pos(), ec::NotInHomeWork, "unknown expr type");
            return;
    }
}


void Checker::CheckIdentExprNodeAndGetType(const shared_ptr<ast::IdentNode> &expr, shared_ptr<ast::TypeNode> *typ) {
    if (expr == nullptr) {
        errors_->Emplace(token::npos, ec::Type::NotInHomeWork, "CheckIdentExprNodeAndGetType: expr is nullptr");
        return;
    }

    // check if ident existed.
    VarTable::Identifier ident;
    int ret = var_table_->GetVar(expr->name_, &ident);
    if (ret) {
        *typ = make_shared<ast::BadTypeNode>(expr->Pos());
        errors_->Emplace(expr->Pos(), ec::Undefine, "for ident expr, var not found");
        return;
    }

    *typ = ident.type;
}

void Checker::CheckBasicLitNodeAndGetType(const shared_ptr<ast::BasicLitNode> &expr, shared_ptr<ast::TypeNode> *typ) {
    *typ = make_shared<ast::BadTypeNode>(expr->Pos());
    if (expr == nullptr) {
        errors_->Emplace(token::npos, ec::Type::NotInHomeWork, "CheckBasicLitNodeAndGetType: expr is nullptr");
        return;
    }

    // check basic literal type.
    if (expr->tok_ != token::Token::INTCON && expr->tok_ != token::Token::CHARCON && expr->tok_ != token::Token::STRCON) {
        errors_->Emplace(expr->Pos(), ec::NotInHomeWork, "for basic literal, expect int, char or string type");
        return;
    }

    // FIXME: Required by homework, charlit and stringlit can't be empty.
    if ((expr->tok_ == token::Token::CHARCON || expr->tok_ == token::Token::STRCON) && expr->val_.empty()) {
        errors_->Emplace(expr->Pos(), ec::EmptyCharOrStringLit, "for <char/string> basic lit, expect not empty");
        return;
    }

    if (expr->tok_ == token::Token::INTCON) {
        *typ = make_shared<ast::IntTypeNode>(expr->Pos());
    } else if (expr->tok_ == token::Token::CHARCON) {
        *typ = make_shared<ast::CharTypeNode>(expr->Pos());
    } else if (expr->tok_ == token::Token::STRCON) {
        *typ = make_shared<ast::StringTypeNode>(expr->Pos());
    }
}

void Checker::CheckCompositeLitNodeAndGetType(const shared_ptr<ast::CompositeLitNode> &expr,
                                              shared_ptr<ast::TypeNode> *typ) {
    *typ = make_shared<ast::BadTypeNode>(expr->Pos());
    if (expr == nullptr) {
        errors_->Emplace(token::npos, ec::Type::NotInHomeWork, "CheckCompositeLitNodeAndGetType: expr is nullptr");
        return;
    }

    vector<shared_ptr<ast::ExprNode>> cur_demission_nodes, next_demission_nodes;
    cur_demission_nodes.push_back(expr);

    vector<int> demissions;
    pair<bool, token::Token> get_basic_type_token;
    while(!cur_demission_nodes.empty()) {
        auto cur_demission_first_node = cur_demission_nodes.front();
        shared_ptr<ast::CompositeLitNode> cur_demission_ref_composite_lit_node;
        shared_ptr<ast::BasicLitNode> cur_demission_ref_basic_lit_node;

        if (cur_demission_first_node->Type() == ast::CompositeLit) {
            goto CompositeLitDemissionCheck;
        } else if (cur_demission_first_node->Type() == ast::BasicLit)  {
            goto BasicLitDemissionCheck;
        } else {
            errors_->Emplace(
                cur_demission_first_node->Pos(),
                ec::Type::CompositeLitSizeError,
                "for composite literal item, type undefined"
            );
            return;
        }

// nodes in current demission are all array type.
CompositeLitDemissionCheck:
        cur_demission_ref_composite_lit_node = dynamic_pointer_cast<ast::CompositeLitNode>(cur_demission_first_node);
        demissions.push_back(cur_demission_ref_composite_lit_node->items_.size());

        for (const auto& node: cur_demission_nodes) {
            if (node->Type() != ast::CompositeLit) {
                errors_->Emplace(
                    node->Pos(),
                    ec::Type::CompositeLitSizeError,
                    "for composite literal item, type error"
                );
                return;
            }

            auto composite_lit_node = dynamic_pointer_cast<ast::CompositeLitNode>(node);
            if (composite_lit_node->items_.size() != cur_demission_ref_composite_lit_node->items_.size()) {
                errors_->Emplace(
                    node->Pos(),
                    ec::Type::CompositeLitSizeError,
                    "for composite literal item, size not equal"
                );
                return;
            }

            // correct size and type, add items.
            for (const auto& item: composite_lit_node->items_) {
                next_demission_nodes.push_back(item);
            }
        }

        goto PostCheckProcess;

// nodes in current demission have same tok_.
BasicLitDemissionCheck:
        cur_demission_ref_basic_lit_node = dynamic_pointer_cast<ast::BasicLitNode>(cur_demission_first_node);
        for(const auto& node: cur_demission_nodes) {
            if (node->Type() != ast::BasicLit) {
                errors_->Emplace(
                    node->Pos(),
                    ec::Type::CompositeLitSizeError,
                    "for composite literal item, type error"
                );
                return;
            }

            auto basic_lit_node = dynamic_pointer_cast<ast::BasicLitNode>(node);
            if (basic_lit_node->tok_ != cur_demission_ref_basic_lit_node->tok_) {
                errors_->Emplace(
                    node->Pos(),
                    ec::Type::CompositeLitSizeError,
                    "for composite literal item, token's type error"
                );
                return;
            }
        }
        get_basic_type_token = make_pair(true, cur_demission_ref_basic_lit_node->tok_);

        goto PostCheckProcess;

PostCheckProcess:
        cur_demission_nodes = next_demission_nodes;
        next_demission_nodes.clear();
    }

    if (!get_basic_type_token.first) {
        errors_->Emplace(
            expr->Pos(),
            ec::Type::NotInHomeWork,
            "for composite literal, basic lit type not found"
        );
        return;
    }

    // write back type.
    if (get_basic_type_token.second == token::INTCON) *typ = make_shared<ast::IntTypeNode>(expr->Pos());
    else if (get_basic_type_token.second == token::CHARCON) *typ = make_shared<ast::CharTypeNode>(expr->Pos());
    else if (get_basic_type_token.second == token::STRCON) *typ = make_shared<ast::StringTypeNode>(expr->Pos());
    else *typ = make_shared<ast::BadTypeNode>(expr->Pos());

    for (int i = demissions.size() - 1; i >= 0; i--) {
        *typ= make_shared<ast::ArrayTypeNode>(expr->Pos(), demissions.at(i), *typ);
    }
}


/**
 * @brief CheckStmt check stmt node.
 * 
 * @param stmt stmt node.
 */
void Checker::CheckStmt(const shared_ptr<ast::StmtNode>& stmt) {
    if (stmt == nullptr) {
        errors_->Emplace(token::npos, ec::Type::NotInHomeWork, "CheckStmt: stmt is nullptr");
        return;
    }

    switch (stmt->Type()) {
        case ast::DeclStmt:
            CheckDeclStmt(dynamic_pointer_cast<ast::DeclStmtNode>(stmt));
            return;
        case ast::ExprStmt:
            CheckExprStmt(dynamic_pointer_cast<ast::ExprStmtNode>(stmt));
            return;
        case ast::AssignStmt:
            CheckAssignStmt(dynamic_pointer_cast<ast::AssignStmtNode>(stmt));
            return;
        case ast::ReturnStmt:
            CheckReturnStmt(dynamic_pointer_cast<ast::ReturnStmtNode>(stmt));
            return;
        case ast::BlockStmt:
            var_table_->CreateCodeBlock();
            CheckBlockStmt(dynamic_pointer_cast<ast::BlockStmtNode>(stmt));
            var_table_->DestroyCodeBlock();
            return;
        case ast::IfStmt:
            CheckIfStmt(dynamic_pointer_cast<ast::IfStmtNode>(stmt));
            return;
        case ast::SwitchStmt:
            CheckSwitchStmt(dynamic_pointer_cast<ast::SwitchStmtNode>(stmt));
            return;
        case ast::ForStmt:
            CheckForStmt(dynamic_pointer_cast<ast::ForStmtNode>(stmt));
            return;
        case ast::WhileStmt:
            CheckWhileStmt(dynamic_pointer_cast<ast::WhileStmtNode>(stmt));
            return;
        case ast::ScanStmt:
            CheckScanStmt(dynamic_pointer_cast<ast::ScanStmtNode>(stmt));
            return;
        case ast::PrintfStmt:
            CheckPrintfStmt(dynamic_pointer_cast<ast::PrintfStmtNode>(stmt));
            return;
        default:
            errors_->Emplace(stmt->Pos(), ec::Type::NotInHomeWork, "CheckStmt: stmt type error");
            return;
    }
}

/**
 * @brief CheckDeclStmt check decl stmt node.
 * 
 * @param decl_stmt decl stmt node.
 */
void Checker::CheckDeclStmt(const shared_ptr<ast::DeclStmtNode>& decl_stmt) {

}

/**
 * @brief CheckExprStmt check expr stmt node.
 * 
 * @param expr_stmt expr stmt node.
 */
void Checker::CheckExprStmt(const shared_ptr<ast::ExprStmtNode>& expr_stmt) {

}

/**
 * @brief CheckAssignStmt check assign stmt node.
 * 
 * @param assign_stmt assign stmt node.
 */
void Checker::CheckAssignStmt(const shared_ptr<ast::AssignStmtNode>& assign_stmt) {

}

/**
 * @brief CheckReturnStmt check return stmt node.
 * 
 * @param return_stmt return stmt node.
 */
void Checker::CheckReturnStmt(const shared_ptr<ast::ReturnStmtNode>& return_stmt) {

}

/**
 * @brief CheckBlockStmt check block stmt node.
 * 
 * @param block_stmt block stmt node.
 */
void Checker::CheckBlockStmt(const shared_ptr<ast::BlockStmtNode>& block_stmt) {

}

/**
 * @brief CheckIfStmt check if stmt node.
 * 
 * @param if_stmt if stmt node.
 */
void Checker::CheckIfStmt(const shared_ptr<ast::IfStmtNode>& if_stmt) {

}

/**
 * @brief CheckSwitchStmt check switch stmt node.
 * 
 * @param switch_stmt switch stmt node.
 */
void Checker::CheckSwitchStmt(const shared_ptr<ast::SwitchStmtNode>& switch_stmt) {

}

/**
 * @brief CheckForStmt check for stmt node.
 * 
 * @param for_stmt for stmt node.
 */
void Checker::CheckForStmt(const shared_ptr<ast::ForStmtNode>& for_stmt) {

}

/**
 * @brief CheckWhileStmt check while stmt node.
 * 
 * @param while_stmt while stmt node.
 */
void Checker::CheckWhileStmt(const shared_ptr<ast::WhileStmtNode>& while_stmt) {

}

/**
 * @brief CheckScanStmt check scan stmt node.
 * 
 * @param scan_stmt scan stmt node.
 */
void Checker::CheckScanStmt(const shared_ptr<ast::ScanStmtNode>& scan_stmt) {

}

/**
 * @brief CheckPrintfStmt check printf stmt node.
 * 
 * @param printf_stmt printf stmt node.
 */
void Checker::CheckPrintfStmt(const shared_ptr<ast::PrintfStmtNode>& printf_stmt) {

}

void Checker::CheckIndexExprNodeAndGetType(const shared_ptr<ast::IndexExprNode> &expr, shared_ptr<ast::TypeNode> *typ) {

}

void Checker::CheckCallExprNodeAndGetType(const shared_ptr<ast::CallExprNode> &expr, shared_ptr<ast::TypeNode> *typ) {

}

void Checker::CheckUnaryExprNodeAndGetType(const shared_ptr<ast::UnaryExprNode> &expr, shared_ptr<ast::TypeNode> *typ) {

}

/**
 * @brief CheckBinaryExprNodeAndGetType check binary expr node and get type.
 * @param expr binary expr node.
 * @param typ type of binary expr node.
 */
void Checker::CheckBinaryExprNodeAndGetType(const shared_ptr<ast::BinaryExprNode> &expr,
                                            shared_ptr<ast::TypeNode> *typ) {

}


}