#include <iostream>
#include <fstream>
#include <sstream>

#include "scanner/scanner.h"
#include "parser/parser.h"

using namespace std;

class StdErrHandler: public ErrorHandler{
public:
    ~StdErrHandler() override = default;
    void Report(const token::Position& pos, const string& msg) override {
        cerr << "(" << pos.line << ", " << pos.column << ") :=> " << msg << endl;
    }
};

string GetInputFile(const string& filename) {
    ifstream fin;
    fin.open(filename, ios::in);
    if (!fin.is_open()) {
        cout << "input file not found!" << endl;
        exit(EXIT_FAILURE);
    }

    stringstream buf;
    buf << fin.rdbuf();
    string raw_input = buf.str();

    fin.close();

    return raw_input;
}

/**
 * @brief LexicalAnalysisMain is main function for first lab.
 */
void LexicalAnalysisMain() {
    auto test_file = make_shared<token::File>();
    test_file->name = "testfile.txt";
    string txt = GetInputFile(test_file->name);
    test_file->size = (int)txt.size();

    auto err_handler = make_shared<StdErrHandler>();
    Scanner scanner(test_file, txt, err_handler);

    ofstream f_out;
    f_out.open("output.txt", ios::out);

    while (true) {
        int pos;
        token::Token tok;
        string lit;
        scanner.Scan(&pos, &tok, &lit);

        if (tok == token::Token::END_OF_FILE) {
            break;
        }

        string token_name = GetTokenName(tok);
        if (tok == token::Token::STRCON || tok == token::Token::CHARCON) {
            f_out << token_name << " " << lit.substr(1, lit.size() - 2) << endl;
            continue;
        }

        f_out << token_name << " " << lit << endl;
    }

    f_out.close();
}

void ParsingMain() {
    auto test_file = make_shared<token::File>();
    test_file->name = "testfile.txt";
    string txt = GetInputFile(test_file->name);
    test_file->size = (int)txt.size();

    auto err_handler = make_shared<StdErrHandler>();

    Parser parser(test_file, txt, err_handler);
    auto ast_file = parser.Parse();

    // cout << ast_file.ToString() << endl;

    //parser.ReportErrors();
}

void PostParsing() {
    std::ifstream fin("output.txt");
    vector<string> file_lines;

    cin.clear();

    string line;
    while(getline(fin, line)) {
        file_lines.push_back(line);
    }

    fin.close();

    // 过滤, 获取函数定义部分.
    vector<vector<string>> defs;
    vector<string> cur_def;
    for (const string& line: file_lines) {
        cur_def.push_back(line);
        // 函数定义开始
        if (line.find("MAINTK") != string::npos || line.find("声明头部") != string::npos || line.find("参数表") != string::npos) {
            defs.push_back(cur_def);
            cur_def.clear();
        }
    }
    defs.push_back(cur_def);

    freopen("/dev/tty", "w", stdout);

    // 过滤, 对于每个部分，从后向前，只留一个常量说明&变量说明.
    for (auto& def: defs) {
        cout << ">>>>>>>>>>>>>>>>>>重新扫描段落: " << def.size() << endl;
        for (auto& line: def) cout << line << endl;
        while(true) {
            bool do_delete = false;

            // 找常量删除.
            bool is_first_const_decl = true;
            for (int index = def.size() - 1; index >= 0; index --) {
                if (def[index].find("常量说明") != string::npos) {
                    if (is_first_const_decl) {
                        is_first_const_decl = false;
                        cout << "第一个常量说明在: " << index << endl;
                    } else {
                        cout << "删除常量说明: " << index << endl;
                        do_delete = true;
                        def.erase(def.begin() + index);
                    }
                }
            }

            bool is_first_var_decl = true;
            for (int index = def.size() - 1; index >= 0; index --) {
                if (def[index].find("变量说明") != string::npos) {
                    if (is_first_var_decl) {
                        is_first_var_decl = false;
                    } else {
                        do_delete = true;
                        def.erase(def.begin() + index);
                    }
                }
            }

            if (!do_delete) {
                break;
            }
        }
    }

    ofstream myout("output.txt");

    for (auto& def: defs) {
        for (auto& line: def) {
            myout << line << endl;
        }
    }

    myout.close();

}

int main() {
    freopen("output.txt", "w", stdout);
    ParsingMain();
    PostParsing();
    return 0;
}
