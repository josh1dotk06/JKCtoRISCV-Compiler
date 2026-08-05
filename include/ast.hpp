#pragma once

//the whole point of this file is to create the structures to represent the parsed/ast
//i.e initializing the AST node classes
//program in memory, also the print stuff are just there for debugging and visualization
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

//only var types, not token types
enum class JKCType{
    Int,
    Bool
};

enum class BinaryOperator{
    Add,
    Subtract,
    Multiply,
    Divide,

    Equal,
    LessThan,
    LessThanEqual,
    GreaterThan,
    GreaterThanEqual,

    And,
    Or
};

//we have arithmetic negation and NOT as our unary
enum class UnaryOperator{
    Not,
    Negation
};

//inline function causes compiler to replaces function call directly with this code
//also use std for headers, since using namespace std can get copied to whatever file
//is using this header
inline std::string getJKCTypeName(JKCType type){
    switch(type){
        case JKCType::Int:
            return "int";
        case JKCType::Bool:
            return "bool";
    }

    return "unknown";
}

inline std::string getUnaryOperatorName(UnaryOperator op){
    switch(op){
        case UnaryOperator::Not:
            return "NOT";
        
        case UnaryOperator::Negation:
            return "-";
    }
    return "unknown";
}

inline std::string getBinaryOperatorName(BinaryOperator op) {
    switch (op) {
        case BinaryOperator::Add:
            return "+";

        case BinaryOperator::Subtract:
            return "-";

        case BinaryOperator::Multiply:
            return "*";

        case BinaryOperator::Divide:
            return "/";

        case BinaryOperator::Equal:
            return "is";

        case BinaryOperator::LessThan:
            return "is_lt";

        case BinaryOperator::LessThanEqual:
            return "is_lte";

        case BinaryOperator::GreaterThan:
            return "is_gt";

        case BinaryOperator::GreaterThanEqual:
            return "is_gte";

        case BinaryOperator::And:
            return "AND";

        case BinaryOperator::Or:
            return "OR";
    }

    return "unknown";
}


//base class for every ast node
//all these weird virtual ~ stuff, this is just cpp's version of java's abstract class
//it just needs to be more explicit in cpp since java better
struct ASTNode{
    //~ makes sure memory gets freed automatically when object dies
    //e.g new allocates memory, this memory stays until its freed
    virtual ~ASTNode() = default;
    virtual void print(int indent = 0) const = 0;
};

//expr base clasee
struct Expr : ASTNode{

};

//stmt base class
struct Stmt : ASTNode{

};

//represent an integer literal like 5 or 22
struct IntegerLiteralExpr : Expr{
    std::int64_t value;
    //these explciit things are constructors btw
    //value(value) is this->value = value; (member intiializer)
    explicit IntegerLiteralExpr(std::int64_t value) : value(value) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "IntegerLiteral(" << value << ")\n";
    }
};

struct BoolLiteralExpr : Expr{
    bool value;
    explicit BoolLiteralExpr(bool value) : value(value) {}

    void print(int indent = 0) const override{
        std::cout << std::string(indent, ' ') << "BoolLiteral(" << (value ? "true" : "false") << ")\n";
    }
};

//repr a variable
struct VariableExpr : Expr {
    std::string name;

    explicit VariableExpr(std::string name) : name(std::move(name)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "Variable(" << name << ")\n";
    }
};


struct UnaryExpr : Expr{
    std::unique_ptr<Expr> operand;
    UnaryOperator op;

    UnaryExpr(std::unique_ptr<Expr> operand, UnaryOperator op) : operand(std::move(operand)), op(op) {}

    void print(int indent = 0) const override {
        const std::string padding(indent, ' ');
       //future text
    }
};

//binary expression will consist of a left exp, right exp, and an operator inbetween
struct BinaryExpr : Expr {
    std::unique_ptr<Expr> left;
    BinaryOperator op;
    std::unique_ptr<Expr> right;

    BinaryExpr(std::unique_ptr<Expr> left, BinaryOperator op, std::unique_ptr<Expr> right) : left(std::move(left)), op(op), right(std::move(right)) {}
    
    void print(int indent = 0) const override {
        const std::string padding(indent, ' ');
       //future text
    }
};

//let statement like let x : int = 5
struct LetStmt : Stmt{
    std::string name;
    JKCType decType;
    //essentially the expr thats being assigned in the let
    std::unique_ptr<Expr> initializer;

    LetStmt(std::string name, JKCType decType, std::unique_ptr<Expr> initializer)
    : name(std::move(name)),
    decType(decType),
    initializer(std::move(initializer)) {}

