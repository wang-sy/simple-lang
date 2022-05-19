#include "check/check.h"

namespace check {
Checker::Checker(const shared_ptr<ast::FileNode>& file_node, const shared_ptr<ec::ErrorReminder>& error_reminder)
    : ast_(file_node), errors_(error_reminder) {}

void Checker::Check() {
    for (const auto& decl: ast_->decl_) {

    }
}

/**
 * @brief CheckRootDecl check decl in root.
 * 
 * @param decl ast decl node, expect var decl / func decl. 
 */
void Checker::CheckRootDecl(const shared_ptr<ast::DeclNode>& decl) {
    switch (decl->Type()) {
        case ast::VarDecl:
            CheckVarDeclNode(dynamic_pointer_cast<ast::VarDeclNode>(decl));
            return;
        case ast::FuncDecl:
            CheckFuncDeclNode(dynamic_pointer_cast<ast::FuncDeclNode>(decl));
        default:
            return;
    }
}


/**
 * @brief CheckVarDeclNode check var decl node in any position.
 * 
 * @param decl var decl node. 
 */
void Checker::CheckVarDeclNode(const shared_ptr<ast::VarDeclNode>& decl) {

}

/**
 * @brief CheckFuncDeclNode check func decl node in root.
 * 
 * @param decl func decl node.
 */
void Checker::CheckFuncDeclNode(const shared_ptr<ast::FuncDeclNode>& decl) {

}
}