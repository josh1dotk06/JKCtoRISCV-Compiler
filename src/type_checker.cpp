#pragma once
//traverse ast and type check using symbol table
#include <string>
#include <vector>
#include <unordered_map>
#include "ast.hpp" //we need jkc type
#include "symbol_table.hpp"
#include "type_checker.hpp"

std::string errormsg(std::string type1, std::string type2){
    return "Expected " + type1 + ", got " + type2;
}
//test
//test 2

void TypeChecker::checkLetStmt(const LetStmt& stmt){
    if(symbols.variableExistsInCurrentScope(stmt.name)){
        throw std::runtime_error("variable '" + stmt.name + "' already declared");
        //aka you cant just do let x : int = 5, then let x :int = 21 again
    }

    //recall that initializer is unique_ptr, we need the objct
    JKCType initializerType = checkExpr(*(stmt.initializer));

    //let x : int = true   -> this is bad
    if(initializerType != stmt.decType){
        throw std::runtime_error(errormsg(getJKCTypeName(stmt.decType), getJKCTypeName(initializerType)));
    }

    symbols.declareVariable(stmt.name, stmt.decType, VariableKind::LOCAL);
}

//AssignStmt   → IDENT "=" Expr ";"
//checks: ensure its been declared, ensure decType matches
void TypeChecker::checkAssignStmt(const AssignStmt& stmt){
    //same var can be reassigned, so the first check from the letstmt
    //is not needed
    //also you can reassign variables that exist outside curr scope
    //thus use lookupVariable
    const VariableSymbol* symbol = symbols.lookupVariable(stmt.name);

    if(symbol==nullptr){
        throw std::runtime_error("variable '" + stmt.name + "' not declared in local scope");
    }

    JKCType initializerType = checkExpr(*(stmt.initializer));

    if(initializerType != (*symbol).type){
        throw std::runtime_error("incorrect type expected");
    }
    
    //dont use declareVariable here, we arent making a new variable, just changing value
}

//SendStmt → "send" Expr ";"
//checks: ensure send expr type is the same as function declared type
void TypeChecker::checkSendStmt(const SendStmt& stmt){
    JKCType initializerType = checkExpr(*(stmt.initializer));

    //require checkFunction to set the currentFunction of our typechecker
    if(initializerType != (*currentFunction).returnType){
        throw std::runtime_error("incorrect return type");
    }
}

//WhileStmt → "while" "(" Expr ")" "then" Block
//checks: ensure expr type is type bool, 
void TypeChecker::checkWhileStmt(const WhileStmt& stmt){
    JKCType conditionType = checkExpr(*(stmt.condition));
    if(conditionType != JKCType::Bool){
        throw std::runtime_error("while condition must be of type boolean");
    }

    //type check loop body later here

}

//checks: ensure all if-else branches have expr of type bool
//we dont need to verify that else comes after if or something, because parser
//already verified that 
void TypeChecker::checkIfStmt(const IfStmt& stmt){

    for(auto& branch : stmt.branches){
        JKCType conditionType = checkExpr(*(branch.condition));
        if(conditionType != JKCType::Bool){
            throw std::runtime_error("if-else condition must be of type boolean");
        }
        //check individual bodys
    }
    //also check the final else body if it exists
}

void TypeChecker::checkStmt(const Stmt& stmt){
    //the pointers are of type Stmt, but the actual objects are the specific subclass type (again just polymorphism)
    //how do we check the subclass type for each specific stmt object?
    //use dynamic casting

    //dynamic_cast<const LetStmt*>*(&stmt) returns nullptr if stmt is not a LetStmt
    //null ptr behaves like false
    //otherwise it returns a valid pointer to the letstmt object in the block

    if(auto letStmt = dynamic_cast<const LetStmt*>(&stmt)) checkLetStmt(*letStmt);
    else if(auto assignStmt = dynamic_cast<const AssignStmt*>(&stmt)) checkAssignStmt(*assignStmt);
    else if(auto ifStmt = dynamic_cast<const IfStmt*>(&stmt)) checkIfStmt(*ifStmt);
    else if(auto whileStmt = dynamic_cast<const WhileStmt*>(&stmt)) checkWhileStmt(*whileStmt);
    else if(auto sendStmt = dynamic_cast<const SendStmt*>(&stmt)) checkSendStmt(*sendStmt);
    else if(auto exprStmt = dynamic_cast<const ExprStmt*>(&stmt)) checkExprStmt(*exprStmt);
    else if(auto blockStmt = dynamic_cast<const BlockStmt*>(&stmt)) checkBlockStmt(*blockStmt);
}

