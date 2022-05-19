#pragma once

#include "ast/ast.h"
#include "error.h"

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
     * @brief CheckRootDecl check decl in root.
     * 
     * @param decl ast decl node, expect var decl / func decl. 
     */
    void CheckRootDecl(const shared_ptr<ast::DeclNode>& decl);

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

    shared_ptr<ast::FileNode> ast_;

    map<string, ast::FuncDeclNode> funcs_;
    map<string, ast::SingleVarDeclNode> vars_;
    shared_ptr<ec::ErrorReminder> errors_;
};
}