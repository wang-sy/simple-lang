#include "scanner.h"

#include <iostream>

using namespace std;

// Helpers.

// IsLetter 判断是否是字符
inline bool IsLetter(const char& c)  {
    bool is_lower_letter = ('a' <= c && c <= 'z');
    bool is_upper_letter = ('A' <= c && c <= 'Z');

    return is_lower_letter || is_upper_letter || c == '_';
}

// IsDigit 判断是否是数字
inline bool IsDigit(const char& c) {
    return '0' <= c && c <= '9';
}


// Impl.
Scanner::Scanner(const shared_ptr<token::File> &file, const string &src, const shared_ptr<ErrorHandler> &err) {
    error_count = 0;

    src_.assign(src);
    file_ = file;

    ch_ = ' ';
    offset_ = 0;
    read_offset_ = 0;

    Next();
}

void Scanner::Scan(int *pos, token::Token *tok, string *lit) {
    SkipWhiteSpace();

    // current token start
    *pos = offset_;

    char ch = ch_;
    if (IsLetter(ch)) {
        lit->assign(ScanIdentifier());
        *tok = token::LookUp(*lit);
        return;
    }

    if (IsDigit(ch)) {
        lit->assign(ScanNumber());
        *tok = token::Token::INTCON;
        return;
    }

    Next(); // always make progress
    switch (ch) {
        case -1:
            *tok = token::Token::END_OF_FILE;
            break;
        case '"':
            *tok = token::Token::STRCON;
            lit->assign(ScanString());
            break;
        case '\'':
            *tok = token::Token::CHARCON;
            lit->assign(ScanChar());
            break;
        case ':':
            *tok = token::Token::COLON;
            lit->assign(":");
            break;
        case ',':
            *tok = token::Token::COMMA;
            lit->assign(",");
            break;
        case ';':
            *tok = token::Token::SEMICN;
            lit->assign(";");
            break;
        case '(':
            *tok = token::Token::LPARENT;
            lit->assign("(");
            break;
        case ')':
            *tok = token::Token::RPARENT;
            lit->assign(")");
            break;
        case '[':
            *tok = token::Token::LBRACK;
            lit->assign("[");
            break;
        case ']':
            *tok = token::Token::RBRACK;
            lit->assign("]");
            break;
        case '{':
            *tok = token::Token::LBRACE;
            lit->assign("{");
            break;
        case '}':
            *tok = token::Token::RBRACE;
            lit->assign("}");
            break;
        case '+':
            *tok = token::Token::PLUS;
            lit->assign("+");
            break;
        case '-':
            *tok = token::Token::MINU;
            lit->assign("-");
            break;
        case '*':
            *tok = token::Token::MULT;
            lit->assign("*");
            break;
        case '/':
            *tok = token::Token::DIV;
            lit->assign("/");
            break;
        case '<':
            if (ch_ == '=') {
                Next();
                *tok = token::Token::LEQ;
                lit->assign("<=");
            } else {
                *tok = token::Token::LSS;
                lit->assign("<");
            }
            break;
        case '>':
            if (ch_ == '=') {
                Next();
                *tok = token::Token::GEQ;
                lit->assign(">=");
            } else {
                *tok = token::Token::GRE;
                lit->assign(">");
            }
            break;
        case '=':
            if (ch_ == '=') {
                Next();
                *tok = token::Token::EQL;
                lit->assign("==");
            } else {
                *tok = token::Token::ASSIGN;
                lit->assign("=");
            }
            break;
        case '!':
            if (ch_ == '=') {
                Next();
                *tok = token::Token::NEQ;
                lit->assign("!=");
            } else {
                Error(*pos, "unknown token");
            }
            break;
        default:
            // next reports unexpected BOMs - don't repeat
            Error(*pos, "illegal character");
            *tok = token::Token::ILLEGAL;
            string new_lit;
            new_lit.push_back(ch);
            lit->assign(new_lit);
            break;
    }
}

void Scanner::Next() {
    if (read_offset_ >= src_.size()) {
        offset_ = (int)src_.size();
        if (ch_ == '\n') {
            file_->AddLine(offset_);
        }
        ch_ = -1;
        return;
    }

    offset_ = read_offset_;
    if (ch_ == '\n') {
        file_->AddLine(offset_);
    }

    char c = src_[read_offset_];
    if (c == 0) {
        Error(offset_, "illegal character NUL");
    }

    read_offset_ ++;
    ch_ = c;
}

void Scanner::Error(int offs, const string &msg) {
    auto pos = file_->GetPositionByOffset(offs);

    cerr << "pos (" << pos.line << ", " << pos.column << ")err: " << offs << " => " << msg << endl;
    //err_->Report(file_->GetPositionByOffset(offs), msg);
    error_count ++;
}

void Scanner::SkipWhiteSpace() {
    while (ch_ == ' ' || ch_ == '\t' || ch_ == '\n' || ch_ == '\r') {
        Next();
    }
}

string Scanner::ScanIdentifier() {
    int offs = offset_;
    while (IsLetter(ch_) || IsDigit(ch_)) {
        Next();
    }
    return src_.substr(offs, offset_ - offs);
}

string Scanner::ScanNumber() {
    int offs = offset_;

    while (IsDigit(ch_)) {
        Next();
    }

    return src_.substr(offs, offset_ - offs);
}

string Scanner::ScanString() {
    // '"' opening already consumed
    int offs = offset_ - 1;

    while(true) {
        char ch = ch_;
        if (ch == '\n' || ch < 0) {
            Error(offs, "string literal not terminated");
            break;
        }
        Next();
        if (ch == '"') {
            break;
        }
        if (ch == '\\') {
            // TODO(jaegerwang) :=> use scanEscape instead of just read a char.
            Next();
        }
    }

    return src_.substr(offs, offset_ - offs);
}

string Scanner::ScanChar() {
    // '\'' opening already consumed
    int offs = offset_ - 1;

    int n = 0;
    while (true) {
        char ch = ch_;
        if (ch == '\n' || ch < 0) {
            // only report error if we don't have one already
            Error(offs, "rune literal not terminated");
            break;
        }
        Next();
        if (ch == '\'') {
            break;
        }
        n++;
        if (ch == '\\') {
            // TODO(jaegerwang) :=> use scanEscape instead of just read a char.
            Next();
        }
    }

    if (n != 1) {
        Error(offs, "illegal rune literal");
    }

    return src_.substr(offs, offset_ - offs);
}