    void print(int indent = 0) const override {
        const std::string padding(indent, ' ');

        //testing
        // std::cout << padding << "LetStmt\n";
        // std::cout << padding << "├── name: " << name << '\n';
        // std::cout << padding << "├── type: "
        //           << getJKCTypeName(declaredType) << '\n';
        // std::cout << padding << "└── initializer:\n";

        // initializer->print(indent + 4);
    }
};


struct SendStmt : Stmt{
    std::unique_ptr<Expr> initializer;
    SendStmt(std::unique_ptr<Expr> initializer) : initializer(std::move(initializer)) {}

    void print(int indent = 0) const override {
        const std::string padding(indent, ' ');
       //future text
    }
};

struct AssignStmt : Stmt{
    std::unique_ptr<Expr> initializer;
    std::string name;
    AssignStmt(std::string name, std::unique_ptr<Expr> initializer) : initializer(std::move(initializer)), name(std::move(name)) {}
    
    void print(int indent = 0) const override {
    const std::string padding(indent, ' ');
       //future text
    }
};

struct ExprStmt : Stmt{
    std::unique_ptr<Expr> initializer;
    ExprStmt(std::unique_ptr<Expr> initializer) : initializer(std::move(initializer)) {}
    
    void print(int indent = 0) const override {
    const std::string padding(indent, ' ');
       //future text
    }
};

//block will contain 0 or more statements, thus must require a vector
struct BlockStmt : Stmt{
    std::vector<std::unique_ptr<Stmt>> statements;

    explicit BlockStmt(std::vector<std::unique_ptr<Stmt>> statements) : statements(std::move(statements)) {}
    void print(int indent = 0) const override {
    const std::string padding(indent, ' ');
       //future text
    }
};

//while consists of an expression and a blockstmt body
struct WhileStmt : Stmt{
    std::unique_ptr<Expr> condition;
    std::unique_ptr<BlockStmt> body;

    WhileStmt(std::unique_ptr<Expr> condition, std::unique_ptr<BlockStmt> body) : condition(std::move(condition)), body(std::move(body)) {}
    void print(int indent = 0) const override {
    const std::string padding(indent, ' ');
       //future text
    }
};

//an if statement can have 1 or more if branches (includes if or else if)
//each branch consists of a condition and body
//the last optional else branch would contain only a body

struct IfBranch{
    std::unique_ptr<Expr> condition;
    std::unique_ptr<BlockStmt> body;

    IfBranch(std::unique_ptr<Expr> condition, std::unique_ptr<BlockStmt> body) : condition(std::move(condition)), body(std::move(body)) {}
};

struct IfStmt : Stmt{
    std::vector<IfBranch> branches;
    std::unique_ptr<BlockStmt> elseBody;

    IfStmt(std::vector<IfBranch> branches, std::unique_ptr<BlockStmt> elseBody) : branches(std::move(branches)), elseBody(std::move(elseBody)) {}
    
    void print(int indent = 0) const override {
    const std::string padding(indent, ' ');
      //future text
    }
};

//function related stuff//
//last bit of stuff we need to be able to parse
//since our parser has yet to recognize stuff like calc(x,y) or fn func1(. . .)
//or fn main(){} etc

//call expression like fact(5) or add(x,y)
struct CallExpr : Expr{

    std::vector<std::unique_ptr<Expr>> arguments;
    std::string funcName;

    explicit CallExpr(std::vector<std::unique_ptr<Expr>> arguments, std::string funcName) : arguments(std::move(arguments)), funcName(std::move(funcName)) {}
    void print(int indent = 0) const override {
    const std::string padding(indent, ' ');
       //future text
    }
};

//require parameters for function declarations
/*
Function     → "fn" IDENT "(" Parameters? ")" "->" Type Block
Parameters   → Parameter ("," Parameter)*
Parameter    → IDENT ":" Type
*/

struct Parameter : ASTNode{
    std::string name;
    JKCType type;

    Parameter(std::string name, JKCType type) : name(std::move(name)), type(type) {}
    void print(int indent = 0) const override {
    const std::string padding(indent, ' ');
       //future text
    }
};

struct FunctionDec : ASTNode{
    std::string name;
    std::vector<Parameter> parameters;
    JKCType returnType;
    std::unique_ptr<BlockStmt> body;

    FunctionDec(std::string name, std::vector<Parameter> parameters, JKCType returnType, std::unique_ptr<BlockStmt> body) : name(std::move(name)), parameters(std::move(parameters)), returnType(returnType), body(std::move(body)) {}
    void print(int indent = 0) const override {
    const std::string padding(indent, ' ');
       //future text
    }
};

//Program      → Function*
//program node is just a vector of functions
struct Program : ASTNode{
    std::vector<std::unique_ptr<FunctionDec>> functions;

    Program(std::vector<std::unique_ptr<FunctionDec>> functions) : functions(std::move(functions)) {}
    void print(int indent = 0) const override {
    const std::string padding(indent, ' ');
       //future text
    }
};






