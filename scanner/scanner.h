#pragma once

#include <string>
#include <memory> // shared_ptr
#include "token/token.h"
#include "token/position.h"

using namespace std;

/**
 * @brief ErrorHandler report error.
 */
class ErrorHandler {
public:
    virtual ~ErrorHandler() = default;
    virtual void Report(const token::Position& pos, const string& msg) = 0;
};

/**
 * @brief Scanner scan given text and split it into tokens.
 */
class Scanner {
public:
    Scanner(const shared_ptr<token::File>& file, const string& src,const shared_ptr<ErrorHandler>& err);

    /**
     * @brief Scan the next token.
     * @param pos begin of the next token's literal
     * @token Token
     * @lit next token's literal
     */
    void Scan(int* pos, token::Token* tok, string* lit);



    int error_count{};

private:
    /**
     * @breif Read the next char into ch_.
     */
    void Next();

    /**
     * @brief SkipWhiteSpace skip chars like space, enter, tabs.
     */
    void SkipWhiteSpace();

    /**
     * @brief returns the next identifier.
     */
    string ScanIdentifier();

    /**
     * @brief Scan and returns the next Number.
     */
    string ScanNumber();

    /**
     * @brief Scan and returns the next identifier.
     */
    string ScanString();

    /**
     * @brief Scan and returns the next identifier.
     */
    string ScanChar();

    /**
     * @brief Error report error info.
     * @offs offset
     * @msg error message
     */
    void Error(int offs, const string& msg);

    // immutable state
    string src_;
    shared_ptr<token::File> file_;

    // scanning state
    char ch_{};
    int offset_{};
    int read_offset_{};

    // error handler.
    shared_ptr<ErrorHandler> err_;
};
