#pragma once

#include "ast.hpp"
#include "token.hpp"
#include <cstddef>
#include <memory>
#include <string> 
#include <vector> 

//parser basically = lexer, it stores string input, and position
//except this time it moves through the tokens rather than through the source code
class Parser{
private:
    const std::vector<Token>& tokens;
    std::size_t position;

    //token functions prototype (declarations)
    //read next token object in input stream, direct reference to token (read only thus)
    //second const promises not to modify any member variables of the parser class
    //also member variables are just private fields (in java terms)
    const Token& peek() const;
    const Token& advance();

    bool check(TokenType type) const;
    bool checkNext(TokenType type) const;
    bool isAtEnd() const;

    Token consume(TokenType expType, const std::string& errorMessage);

    //GRAMMAR PARSERS
    JKCType parseType();

    std::unique_ptr<Expr> parseExpression();

    std::unique_ptr<Expr> parseOr();
    std::unique_ptr<Expr> parseAnd();
    std::unique_ptr<Expr> parseComparison();
    std::unique_ptr<Expr> parseAddition();
    std::unique_ptr<Expr> parseMultiplication();
    std::unique_ptr<Expr> parseUnary();
    std::unique_ptr<Expr> parsePrimary();
    std::unique_ptr<Expr> parseCall(); //function call expressions

    std::unique_ptr<Stmt> parseLetStatement();
    std::unique_ptr<Stmt> parseAssignStatement();
    std::unique_ptr<Stmt> parseSendStatement();
    std::unique_ptr<BlockStmt> parseBlockStatement();
    std::unique_ptr<Stmt> parseIfStatement();
    std::unique_ptr<Stmt> parseWhileStatement();
    std::unique_ptr<Stmt> parseExprStatement();

    Parameter parseParameter();
    std::vector<Parameter> parseParameters();
    std::unique_ptr<FunctionDec> Parser::parseFunction();


public:
    //construct and intiialize member list (private fields)
    //dont use move here since its a const reference, we are modifying the og
    Parser(const std::vector<Token>& tokens) : tokens(tokens), position(0) {}

    std::unique_ptr<Stmt> parseStatement();
    std::unique_ptr<Program> parseProgram();
};