
#include "ast.hpp"
#include "token.hpp"
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include <stdexcept>
#include <utility>

//parser basically = lexer, it stores string input, and position
//except this time it moves through the tokens rather than through the source code
class Parser{
private:
    const std::vector<Token>& tokens
    std::size_t position;

    //token functions prototype (declarations)
    //read next token object in input stream, direct reference to token (read only thus)
    //second const promises not to modify any member variables of the parser class
    //also member variables are just private fields (in java terms)
    const Token& peek() const;
    const Token& advance();

    bool check(Token type) const;
    bool checkNext(Token type) const;
    bool isAtEnd() const;

    Token consume(Tokentype expType, const std::string& errorMessage);

    //GRAMMAR PARSERS
    JKCType parseType();

    std::unique_ptr<Expr> parseExpression();
    std::unique_ptr<Expr> parsePrimary();
    std:unique_ptr<Stmt> parseLetStatement();
    std::unique_ptr<Stmt> parseAssignStatement();
    std::unique_ptr<Stmt> parseSendStatement();
    std::unique_ptr<Stmt> parseBlockStatement();
    std::unique_ptr<Stmt> parseIfStatement();
    std::unique_ptr<Stmt> parseWhileStatement();
    std::unique_ptr<Stmt> parseExprStatement();

public:
    //construct and intiialize member list (private fields)
    Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens), position(0) {}

    std::unique_ptr<Stmt> parseStatement();

};


//actual parser logic
//define functions (technically the parser class can be put in a header since it defvines
//the prototypes)

const Token& Parser::peek() const {
    return tokens.at(position);
}

//check curr type == queried type
bool Parser::check(TokenType type) const {
    return peek().type == type;
}

//needed for detecting assignment statement
bool Parser::checkNext(TokenType type) const {
    if(position+1 >= token.size()) return false;
    return tokens.at(position+1).type == type;
}

//check eof
bool Parser::isAtEnd() const {
    return check(TokenType::END_OF_FILE);
}


//return curr token and goto next token
//not const since changes a member (specifically position)
const Token& Parser::advance() {
    const Token& curr = peek();
    if(!isAtEnd()) position++;
    return curr;
}

//consume
//check if curr = type, if so then move past it, if not then return error

Token Parser::consume(Tokentype expType, const std::string& errorMessage){
    if(check(expType)){
        return advance();
    }
    
    //account for eof
    //.value was the actual character/string of the token
    //i.e, would be the variable name if it is an IDENT token
    std::string found = peek().value();
    if(found.empty()) found = "end of file";

    throw std::runtime_error(errorMessage + " Found: '"+ found + "'.");
}

//parse variable types
JKCType Parser::parseType(){
    if(check(TokenType::INT)){
        advance();
        return JKCType::Int;
    }
    else if(check(TokenType::BOOL)){
        advance();
        return JKCType::Bool;
    }

    throw std::runtime_error("expected either type int or bool");


}

//only have let currently, will ad more later
std::unique_ptr<Stmt> Parser::parseStatement(){
    if(check(TokenType::LET)) return parseLetStatement();
    else if(check(TokenType::SEND)) return parseSendStatement();
    else if(check(TokenType::WHILE)) return parseWhileStatement();
    else if(check(TokenType::IF)) return parseIfStatement();
    else if(check(TokenType::LEFT_BRACE)) return parseBlockStatement();
    else if(check(TokenType::IDENT) && checkNext(TokenType::ASSIGN)) return parseAssignStatement();

    //for expr stmt, this is the last possible option
    return parseExprStatement();
}


//statement parser
//LET GRAMMAR: "let" IDENT ":" Type "=" Expr ";"
std::unique_ptr<Stmt> Parser::parseLetStatement(){

    //grammatical order of LET stmt
    consume(TokenType::LET, "expected 'let' at start of variable declaration");
    Token varName = consume(TokenType::IDENT, "expected var IDENT name after 'let'");
    consume(TokenType::COLON, "expected ':' after var name");
    JKCType decType = parseType();
    consume(TokenType::ASSIGN, "expected '=' after var type");
    std::unique_ptr<Expr> initializer = parseExpression();
    consume(TokenType::SEMICOLON, "expected ';' at end of let stmt");

    //construct a new unique_ptr of letstmt (constructor in ast.hpp)
    //recall that a let stmt consists of these 3 elements
    return std::make_unique<LetStmt>(varName.value, decType, std::move(initializer));
}

//SendStmt     → "send" Expr ";"
std::unique_ptr<Stmt> Parser::parseSendStatement(){
    consume(TokenType::SEND, "expected 'send' at start of send statement");
    std::unique_ptr<Expr> initializer = parseExpression();
    consume(TokenType::SEMICOLON, "expected ; at end of send statement");

    return std::make_unique<SendStmt>(std::move(initializer));
}

//AssignStmt   → IDENT "=" Expr ";"
//post declaration, if you want to update an existing identifier
std::unique_ptr<Stmt> Parser::parseAssignStatement(){
    Token varName = consume(TokenType::IDENT, "expected var IDENT name after 'let'");
    consume(TokenType::ASSIGN, "expected '=' after IDENT name");
    std::unique_ptr<Expr> initializer = parseExpression();
    consume(TokenType::SEMICOLON, "expected ; at end of assignment statement");

    return std::make_unique<AssignStmt>(varName.value, std::move(intializer));
}

