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

    //operators
    PLUS,
    MINUS,
    TIMES,
    DIVIDE,

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
    Token(TokenType tokenType, const string& tokenValue)
        : type(tokenType), value(tokenValue) {
    }
};

class LexicalAnalyzer{
private:
    string input;
    size_t position; //curr character
    unordered_map<string, TokenType> keywords;
    unordered_map<char, TokenType> symbols;
    void initKeywords(){
        keywords["fn"] = TokenType::FN;
        keywords["let"] = TokenType::LET;
        keywords["int"] = TokenType::INT;
        keywords["bool"] = TokenType::BOOL;
        keywords["if"] = TokenType::IF;
        keywords["else"] = TokenType::ELSE;
        keywords["then"] = TokenType::THEN;
        keywords["while"] = TokenType::WHILE;
        keywords["send"] = TokenType::SEND;
        keywords["true"] = TokenType::TRUE;
        keywords["false"] = TokenType::FALSE;
        keywords["AND"] = TokenType::AND;
        keywords["OR"] = TokenType::OR;
        keywords["NOT"] = TokenType::NOT;
        keywords["is"] = TokenType::IS;
        keywords["is_lt"] = TokenType::IS_LT;
        keywords["is_lte"] = TokenType::IS_LTE;
        keywords["is_gt"] = TokenType::IS_GT;
        keywords["is_gte"] = TokenType::IS_GTE;
    }
    void initSymbols(){
        symbols['+'] = TokenType::PLUS;
        symbols['-'] = TokenType::MINUS;
        symbols['*'] = TokenType::TIMES;
        symbols['/'] = TokenType::DIVIDE;
        symbols['('] = TokenType::LEFT_PAREN;
        symbols[')'] = TokenType::RIGHT_PAREN;
        symbols['{'] = TokenType::LEFT_BRACE;
        symbols['}'] = TokenType::RIGHT_BRACE;
        symbols[';'] = TokenType::SEMICOLON;
        symbols[':'] = TokenType::COLON;
        symbols[','] = TokenType::COMMA;
        symbols['='] = TokenType::ASSIGN;
    }


    bool isWhitespace(char c){
        return c == ' ' || c == '\t' || c == '\n' || c == '\r';
    }

    bool isAlpha(char c){
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_');
    }

    bool isDigit(char c){
        return c >= '0' && c <= '9';
    }

    bool isAlphaNumeric(char c){
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

public:
    //constructor
    LexicalAnalyzer(const string& source): input(source), position(0){
        initKeywords();
        initSymbols();
    }
    //after lexer obj is created, input == source, and position == 0
    //the source is just the input program for us to compile
    //position points to first char

    vector<Token> tokenize(){
        //list of tokens for the program
        vector<Token> tokens;

        while(position < input.length()){
            char curr = input[position];

            if(isWhitespace(curr)){
                position++;
                continue;
            }

            if(curr == '/' && position+1 < input.length() && input[position+1] == '/'){
                position += 2;
                while(position < input.length() && input[position] != '\n') position++;
                continue;
            }
            

            //keyword/identifier checker
            if(isAlpha(curr)){
                string word = getNextWord();
                //check if it exists
                if(keywords.find(word) != keywords.end()) tokens.emplace_back(keywords[word], word);
                else tokens.emplace_back(TokenType::IDENT, word);
            }

            else if (curr == '-' &&
                position + 1 < input.length() &&
                input[position + 1] == '>') {

                tokens.emplace_back(TokenType::ARROW, "->");
                position += 2;
                continue;
            }

            //literal checker
            else if(isDigit(curr)){
                string number = getNextNumber();
                tokens.emplace_back(TokenType::INTEGER_LITERAL, number);
            }

            //symbol checker
            else if(curr == '+'
                    || curr == '-'
                    || curr == '*'
                    || curr == '/'
                    || curr == '('
                    || curr == ')'
                    || curr == '{'
                    || curr == '}'
                    || curr == ';'
                    || curr == ':'
                    || curr == ','
                    || curr == '='){
                //turn the char value into string
                tokens.emplace_back(symbols[curr], string(1,curr));
                position++;
            }

            else{
                tokens.emplace_back(TokenType::INVALID, string(1,curr));
                position++;
            }

        }
        //return the final token vector
        tokens.emplace_back(TokenType::END_OF_FILE, "");
        return tokens;
    }

    static string getTokenTypeName(TokenType type) {
        switch (type) {
            //keywords
            case TokenType::FN:
                return "FN";
            case TokenType::LET:
                return "LET";
            case TokenType::INT:
                return "INT";
            case TokenType::BOOL:
                return "BOOL";
            case TokenType::IF:
                return "IF";
            case TokenType::ELSE:
                return "ELSE";
            case TokenType::THEN:
                return "THEN";
            case TokenType::WHILE:
                return "WHILE";
            case TokenType::SEND:
                return "SEND";
            case TokenType::TRUE:
                return "TRUE";
            case TokenType::FALSE:
                return "FALSE";
            case TokenType::AND:
                return "AND";
            case TokenType::OR:
                return "OR";
            case TokenType::NOT:
                return "NOT";

            //identifiers and literals
            case TokenType::IDENT:
                return "IDENT";
            case TokenType::INTEGER_LITERAL:
                return "INTEGER_LITERAL";

            //arithmetic operators
            case TokenType::PLUS:
                return "PLUS";
            case TokenType::MINUS:
                return "MINUS";
            case TokenType::TIMES:
                return "TIMES";
            case TokenType::DIVIDE:
                return "DIVIDE";

            //comparators
            case TokenType::IS:
                return "IS";
            case TokenType::IS_LT:
                return "IS_LT";
            case TokenType::IS_LTE:
                return "IS_LTE";
            case TokenType::IS_GT:
                return "IS_GT";
            case TokenType::IS_GTE:
                return "IS_GTE";


            case TokenType::ASSIGN:
                return "ASSIGN";


            case TokenType::LEFT_PAREN:
                return "LEFT_PAREN";
            case TokenType::RIGHT_PAREN:
                return "RIGHT_PAREN";
            case TokenType::LEFT_BRACE:
                return "LEFT_BRACE";
            case TokenType::RIGHT_BRACE:
                return "RIGHT_BRACE";
            case TokenType::COLON:
                return "COLON";
            case TokenType::SEMICOLON:
                return "SEMICOLON";
            case TokenType::COMMA:
                return "COMMA";
            case TokenType::ARROW:
                return "ARROW";

            case TokenType::END_OF_FILE:
                return "END_OF_FILE";
            case TokenType::INVALID:
                return "INVALID";
        }

        return "UNKNOWN_TOKEN_TYPE";
    }
};


void printTokens(const vector<Token>& tokens)
{
    for (const auto& token : tokens) {
        cout << "Type: " << LexicalAnalyzer::getTokenTypeName(token.type)
             << ", Value: " << token.value << endl;
    }
}


    int main()
{

    string sourceCode
        = "int main() { float x = 3.14; float y=3.15; "
          "float z=x+y; return 0; }";


    LexicalAnalyzer lexer(sourceCode);
    
    vector<Token> tokens = lexer.tokenize();


    cout << "Source code: " << sourceCode << endl << endl;

    cout << "Tokens Generate by Lexical Analyzer:" << endl;
    printTokens(tokens);
    return 0;
}







