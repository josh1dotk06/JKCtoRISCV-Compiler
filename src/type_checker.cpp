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
    JKCType initializerType = checkExpr(*(stmt.initializer));
    if(!symbols.variableExistsInCurrentScope(stmt.name)){
        throw std::runtime_error("variable '" + stmt.name + "' not declared in local scope");
    }

    auto match = symbols.lookupVariable(stmt.name);
    if(initializerType != (*match).type){
        throw std::runtime_error("incorrect type expected");
    }
    
    symbols.declareVariable(stmt.name, (*match).type, VariableKind::LOCAL);

}