//parser logic file
//what are we actually parsing?
//essentially translating the tokens from the token vector (output of lexer)
//into object nodes for the AST, the unique_ptr's are what connect each node down the tree
//AST represents the programs grammatical meaning, and is a useful structure for 
//the next phase (symbol tables, semantic analysis, and type checking)

#include "ast.hpp"
#include "token.hpp"
#include "parser.hpp"
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include <stdexcept>
#include <utility>


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
    if(position+1 >= tokens.size()) return false;
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

Token Parser::consume(TokenType expType, const std::string& errorMessage){
    if(check(expType)){
        return advance();
    }
    
    //account for eof
    //.value was the actual character/string of the token
    //i.e, would be the variable name if it is an IDENT token
    std::string found = peek().value;
    if(found.empty()) found = "end of file";

    throw std::runtime_error(errorMessage + " Found: '"+ found + "'.");
}

//parse variable types
//Type         → "int" | "bool"
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

    return std::make_unique<AssignStmt>(varName.value, std::move(initializer));
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
std::unique_ptr<BlockStmt> Parser::parseBlockStatement(){
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
    consume(TokenType::WHILE, "expected 'while' at start of while statement");
    consume(TokenType::LEFT_PAREN, "expected '(' at start of condition");
    std::unique_ptr<Expr> condition = parseExpression();
    consume(TokenType::RIGHT_PAREN, "expected ')' at end of condition");
    consume(TokenType::THEN, "expected 'then' after condition");
    std::unique_ptr<BlockStmt> body = parseBlockStatement();

    return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

std::unique_ptr<Stmt> Parser::parseIfStatement(){

    std::vector<IfBranch> branches; 
    std::unique_ptr<BlockStmt> elsebody = nullptr;


    consume(TokenType::IF, "expected 'if' at start of if statement");
    consume(TokenType::LEFT_PAREN, "expected '(' at start of condition");
    std::unique_ptr<Expr> condition = parseExpression();
    consume(TokenType::RIGHT_PAREN, "expected ')' at end of condition");
    consume(TokenType::THEN, "expected 'then' after condition");
    std::unique_ptr<BlockStmt> body = parseBlockStatement();
    //initial if
    //could use emplace_back and remove make_unique to not necessarily create a new object in memory
    branches.emplace_back(std::make_unique<IfBranch>(std::move(condition), std::move(body)));

    //looping else-ifs 
    while(check(TokenType::ELSE) && checkNext(TokenType::IF) && !isAtEnd()){
        consume(TokenType::ELSE, "expected 'else' at start of else-if statement");
        consume(TokenType::IF, "expected 'if' after else in else-if statement");
        consume(TokenType::LEFT_PAREN, "expected '(' at start of condition");
        std::unique_ptr<Expr> condition = parseExpression();
        consume(TokenType::RIGHT_PAREN, "expected ')' at end of condition");
        consume(TokenType::THEN, "expected 'then' after condition");
        std::unique_ptr<BlockStmt> body = parseBlockStatement();
        branches.emplace_back(std::make_unique<IfBranch>(std::move(condition), std::move(body)));
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
    //parse expression will begin with the lowest precedence parser
    //then it will work its way up by continously calling higher and higher
    //parsers until it gets to the one it needs to
    return parseOr();
}


/*
precedence rules: 
0. primary and parentheses (the most indivisible bottom case expression)
1. function calls
2. Unary operations
3. Mult and Division
4. Addition and Subtraction
5. Comparison Operations (is, is_lte, is_gte . . .)
6. AND
7. OR
*/


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

    //error check
    std::string found = peek().value.empty() ? "end of file" : peek().value;
    throw std::runtime_error("expected expression. Found: '" + found + "'.");
}

//unary is the only one different with 1 operand
std::unique_ptr<Expr> Parser::parseUnary(){

    if(check(TokenType::NOT) || check(TokenType::MINUS)){
        TokenType opToken = advance().type;
        UnaryOperator op;
        if(opToken == TokenType::NOT) op = UnaryOperator::Not;
        else op = UnaryOperator::Negation;

        //chain repeated unary operators
        //NOT NOT NOT NOT ... TRUE 
        std::unique_ptr<Expr> operand = parseUnary();

        return std::make_unique<UnaryExpr>(std::move(operand), op);
    }

    //assuming no unary operator, just continue to higher precedence
    return parseCall();

}


//although its called parseMultiplication, its just precedence for mult and div
std::unique_ptr<Expr> Parser::parseMultiplication(){
    //should call the next higher precedence parser
    //explaination as to why in doc:
    std::unique_ptr<Expr> left = parseUnary();
    
    while(check(TokenType::TIMES) || check(TokenType::DIVIDE)){
        //consume multiplication/division operator (or just advance doesnt matter)
        TokenType opToken = advance().type;

        std::unique_ptr<Expr> right = parseUnary();
        BinaryOperator op;
        if(opToken == TokenType::TIMES) op = BinaryOperator::Multiply;
        else op = BinaryOperator::Divide;

        //make the new left the current binary operation so we can recursively
        //chain operations
        left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
    }
    return left;
}


std::unique_ptr<Expr> Parser::parseAddition(){
    std::unique_ptr<Expr> left = parseMultiplication();
    while(check(TokenType::PLUS) || check(TokenType::MINUS)){
        TokenType opToken = advance().type;

        std::unique_ptr<Expr> right = parseMultiplication();
        BinaryOperator op;
        if(opToken == TokenType::PLUS) op = BinaryOperator::Add;
        else op = BinaryOperator::Subtract;

        left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
    }
    return left;
}

std::unique_ptr<Expr> Parser::parseComparison(){
    std::unique_ptr<Expr> left = parseAddition();
    while(
        check(TokenType::IS) ||
        check(TokenType::IS_LT) ||
        check(TokenType::IS_LTE) ||
        check(TokenType::IS_GT) ||
        check(TokenType::IS_GTE)
    ){
        TokenType opToken = advance().type;

        std::unique_ptr<Expr> right = parseAddition();
        BinaryOperator op;
        if(opToken == TokenType::IS) op = BinaryOperator::Equal;
        else if(opToken == TokenType::IS_GT) op = BinaryOperator::GreaterThan;
        else if(opToken == TokenType::IS_GTE) op = BinaryOperator::GreaterThanEqual;
        else if(opToken == TokenType::IS_LT) op = BinaryOperator::LessThan;
        else if(opToken == TokenType::IS_LTE) op = BinaryOperator::LessThanEqual;
        advance();

        left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
    }
    return left;
}

std::unique_ptr<Expr> Parser::parseAnd(){
    std::unique_ptr<Expr> left = parseComparison();
    while(check(TokenType::AND)){
        TokenType opToken = advance().type;

        std::unique_ptr<Expr> right = parseComparison();
        left = std::make_unique<BinaryExpr>(std::move(left), BinaryOperator::And, std::move(right));
    }
    return left;
}

std::unique_ptr<Expr> Parser::parseOr(){
    std::unique_ptr<Expr> left = parseAnd();
    while(check(TokenType::OR)){
        TokenType opToken = advance().type;

        std::unique_ptr<Expr> right = parseAnd();
        left = std::make_unique<BinaryExpr>(std::move(left), BinaryOperator::Or, std::move(right));
    }
    return left;
}


//function related parsers
//last bit of stuff

std::unique_ptr<Expr> Parser::parseCall(){

    std::vector<std::unique_ptr<Expr>> arguments;

    Token varName = consume(TokenType::IDENT, "Expected function name");
    consume(TokenType::LEFT_PAREN, "expected '(' after function name");
    //function calls may have 0 arguments, need do-while

    if(!check(TokenType::RIGHT_PAREN)){
        do{
            arguments.push_back(parseExpression());
            if(!check(TokenType::COMMA)) break; //no more arguments
            consume(TokenType::COMMA, "expected comma between arguments");
        } while(true);
    }
    

    consume(TokenType::RIGHT_PAREN, "expected ')' after arguments");
    
    return std::make_unique<CallExpr>(std::move(arguments), std::move(varName.value));
}

//Parameter    → IDENT ":" Type
//does not need to be unique_ptr since parseParameters can own them as
//individual objects in its vector, also since Parameter is not a polymorphic AST node, it just is parameter
//helper function for parameters
Parameter Parser::parseParameter(){
    Token varName = consume(TokenType::IDENT, "expected parameter name");
    consume(TokenType::COLON, "expected ':' after parameter name");
    JKCType parType = parseType();

    return Parameter(std::move(varName.value), parType);
}

//Parameters   → Parameter ("," Parameter)*
//wow good!
std::vector<Parameter> Parser::parseParameters(){
    std::vector<Parameter> parameters;
    parameters.push_back(parseParameter());
    while(check(TokenType::COMMA)){
        advance();
        parameters.push_back(parseParameter());
    }
    return parameters;
}

//Function → "fn" IDENT "(" Parameters? ")" "->" Type Block
std::unique_ptr<FunctionDec> Parser::parseFunction(){
    consume(TokenType::FN, "expected 'fn' at start of function declaration");
    Token funcName = consume(TokenType::IDENT, "expected function name after fn");
    consume(TokenType::LEFT_PAREN, "expected '(' at the start of parameter declaration");
    std::vector<Parameter> parameters;

    //assuming there ARE parameters (0 param functions can skip this)
    if(!check(TokenType::RIGHT_PAREN)) parameters = parseParameters();

    consume(TokenType::RIGHT_PAREN, "expected ')' at the end of parameter declaration");
    consume(TokenType::ARROW, "expected '->' after parameter declaration");
    JKCType funcType = parseType();
    std::unique_ptr<BlockStmt> body = parseBlockStatement();

    return std::make_unique<FunctionDec>(std::move(funcName.value), std::move(parameters), funcType, std::move(body));
}

//Program → Function*
//in essence, the program is basically just a vector of functions
//the program must contain at least one function (main) to run raw code
//this is the top node of the recursive descent
std::unique_ptr<Program> Parser::parseProgram(){
    std::vector<std::unique_ptr<FunctionDec>> functions;
    while(!isAtEnd()){
        functions.push_back(parseFunction());
    }

    return std::make_unique<Program>(std::move(functions));
}
