//expr statement 
//ExprStmt     → Expr ";"
//seems weird for an expr to be a statement, but function call side effects exist
std::unique_ptr<Stmt> Parser::parseExprStatement(){
    std::unique_ptr<Expr> initializer = parseExpression();
    consume(TokenType::SEMICOLON, "expected ; at end of assignment statement");

    return std::make_unique<ExprStmt>(std::move(initializer));
}

//* = 0 or more statements
//Block        → "{" Statement* "}"
//logic is to just keep consuming statements into the vector
std::unique_ptr<Stmt> Parser::parseBlockStatement(){
    consume(TokenType::LEFT_BRACE, "expected '{' at start of block");
    std::vector<std::unique_ptr<Stmt>> statements;
    while(!check(TokenType::RIGHT_BRACE) && !isAtEnd()){
        statements.push_back(parseStatement());
    }
    consume(TokenType::RIGHT_BRACE, "expected '}' at end of block");

    return std::make_unique<BlockStmt>(std::move(statements));
}

/*
IfStmt   → "if" "(" Expr ")" "then" Block
               ("else" "if" "(" Expr ")" "then" Block)*
               ("else" Block)?
*/

//WhileStmt    → "while" "(" Expr ")" "then" Block
std::unique_ptr<Stmt> Parser::parseWhileStatement(){
    consume(TokenType::WHILE,, "expected 'while' at start of while statement");
    consume(TokenType::LEFT_PAREN, "expected '(' at start of condition");
    std::unique_ptr<Expr> condition = parseExpression();
    consume(TokenType::RIGHT_PAREN, "expected ')' at end of condition");
    consume(TokenType::THEN, "expected 'then' after condition");
    std::vector<std::unique_ptr<Stmt>> body = parseBlockStatement();

    return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

std::unique_ptr<Stmt> Parser::parseIfStatement(){

    std::vector<std::unique_ptr<IfBranch>> branches; 
    std::unique_ptr<BlockStmt> elsebody = nullptr;


    consume(TokenType::IF, "expected 'if' at start of if statement");
    consume(TokenType::LEFT_PAREN, "expected '(' at start of condition");
    std::unique_ptr<Expr> condition = parseExpression();
    consume(TokenType::RIGHT_PAREN, "expected ')' at end of condition");
    consume(TokenType::THEN, "expected 'then' after condition");
    std::unique_ptr<BlockStmt> body = parseBlockStatement();
    //initial if
    //could use emplace_back and remove make_unique to not necessarily create a new object in memory
    branches.push_back(std::make_unique<IfBranch>(std::move(condition), std::move(body)));

    //looping else-ifs 
    while(check(TokenType::ELSE) && checkNext(TokenType::IF) && !isAtEnd()){
        consume(TokenType::ELSE, "expected 'else' at start of else-if statement");
        consume(TokenType::IF, "expected 'if' after else in else-if statement");
        consume(TokenType::LEFT_PAREN, "expected '(' at start of condition");
        std::unique_ptr<Expr> condition = parseExpression();
        consume(TokenType::RIGHT_PAREN, "expected ')' at end of condition");
        consume(TokenType::THEN, "expected 'then' after condition");
        std::unique_ptr<BlockStmt> body = parseBlockStatement();
        branches.push_back(std::make_unique<IfBranch>(std::move(condition), std::move(body)));
    }

    if(check(TokenType::ELSE)){
        //technically this error condition will never occur but whatever
        consume(TokenType::ELSE, "expected 'else' at start of else statement");
        elsebody = parseBlockStatement();
    }

    return std::make_unique<IfStmt>(std::move(branches), std::move(elsebody));
}







//basic expression parser for now to test let (we'll only do primaryEXPR)
//primary expressions are the smallest blocks, such as integer literals, boolean literals
//variables, and grouped expressions (like (4+3))
//a complete expression parser will support precedence, not now though

std::unique_ptr<Expr> Parser::parseExpression(){
    return parsePrimary();
}

//will support stuff like let x : int = 5, or let y : bool = true;
std::unique_ptr<Expr> Parser::parsePrimary(){

    //primary expressions are int literals, bool literals, and variables (IDENT)
    if(check(TokenType::INTEGER_LITERAL)){
        Token num = advance();
        std::size_t charUsed = 0;
        long long value;
        //gotta first convert the string into an integer
        try{
            value = std::stoll(num.value, &charUsed);
        }
        catch (const std::exception&){
            throw std::runtime_error("Invalid 64-bit integer literal");
        }
        //chars read is != length of the integer literal, means it wasnt read properly
        if(charUsed != num.value.length()){
            throw std::runtime_error("Invalid integer literal");
        }

        return std::make_unique<IntegerLiteralExpr>(value);
    }

    else if(check(TokenType::TRUE)){
        advance();
        return std::make_unique<BoolLiteralExpr>(true);
    }

    else if(check(TokenType::FALSE)){
        advance();
        return std::make_unique<BoolLiteralExpr>(false);
    }

    else if(check(TokenType::IDENT)){
        Token ident = advance();
        return std::make_unique<VariableExpr>(ident.value);
    }

    //(expr) is also a valid primary expression
    //technically (3+4) requires the binary expression functionality
    //but it is still considered a primary expression due to grouping
    else if(check(TokenType::LEFT_PAREN)){
        advance();
        std::unique_ptr<Expr> exp = parseExpression();
        consume(TokenType::RIGHT_PAREN, "expected ')' after expression");
        return exp;
    }
}







