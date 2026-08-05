#pragma once

#include "token.hpp"
#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

class LexicalAnalyzer {
private:
    std::string input;
    std::size_t position;

    std::unordered_map<std::string, TokenType> keywords;
    std::unordered_map<char, TokenType> symbols;

    void initKeywords();
    void initSymbols();

    bool isWhitespace(char c);
    bool isAlpha(char c);
    bool isDigit(char c);
    bool isAlphaNumeric(char c);

    std::string getNextWord();
    std::string getNextNumber();

public:
    explicit LexicalAnalyzer(const std::string& source);

    std::vector<Token> tokenize();

    static std::string getTokenTypeName(TokenType type);
};

void printTokens(const std::vector<Token>& tokens);