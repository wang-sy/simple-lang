#pragma

#include "ast/define.h"

#include <memory>

using namespace std;

namespace ast {

// FileNode represents a Go source file.
class FileNode {
public:
    shared_ptr <IdentNode> name_{};
    vector <shared_ptr<DeclNode>> decl_{};
    // map<string, Node> scope_;
};

}