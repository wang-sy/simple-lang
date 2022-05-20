#include "var_table.h"

VarTable::VarTable() {
    cur_unique_id_ = 0;
    code_block_ident_stack_.push(map<string, shared_ptr<Identifier>>());
}

VarTable::~VarTable() = default;

void VarTable::DestroyCodeBlock() {
    for (auto& ident : code_block_ident_stack_.top()) {
        id_to_ident_.erase(ident.second->unique_id);
        name_to_ident_.at(ident.first).pop();
    }
    code_block_ident_stack_.pop();
}

void VarTable::CreateCodeBlock() {
    code_block_ident_stack_.push(map<string, shared_ptr<Identifier>>());
}

void VarTable::AddVar(const string &name, const shared_ptr<ast::TypeNode> &type, bool is_const) {
    cur_unique_id_ ++;

    auto ident = make_shared<Identifier>(Identifier(cur_unique_id_, name, type, is_const));

    id_to_ident_[cur_unique_id_] = ident;
    name_to_ident_[name].push(ident);
    code_block_ident_stack_.top()[name] = ident;
}

int VarTable::GetVar(const string &name, VarTable::Identifier *ident) {
    auto get_idents_iter = name_to_ident_.find(name);
    if (get_idents_iter == name_to_ident_.end() || get_idents_iter->second.empty()) {
        return -1;
    }

    *ident = *(get_idents_iter->second.top());

    return 0;
}

bool VarTable::IsVarExistedInCurrentCodeBlock(const string &name) {
    auto current_code_block_idents = code_block_ident_stack_.top();

    return !(current_code_block_idents.find(name) == current_code_block_idents.end());
}

VarTable::Identifier::Identifier(int unique_id, const string &name, const shared_ptr<ast::TypeNode> &type,
                                 bool is_const) : unique_id(unique_id), name(name), type(type), is_const(is_const) {}

VarTable::Identifier::Identifier() = default;
