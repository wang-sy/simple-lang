#pragma once

#include "ast/ast.h"
#include "error.h"
#include "parser/var_table.h"

#include <map>
#include <vector>
#include <memory>


using namespace std;

namespace check{
// Checker check errors builded ast tree.
class Checker {
public:
    Checker(const shared_ptr<ast::FileNode>& ast_file, const shared_ptr<ec::ErrorReminder>& error_reminder);
    void Check();
private:
    /**
     * @brief CheckVarDeclNode check var decl node in any position.
     * 
     * @param decl var decl node. 
     */
    void CheckVarDeclNode(const shared_ptr<ast::VarDeclNode>& decl);

    /**
     * @brief CheckFuncDeclNode check func decl node in root.
     *
     * @param decl func decl node.
     */
    void CheckFuncDeclNode(const shared_ptr<ast::FuncDeclNode>& decl);

    /**
     * @brief CheckSingleVarDecl check single var decl node.
     *
     * @param decl single var decl node.
     */
    void CheckSingleVarDeclNode(const shared_ptr<ast::SingleVarDeclNode>& decl);

    /**
     * @brief CheckArrayVarDeclNode check array var decl node.
     *
     * @param decl array var decl node.
     */
    void CheckArrayVarDeclNode(const shared_ptr<ast::SingleVarDeclNode>& decl);

    /**
     * @brief CheckBasicVarDeclNode check basic var decl node.
     *
     * @param decl basic var decl node, e.g. int, char.
     */
    void CheckBasicVarDeclNode(const shared_ptr<ast::SingleVarDeclNode>& decl);

    /**
     * CheckExprAndGetType check expr node and get its type.
     * @param expr expr node.
     * @param typ type of expr node.
     */
    void CheckExprAndGetType(const shared_ptr<ast::ExprNode>& expr, shared_ptr<ast::TypeNode>* typ);

    /**
     * @brief CheckIdentNodeAndGetType check ident expr node and get its type.
     *
     * @param ident ident expr node.
     * @param typ type of ident expr node.
     */
    void CheckIdentExprNodeAndGetType(const shared_ptr<ast::IdentNode>& expr, shared_ptr<ast::TypeNode>* typ);

    /**
     * @brief CheckBasicLitNodeAndGetType check basic literal expr node.
     *
     * @param expr basic literal expr node.
     * @param typ type of basic literal expr node.
     */
    void CheckBasicLitNodeAndGetType(const shared_ptr<ast::BasicLitNode>& expr, shared_ptr<ast::TypeNode>* typ);

    /**
     * @brief CheckCompositeLitNodeAndGetType check composite literal expr node.
     *
     * @param expr composite literal expr node.
     * @param typ type of composite literal expr node.
     */
    void CheckCompositeLitNodeAndGetType(const shared_ptr<ast::CompositeLitNode>& expr, shared_ptr<ast::TypeNode>* typ);

    /**
     * @brief CheckIndexExprNodeAndGetType check index expr node and get its type.
     *
     * @param expr index expr node.
     * @param typ type of index expr node.
     */
    void CheckIndexExprNodeAndGetType(const shared_ptr<ast::IndexExprNode>& expr, shared_ptr<ast::TypeNode>* typ);

    /**
     * @brief CheckCallExprNodeAndGetType check call expr node and get its type.
     *
     * @param expr call expr node.
     * @param typ type of call expr node.
     */
    void CheckCallExprNodeAndGetType(const shared_ptr<ast::CallExprNode>& expr, shared_ptr<ast::TypeNode>* typ);

    /**
     * @brief CheckUnaryExprNodeAndGetType check unary expr node and get its type.
     *
     * @param expr unary expr node.
     * @param typ type of unary expr node.
     */
    void CheckUnaryExprNodeAndGetType(const shared_ptr<ast::UnaryExprNode>& expr, shared_ptr<ast::TypeNode>* typ);

    /**
     * @brief CheckBinaryExprNodeAndGetType check binary expr node and get its type.
     *
     * @param expr binary expr node.
     * @param typ type of binary expr node.
     */
    void CheckBinaryExprNodeAndGetType(const shared_ptr<ast::BinaryExprNode>& expr, shared_ptr<ast::TypeNode>* typ);

    shared_ptr<ast::FileNode> ast_;

    shared_ptr<VarTable> var_table_;
    shared_ptr<ec::ErrorReminder> errors_;
};
}