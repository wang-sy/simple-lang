#include "token.h"
#include <string>
#include <map>

using namespace std;

const static map<Token, string> token_names{
    {ILLEGAL,"ILLEGAL"}, // 无效token

    {IDENFR,"IDENFR"},  // 标识符
    {INTCON,"INTCON"},  // 整形常量
    {CHARCON,"CHARCON"}, // 字符常量
    {STRCON,"STRCON"},  // 字符串

    // keywords.
    {CONSTTK, "CONSTTK"},   // const
    {INTTK, "INTTK"},     // int
    {CHARTK, "CHARTK"},    // char
    {VOIDTK, "VOIDTK"},    // void
    {MAINTK, "MAINTK"},    // main
    {IFTK, "IFTK"},      // if
    {ELSETK, "ELSETK"},    // else
    {SWITCHTK, "SWITCHTK"},  // switch
    {CASETK, "CASETK"},    // case
    {DEFAULTTK, "DEFAULTTK"}, // default
    {WHILETK, "WHILETK"},   // while
    {FORTK, "FORTK"},     // for
    {SCANFTK, "SCANFTK"},   // scanf
    {PRINTFTK, "PRINTFTK"},  // printf
    {RETURNTK, "RETURNTK"},  // return

    // Operators and delimiters
    {PLUS, "PLUS"},    // +
    {MINU, "MINU"},    // -
    {MULT, "MULT"},    // *
    {DIV, "DIV"},     // /
    {LSS, "LSS"},     // <
    {LEQ, "LEQ"},     // <=
    {GRE, "GRE"},     // >
    {GEQ, "GEQ"},     // >=
    {EQL, "EQL"},     // ==
    {NEQ, "NEQ"},     // !=
    {COLON, "COLON"},   // :
    {ASSIGN, "ASSIGN"},  // =
    {SEMICN, "SEMICN"},  // ;
    {COMMA, "COMMA"},   // ,
    {LPARENT, "LPARENT"}, // (
    {RPARENT, "RPARENT"}, // )
    {LBRACK, "LBRACK"},  // [
    {RBRACK, "RBRACK"},  // ]
    {LBRACE, "LBRACE"},  // {
    {RBRACE, "RBRACE"},  // }

    {END_OF_FILE, "END_OF_FILE"}, // end of file.
};

const static map<Token, string> tokens {
    {ILLEGAL,"ILLEGAL"}, // 无效token

	//literal
	{IDENFR,  "IDENFR"},
	{INTCON,  "INTCON"},
	{CHARCON, "CHARCON"},
	{STRCON,  "STRCON"},

	// keywords.
	{CONSTTK,   "const"},
	{INTTK,     "int"},
	{CHARTK,    "char"},
	{VOIDTK,    "void"},
	{MAINTK,    "main"},
	{IFTK,      "if"},
	{ELSETK,    "else"},
	{SWITCHTK,  "switch"},
	{CASETK,    "case"},
	{DEFAULTTK, "default"},
	{WHILETK,   "while"},
	{FORTK,     "for"},
	{SCANFTK,   "scanf"},
	{PRINTFTK,  "printf"},
	{RETURNTK,  "return"},

	// operators.
	{PLUS,    "+"},
	{MINU,    "-"},
	{MULT,    "*"},
	{DIV,     "/"},
	{LSS,     "<"},
	{LEQ,     "<="},
	{GRE,     ">"},
	{GEQ,     ">="},
	{EQL,     "=="},
	{NEQ,     "!="},
	{COLON,   ":"},
	{ASSIGN,  "="},
	{SEMICN,  ";"},
	{COMMA,   ","},
	{LPARENT, "("},
	{RPARENT, ")"},
	{LBRACK,  "["},
	{RBRACK,  "]"},
	{LBRACE,  "{"},
	{RBRACE,  "}"},

	// EOF is end of file.
	{END_OF_FILE, "EOF"},
};

const static map<string, Token> key_words {
    {"const", CONSTTK},
    {"int", INTTK},
    {"char", CHARTK},
    {"void", VOIDTK},
    {"main", MAINTK},
    {"if", IFTK},
    {"else", ELSETK},
    {"switch", SWITCHTK},
    {"case", CASETK},
    {"default", DEFAULTTK},
    {"while", WHILETK},
    {"for", FORTK},
    {"scanf", SCANFTK},
    {"printf", PRINTFTK},
    {"return", RETURNTK},
};

/**
 * @brief GetTokenName get token's raw name.
 * @param tok token enum
 * @param name token's name
 * @return true for found, false for notfound
 */
string GetTokenName(const Token& tok) {
    auto get_token_name_iter = token_names.find(tok);
    if (get_token_name_iter == token_names.end()) {
        return "";
    }

    return get_token_name_iter->second;
}

/**
 * Lookup maps an identifier to its keyword token or IDENT (if not a keyword)
 * @param ident identifier
 * @return identifier's keyword token
 */
Token LookUp(const string& ident) {
    auto get_keyword_token_iter = key_words.find(ident);
    if (get_keyword_token_iter == key_words.end()) {
        return Token::IDENFR;
    }

    return get_keyword_token_iter->second;
}
