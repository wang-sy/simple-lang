#pragma once

#include <map>
#include <iostream>

#include "token/position.h"

// errcode, token name not found.

namespace ec {
enum Type {
    // char or string literal should not be empty.
    EmptyCharOrStringLit,
    // define same identifier twice.
    Redefine,
    // use undeclared identifier.
    Undefine,
    // in call expression, argv number not matched.
    ArgNumberNotMatched,
    // in call expression, argv type not matched.
    ArgTypeNotMatched,
    // in beq/leq or other logic expression, type not matched.
    CondValueNotMatched,
    // in void function, return value not allowed.
    ReturnValueNotAllowed,
    // in nonevoid function, return value is required.
    ReturnValueRequired,
    // for index expression, index type must be integer.
    IndexTypeNotAllowed,
    // const value shouldn't be changed.
    UpdateConstValue,
    // ';' is expected.
    SEMICNExpected,
    // ')' is expected.
    RPARENTExpected,
    // ']' is expected.
    RBRACKExpected,
    // composite literal size error, e.g. int a[2][3] = {{1,2,3}, {1,2}}.
    CompositeLitSizeError,
    // switch expression type and case type not matched.
    SwitchTypeError,
    // default case should be the last one.
    DefaultExpected,
    NotInHomeWork,
};

class Error {
public:
    Error() = default;
    Error(const token::Position& pos, Type type, const string& msg)
        : pos_(pos), type_(type), msg_(msg) {}

    Type type() const { return type_; }
    const token::Position& pos() const { return pos_; }
    const string& msg() const { return msg_; }
    const string ToString() const {
        // string res = to_string(pos_.line) + " ";
        // res.push_back('a' + type_);
        // return res;
        string res = "["; res.push_back('a' + type_); res += "]";
        res += " => " + pos_.ToString();
        res += " :: " + msg_;
        return res;
    }
public:
    token::Position pos_;
    Type type_;
    string msg_;
};

// ErrorReminder is a reminder of error.
class ErrorReminder {
public:
    ErrorReminder() = default;
    ErrorReminder(bool report_on_add_, ostream& out) : report_on_add_(report_on_add_), out_(out) {}
    string ToString() const {
        string result;
        for (const auto& error : errors_) {
            result += error.second.ToString() + "\n";
        }
        return result;
    }
    void Add(const token::Position& pos, const Error& err) {
        if (pos.offset == token::npos.offset) {
            npos_errors_.push_back(err);
        } else {
            errors_[pos] = err;
        }

        if (report_on_add_) {
            out_ << err.ToString() << std::endl;
        }
    }
    void Emplace(const token::Position& pos, const Type& typ, const string& msg) {
        Add(pos, Error(pos, typ, msg));
    }

public:
    std::map<token::Position, Error> errors_;
    std::vector<Error> npos_errors_;
    bool report_on_add_ = false;
    ostream& out_;
};
}