void TypeChecker::checkBlockStmt(const BlockStmt& stmt){
    for(const auto& singularStmt : stmt.statements){
        checkStmt(*singularStmt);
    }
}


//EXPRESSION CHECKERS

//check: 
JKCType TypeChecker::checkIntegerLiteralExpr(const IntegerLiteralExpr& expr){
    return JKCType::Int;
}

JKCType TypeChecker::checkBoolLiteralExpr(const BoolLiteralExpr& expr){
    return JKCType::Bool;
}

//variable as an expression
//check: to ensure that this variable exists in an active scope
JKCType TypeChecker::checkVariableExpr(const VariableExpr& expr){
    const VariableSymbol* symbol = symbols.lookupVariable(expr.name);
    if(symbol == nullptr){
        throw std::runtime_error("undeclared variable: " + expr.name);
    }

    return (*symbol).type;
}

//check to ensure that the 2 operands are not of different type
//and that the operands must be a specific type depending on the operation
JKCType checkBinaryExpr(const BinaryExpr& expr){
    BinaryOperator opr = expr.op;
    JKCType typeLeft = checkExpr(*(expr.left));
    JKCType typeRight = checkExpr(*(expr.right));
    

    if(opr == BinaryOperator::Add || opr == BinaryOperator::Multiply|| opr == BinaryOperator::Subtract || opr == BinaryOperator::Divide){
        if(typeLeft == JKCType::Bool || typeRight == JKCType::Bool){
            throw std::runtime_error("incorrect type, expected int");
        }
        return JKCType::Int;
    }

    else if(opr == BinaryOperator::Equal || opr == BinaryOperator::LessThan || opr == BinaryOperator::LessThanEqual || opr == BinaryOperator::GreaterThan || opr == BinaryOperator::GreaterThanEqual){
        if(typeLeft == JKCType::Bool || typeRight == JKCType::Bool){
            throw std::runtime_error("incorrect type, expected int");
        }
        return JKCType::Bool;
    }

    else if(opr == BinaryOperator::And || opr == BinaryOperator::Or){
        if(typeLeft == JKCType::Int || typeRight == JKCType::Int){
            throw std::runtime_error("incorrect ty[e, expected bool");
        }
        return JKCType::Bool;
    }

    throw std::runtime_error("unknown binary operator");
}

JKCType TypeChecker::checkUnaryExpr(const UnaryExpr& expr){
    UnaryOperator opr = expr.op;
    JKCType exprType = checkExpr(*(expr.operand));

    if(opr == UnaryOperator::Not){
        if(exprType == JKCType::Int){
            throw std::runtime_error("incorrect type, expected bool");
        }
        return JKCType::Bool;
    }
    else if(opr == UnaryOperator::Negation){
        if(exprType == JKCType::Bool){
            throw std::runtime_error("incorrct type, expected int");
        }
        return JKCType::Int;
    }

    throw std::runtime_error("unknown unary operator");
}

//check to ensure function being called exists and parameters match arguments
JKCType TypeChecker::checkCallExpr(const CallExpr& expr){
    if(!symbols.functionExists(expr.funcName)){
        throw std::runtime_error("' " + expr.funcName + "' does not exist");
    }

    const FunctionSymbol* function = symbols.lookupFunction(expr.funcName);
    std::vector<JKCType> params = function->paramTypes;

    int i = 0;
    for(const auto& argument : expr.arguments){
        JKCType exprType = checkExpr(*argument);
        if(exprType != params[i]){
            throw std::runtime_error("incorrect type in function arguments");
        }
        i++;
    }

    return (*function).returnType;
}
