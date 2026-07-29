#pragma once

//the whole point of this file is to create the structures to represent the parsed/ast
//i.e initializing the AST node classes
//program in memory, also the print stuff are just there for debugging and visualization
#include <stdint>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

//only var types, not token types
enum class JKCType{
    Int,
    Bool
};

//inline function causes compiler to replaces function call directly with this code
//also use std for headers, since using namespace std can get copied to whatever file
//is using this header
inline std::string getJKCTypeName(JKCType type){
    switch(type){
        case JKCType::Int:
            return "int";
        case JkCType::Bool:
            return "bool";
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
}

//expr base clasee
struct Expr : ASTNode{

};

//stmt base class
struct Stmt : ASTNode{

};

//repr an integer literal like 5 or 22
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
        std::cout << std::string(indent, ' ') << "IntegerLiteral(" << (value ? "true" : "false") << ")\n";
    }
};

//repr a variable
struct VariableExpr : Expr {
    std::string name;

    explicit VariableExpr(std::string name) : name(std::move(name) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "Variable(" << name << ")\n";
    }
};

//let statement like let x : int = 5
struct LetStmt : Stmt{
    std::string name;
    JKCType decType;
    //essentially the expr thats being assigned in the let
    std::unique_ptr<Expr> initializer;

    LetStmt(std::string name, JKCType decType, std::unique_ptr<Expr> initializer)
    : name(std::move(name));
    decType(decType);
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


struct SendStmt : Stmt{
    std::unique_ptr<Expr> initializer;
    SendStmt(std::unique_ptr<Expr> initializer) : initializer(std::move(intitializer)) {}

    void print(int indent = 0) const override {
        const std::string padding(indent, ' ');
1       //future text
    }
}

struct AssignStmt : Stmt{
    std::unique_ptr<Expr> initializer;
    std::string name;
    AssignStmt(std::string name, std::unique_ptr<Expr> initializer) : initializer(std::move(initializer)); name(std::move(name)) {}
    
    void print(int indent = 0) const override {
    const std::string padding(indent, ' ');
1       //future text
    }
}

struct ExprStmt : Stmt{
    std::unique_ptr<Expr> initializer;
    ExprStmt(std::unique_ptr<Expr> initializer) : initializer(std::move(initializer)) {}
    
    void print(int indent = 0) const override {
    const std::string padding(indent, ' ');
1       //future text
    }
}



};





