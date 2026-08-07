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

void TypeChecker::checkBlockStmt(const BlockStmt& stmt){

}
