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
    map<string, ast::FuncDeclNode> funcs_;
    map<string, ast::SingleVarDeclNode> vars_;
    shared_ptr<ec::ErrorReminder> errors_;
};
}