#include <iostream>
#include <fstream>
#include <sstream>

#include "scanner/scanner.h"
#include "parser/parser.h"
#include "error.h"
#include "check/check.h"

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
    auto error_reporter = make_shared<ec::ErrorReminder>(true, cerr);

    Parser parser(test_file, txt, err_handler, error_reporter);
    auto ast_file = parser.Parse();

    cout << ast_file->ToString() << endl;

    parser.ReportErrors();
}

void ErrorMain() {
    auto test_file = make_shared<token::File>();
    test_file->name = "testfile.txt";
    string txt = GetInputFile(test_file->name);
    test_file->size = (int)txt.size();

    ofstream out_file("error.txt");

    auto err_handler = make_shared<StdErrHandler>();
    auto error_reporter = make_shared<ec::ErrorReminder>(true, out_file);

    Parser parser(test_file, txt, err_handler, error_reporter);

    check::Checker c(parser.Parse(), error_reporter);

    c.Check();

    out_file.close();

    ifstream pos_process_f_in("error.txt");

    string output_full_text;

    string pre_line = "asdfzxvoihcxhsdf";
    string line;
    while (getline(pos_process_f_in, line)) {
        if (line.substr(0, 2) == pre_line.substr(0, 2)) {
            continue;
        }

        output_full_text += line + "\n";
    }

    pos_process_f_in.close();

    ofstream pos_process_f_out("error.txt");
    pos_process_f_out << output_full_text;
    pos_process_f_out.close();
    
    // cout << ast_file.ToString() << endl;

    // parser.ReportErrors();
}

int main() {
    ErrorMain();
    return 0;
}
