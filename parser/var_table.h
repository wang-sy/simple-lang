#pragma once

#include "ast/ast.h"

#include <map>
#include <string>
#include <stack>

using namespace std;

// VarTable is a class used to store all variables.
class VarTable {
public:
    class Identifier {
    public:
        Identifier();
        Identifier(int unique_id, const string& name, const shared_ptr<ast::TypeNode>& type, bool is_const=false);
        int unique_id;
        string name;
        shared_ptr<ast::TypeNode> type;
        bool is_const;
    };
public:
    VarTable();
    ~VarTable();

    // CreateCodeBlock create a code block.
    void CreateCodeBlock();

    // DestroyCodeBlock destroy a code block.
    void DestroyCodeBlock();

    // AddVar add a variable to current code block.
    void AddVar(const string& name, const shared_ptr<ast::TypeNode>& type, bool is_const=false);

    // GetVar get a variable from current code block.
    int GetVar(const string& name, Identifier* ident);

    // IsVarExistInCurrentCodeBlock check if a variable is existed in current code block.
    bool IsVarExistedInCurrentCodeBlock(const string& name);
private:

    int cur_unique_id_;

    map<string, ast::FuncDeclNode> func_table_;

    map<string, stack<shared_ptr<Identifier>>> name_to_ident_;
    map<int, shared_ptr<Identifier>> id_to_ident_;

    stack<map<string, shared_ptr<Identifier>>> code_block_ident_stack_;
};