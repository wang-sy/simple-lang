#include "check/check.h"

namespace check {
Checker::Checker(const shared_ptr<ast::FileNode>& ast_file, const shared_ptr<ec::ErrorReminder>& error_reminder)
    : errors_(error_reminder) {}

void Checker::Check() {
    
}
}