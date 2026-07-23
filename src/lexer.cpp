#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

/*
token types:
keywords: fn, let, int . . .
identifiers: x, result, factorial, main, . . .
literals: 1, 5, 100
operators: +, -, *, lte, gte, NOT, OR . . .
symbols: (, ), {, }, :, ; . . .
*/

enum class TokenType{
    //keywords
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

    //identi and literal
    IDENT,
    INTEGER_LITERAL,
    //bool literal is not needed since its just t/f

    //comparitors
    IS,
    IS_LT,
    IS_LTE,
    IS_GT,
    IS_GTE,

    //assign,ent
    ASSIGN,

    //symbols
    LEFT_PAREN,
    RIGHT_PAREN,
    LEFT_BRACE,
    RIGHT_BRACE,
    COLON,
    SEMICOLON,
    COMMA,
    ARROW,

    //special symbols
    END_OF_FILE,
    INVALID
};


struct Token {
    TokenType type;
    string value;
};

class LexicalAnalyzer{
private:
    string input;
    size_t position; //curr character

    bool isWhitespace(char c){
        return c == ' ' || c == '\t' || c == '\n' || c == '\r';
    }

    bool isAlpha(char c){
        return (c >= 'a' && c <= 'z') || (c >= 'A' || c <= 'Z');
    }

    bool isDigit(char c){
        return c >= '0' && c <= '9';
    }

    bool isAlphaNumeric{
        return isAlpha(c) || isDigit(c);
    }

    //get next identifier or keyword
    string getNextWord(){
        size_t start = position;
        while(position < input.length() && isAlphaNumeric(input[position])) position++;

        return input.substr(start, position-start);
    }


    //jkc only supports 64bit int, for now we will just keep it to consume decimals too
    //its redundant now but itll be useful later when we go to mvp+
    string getNextNumber(){
        size_t start = position;
        bool hasDecimal = false;
        while(position < input.length() && (isDigit(input[position]) || input[position] == '.')){
            if(input[position] == '.'){
                if(hasDecimal) break;
                hasDecimal = true;
            }
            position++;
        }
        return input.substr(start, position-start);
    }

    //todo: create unordered_map that detects the keyword and maps it to the token
    //up next is emplacing the tokens to the vector and processing the input
}




