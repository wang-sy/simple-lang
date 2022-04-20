#pragma once

#include <string>

using namespace std;

enum Token {
    ILLEGAL, // 无效token

    literal_beg,
    IDENFR,  // 标识符
    INTCON,  // 整形常量
    CHARCON, // 字符常量
    STRCON,  // 字符串
    literal_end,

    // keywords.
    keyword_beg,
    CONSTTK,   // const
    INTTK,     // int
    CHARTK,    // char
    VOIDTK,    // void
    MAINTK,    // main
    IFTK,      // if
    ELSETK,    // else
    SWITCHTK,  // switch
    CASETK,    // case
    DEFAULTTK, // default
    WHILETK,   // while
    FORTK,     // for
    SCANFTK,   // scanf
    PRINTFTK,  // printf
    RETURNTK,  // return
    keyword_end,

    // Operators and delimiters
    operator_beg,
    PLUS,    // +
    MINU,    // -
    MULT,    // *
    DIV,     // /
    LSS,     // <
    LEQ,     // <=
    GRE,     // >
    GEQ,     // >=
    EQL,     // ==
    NEQ,     // !=
    COLON,   // :
    ASSIGN,  // =
    SEMICN,  // ;
    COMMA,   // ,
    LPARENT, // (
    RPARENT, // )
    LBRACK,  // [
    RBRACK,  // ]
    LBRACE,  // {
    RBRACE,  // }
    operator_end,

    END_OF_FILE // end of file.
};


/**
 * @brief GetTokenName get token's raw name.
 * @param tok token enum
 * @return string, token name
 */
extern string GetTokenName(const Token& tok);

/**
 * Lookup maps an identifier to its keyword token or IDENT (if not a keyword)
 * @param ident identifier
 * @return identifier's keyword token
 */
extern Token LookUp(const string& ident);

/**
 * IsLiteral returns true for tokens corresponding to identifiers and basic type literals;
 * it returns false otherwise.
 * @param tok token
 */
inline bool IsLiteral(const Token& tok) { return literal_beg < tok && tok < literal_end; }

/**
 * IsOperator returns true for tokens corresponding to operators anddelimiters;
 * it returns false otherwise.
 * @param tok token
 */
inline bool IsOperator(const Token& tok) { return operator_beg < tok && tok < operator_end; }

/**
 * IsKeyword returns true for tokens corresponding to keywords;
 * it returns false otherwise.
 * @param tok token
 */
inline bool IsKeyword(const Token& tok) { return keyword_beg < tok && tok < keyword_end; }
