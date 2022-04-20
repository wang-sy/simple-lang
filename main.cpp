#include <iostream>
#include <fstream>
#include <sstream>

#include "scanner.h"

using namespace std;

class StdErrHandler: public ErrorHandler{
public:
    ~StdErrHandler() override = default;
    void Report(const Position& pos, const string& msg) override {
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
    auto test_file = make_shared<File>();
    test_file->name = "testfile.txt";
    string txt = GetInputFile(test_file->name);
    test_file->size = (int)txt.size();


    auto err_handler = make_shared<StdErrHandler>();
    Scanner scanner(test_file, txt, err_handler);

    ofstream f_out;
    f_out.open("output.txt", ios::out);

    while (true) {
        int pos;
        Token tok;
        string lit;
        scanner.Scan(&pos, &tok, &lit);

        if (tok == Token::END_OF_FILE) {
            break;
        }

        string token_name = GetTokenName(tok);
        if (tok == Token::STRCON || tok == Token::CHARCON) {
            f_out << token_name << " " << lit.substr(1, lit.size() - 2) << endl;
            continue;
        }

        f_out << token_name << " " << lit << endl;
    }

    f_out.close();
}

int main() {
    LexicalAnalysisMain();
    return 0;
}