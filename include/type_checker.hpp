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
    const FunctionDec* currentFunction = nullptr; //what func were curr in

    void checkLetStmt(const LetStmt& stmt);
    void checkAssignStmt(const AssignStmt& stmt);
    void checkSendStmt(const SendStmt& stmt);
    void checkWhileStmt(const WhileStmt& stmt);
    void checkIfStmt(const IfStmt& stmt);
    void checkExprStmt(const ExprStmt& stmt);

    void checkStmt(const Stmt& stmt);
    void checkBlockStmt(const BlockStmt& stmt);

    JKCType checkExpr(const Expr& expr);
    JKCType checkIntegerLiteralExpr(const IntegerLiteralExpr& expr);
    JKCType checkBoolLiteralExpr(const BoolLiteralExpr& expr);
    JKCType checkVariableExpr(const VariableExpr& expr);
    JKCType checkBinaryExpr(const BinaryExpr& expr);
    JKCType checkUnaryExpr(const UnaryExpr& expr);
    JKCType checkCallExpr(const CallExpr& expr);

public:
    void checkProgram(const Program& program);
    void checkStmt(const Stmt& stmt);
    JKCType checkExpr(const Expr& expr);

};