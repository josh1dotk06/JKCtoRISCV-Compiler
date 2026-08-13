#pragma once
#include "ast.hpp"
#include "ir.hpp"
#include <string>
#include <vector>
#include <unordered_map>

class IRGenerate{
private:
    //head of our IR structure, we will step by step construct it
    IRProgram program;

    IRFunction* currentFunction = nullptr;

    //t1,t2,t3 and l1,l2,l3
    std::size_t tempCounter = 0;
    std::size_t labelCounter = 0;

    IRValue makeTemp();
    std::string makeLabel();
    void emit(std::unique_ptr<IRInstruction> instructions);


    void lowerFunction(const FunctionDec& func);
    void lowerStmt(const Stmt& stmt);

    void lowerLetStmt(const LetStmt& stmt);
    void lowerAssignStmt(const AssignStmt& stmt);
    void lowerSendStmt(const SendStmt& stmt);
    void lowerWhileStmt(const WhileStmt& stmt);
    void lowerIfStmt(const IfStmt& stmt);
    void lowerExprStmt(const ExprStmt& stmt);
    void lowerBlockStmt(const BlockStmt& stmt);



    IRValue lowerExpr(const Expr& expr);

    IRValue lowerIntegerLiteralExpr(const IntegerLiteralExpr& expr);
    IRValue lowerBoolLiteralExpr(const BoolLiteralExpr& expr);
    IRValue lowerVariableExpr(const VariableExpr& expr);
    IRValue lowerBinaryExpr(const BinaryExpr& expr);
    IRValue lowerUnaryExpr(const UnaryExpr& expr);
    IRValue lowerCallExpr(const CallExpr& expr);

public:

    //in main: take the checked ast and generate an IRProgram obj
    //pass that object into our constructor which will be the head
    explicit IRGenerate(IRProgram& program) : program(program) {}
};