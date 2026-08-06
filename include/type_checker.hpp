#pragma once
//traverse ast and type check using symbol table
#include <string>
#include <vector>
#include <unordered_map>
#include "ast.hpp" //we need jkc type
#include "symbol_table.hpp"

//semantic analyzer and type checker in one
class TypeChecker{
private:
    SymbolTable symbols;
    void checkLetStmt(const LetStmt& stmt);
    void checkAssignStmt(const AssignStmt& stmt);

public:
    void checkProgram(const Program& program);
    void checkStmt(const Stmt& stmt);
    JKCType checkExpr(const Expr& expr);

};