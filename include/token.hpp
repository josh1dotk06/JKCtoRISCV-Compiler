#pragma once

#include <string>

enum class TokenType {
    FN,
    LET,
    INT,
    BOOL,
    IF,
    ELSE,
    THEN,
    WHILE,
    SEND,
    TRUE,
    FALSE,
    AND,
    OR,
    NOT,

    IDENT,
    INTEGER_LITERAL,

    PLUS,
    MINUS,
    TIMES,
    DIVIDE,

    IS,
    IS_LT,
    IS_LTE,
    IS_GT,
    IS_GTE,

    ASSIGN,

    LEFT_PAREN,
    RIGHT_PAREN,
    LEFT_BRACE,
    RIGHT_BRACE,
    COLON,
    SEMICOLON,
    COMMA,
    ARROW,

    END_OF_FILE,
    INVALID
};

struct Token {
    TokenType type;
    std::string value;

    Token(TokenType tokenType, const std::string& tokenValue)
        : type(tokenType), value(tokenValue) {}
};