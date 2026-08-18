#pragma once
#include "ast.hpp"
#include "ir.hpp"
#include "ir_generator.hpp"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
//how to traverse + build at the same time
//use the type_checker's recursive descent approach to traverse the ast nodes
//use emit helper function to individually create ir objects and thus construct the whole ir
//ast nodes are to be traversed, ir nodes are to be built


IRUnaryOp IRGenerate::convertUnaryOp(UnaryOperator op){
    switch(op){
        case UnaryOperator::Negation:
            return IRUnaryOp::Negation;
        case UnaryOperator::Not:
            return IRUnaryOp::Not;
    }
    throw std::runtime_error("Unknown UnaryOperator");
}

//convert AST enum type to the IR enum type (for lowerIrBin)
IRBinaryOp IRGenerate::convertBinaryOp(BinaryOperator op){
    switch(op){
        case BinaryOperator::Add:
            return IRBinaryOp::Add;

        case BinaryOperator::Subtract:
            return IRBinaryOp::Subtract;

        case BinaryOperator::Multiply:
            return IRBinaryOp::Multiply;

        case BinaryOperator::Divide:
            return IRBinaryOp::Divide;

        case BinaryOperator::Equal:
            return IRBinaryOp::Equal;

        case BinaryOperator::LessThan:
            return IRBinaryOp::LessThan;

        case BinaryOperator::LessThanEqual:
            return IRBinaryOp::LessThanEqual;

        case BinaryOperator::GreaterThan:
            return IRBinaryOp::GreaterThan;

        case BinaryOperator::GreaterThanEqual:
            return IRBinaryOp::GreaterThanEqual;

        case BinaryOperator::And:
            return IRBinaryOp::And;

        case BinaryOperator::Or:
            return IRBinaryOp::Or;
    }

    throw std::runtime_error("Unknown BinaryOperator");
}


void IRGenerate::lowerLetStmt(const LetStmt& stmt){
    IRValue destination;
    destination.kind = IRValueKind::Variable;
    destination.name = stmt.name;

    IRValue value = lowerExpr(*stmt.initializer);

    if(value.kind == IRValueKind::IntegerConstant || value.kind == IRValueKind::BoolConstant){
        emit(std::make_unique<IRConst>(destination, value));
        return;
    }

    //let x = a + b translaes to
    //binOp t1 = a + b
    //move x, t1

    emit(std::make_unique<IRMove>(destination, value));
}

//same as let but no need for const because no initialization
void IRGenerate::lowerAssignStmt(const AssignStmt& stmt){
    IRValue destination;
    destination.kind = IRValueKind::Variable;
    destination.name = stmt.name;

    IRValue value = lowerExpr(*stmt.initializer);

    emit(std::make_unique<IRMove>(destination, value));
}

void IRGenerate::lowerSendStmt(const SendStmt& stmt){
    IRValue value = lowerExpr(*stmt.initializer);
    emit(std::make_unique<IRReturn>(value));
}

void IRGenerate::lowerBlockStmt(const BlockStmt& stmt){
    for(const auto& s : stmt.statements){
        lowerStmt(*s);
    }
}

void IRGenerate::lowerWhileStmt(const WhileStmt& stmt){

    std::string condLabel = makeLabel();
    std::string bodyLabel = makeLabel();
    std::string endLabel = makeLabel();

    emit(std::make_unique<IRLabel>(condLabel));

    IRValue cond = lowerExpr(*stmt.condition);

    emit(std::make_unique<IRBranch>(cond, bodyLabel, endLabel));

    emit(std::make_unique<IRLabel>(bodyLabel));
    lowerBlockStmt(*stmt.body);

    emit(std::make_unique<IRJump>(condLabel));

    emit(std::make_unique<IRLabel>(endLabel));
}

//wow nice!
void IRGenerate::lowerIfStmt(const IfStmt& stmt){
    std::string currLabel;
    std::string endLabel = makeLabel();
    int i = 0;
    for(const auto& branch : stmt.branches){
        if(i!=0) emit(std::make_unique<IRLabel>(currLabel));
        std::string ifLabel = makeLabel();
        std::string checkLabel = makeLabel();
        IRValue cond = lowerExpr(*branch.condition);
        emit(std::make_unique<IRBranch>(cond, ifLabel, checkLabel));
        emit(std::make_unique<IRLabel>(ifLabel));
        lowerBlockStmt(*branch.body);
        emit(std::make_unique<IRJump>(endLabel));
        currLabel = checkLabel;
        i++;
    }

    if(stmt.elseBody != nullptr){
        emit(std::make_unique<IRLabel>(currLabel));
        lowerBlockStmt(*stmt.elseBody);
    }
    else{
        emit(std::make_unique<IRLabel>(currLabel));
    }

    emit(std::make_unique<IRLabel>(endLabel));
}

void IRGenerate::lowerExprStmt(const ExprStmt& stmt){
    lowerExpr(*stmt.initializer);
}



IRValue IRGenerate::makeTemp(){
    IRValue temp;
    tempCounter++;
    temp.kind = IRValueKind::Temporary;
    temp.name = "t" + std::to_string(tempCounter);

    return temp;
}

std::string IRGenerate::makeLabel(){
    labelCounter++;
    std::string name = "label" + std::to_string(labelCounter);
    return name;
}

void IRGenerate::emit(std::unique_ptr<IRInstruction> instruction){
    (*currentFunction).instructions.push_back(instruction);
}

IRValue IRGenerate::lowerBinaryExpr(const BinaryExpr& expr){

    IRValue left = lowerExpr(*expr.left);
    IRValue right = lowerExpr(*expr.right);

    IRValue temp = makeTemp();
    emit(std::make_unique<IRBinOp>(temp, convertBinaryOp(expr.op), left, right));

    return temp;
}


IRValue IRGenerate::lowerUnaryExpr(const UnaryExpr& expr){
    IRValue operand = lowerExpr(*expr.operand);
    IRValue temp = makeTemp();

    emit(std::make_unique<IRUnaryOpStruct>(temp, convertUnaryOp(expr.op), operand));

    return temp;
}

IRValue IRGenerate::lowerIntegerLiteralExpr(const IntegerLiteralExpr& expr){
    IRValue literal;
    literal.kind = IRValueKind::IntegerConstant;
    literal.intValue = expr.value;
    return literal;
}

IRValue IRGenerate::lowerBoolLiteralExpr(const BoolLiteralExpr& expr){
    IRValue literal;
    literal.kind = IRValueKind::BoolConstant;
    literal.boolValue = expr.value;
    return literal;
}

IRValue IRGenerate::lowerVariableExpr(const VariableExpr& expr){
    IRValue variable;
    variable.kind = IRValueKind::Variable;
    variable.name = expr.name;
    return variable;
}

//nice! 
IRValue IRGenerate::lowerCallExpr(const CallExpr& expr){

    std::vector<IRValue> args;

    for(const auto& argument : expr.arguments){
        IRValue arg = lowerExpr(*argument);
        args.push_back(arg);
    }

    IRValue temp = makeTemp();
    emit(std::make_unique<IRCall>(temp, expr.funcName, args));

    return temp;
}


IRValue IRGenerate::lowerExpr(const Expr& expr){
    if(auto intLitExpr = dynamic_cast<const IntegerLiteralExpr*>(&expr)) return lowerIntegerLiteralExpr(*intLitExpr);
    else if(auto boolLitExpr = dynamic_cast<const BoolLiteralExpr*>(&expr)) return lowerBoolLiteralExpr(*boolLitExpr);
    else if(auto varExpr = dynamic_cast<const VariableExpr*>(&expr)) return lowerVariableExpr(*varExpr);
    else if(auto binaryExpr = dynamic_cast<const BinaryExpr*>(&expr)) return lowerBinaryExpr(*binaryExpr);
    else if(auto unaryExpr = dynamic_cast<const UnaryExpr*>(&expr)) return lowerUnaryExpr(*unaryExpr);
    else if(auto callExpr = dynamic_cast<const CallExpr*>(&expr)) return lowerCallExpr(*callExpr);
    else throw std::runtime_error("unknown expression");
}


void IRGenerate::lowerStmt(const Stmt& stmt){
    if(auto letStmt = dynamic_cast<const LetStmt*>(&stmt)) lowerLetStmt(*letStmt);
    else if(auto assignStmt = dynamic_cast<const AssignStmt*>(&stmt)) lowerAssignStmt(*assignStmt);
    else if(auto ifStmt = dynamic_cast<const IfStmt*>(&stmt)) lowerIfStmt(*ifStmt);
    else if(auto whileStmt = dynamic_cast<const WhileStmt*>(&stmt)) lowerWhileStmt(*whileStmt);
    else if(auto sendStmt = dynamic_cast<const SendStmt*>(&stmt)) lowerSendStmt(*sendStmt);
    else if(auto exprStmt = dynamic_cast<const ExprStmt*>(&stmt)) lowerExprStmt(*exprStmt);
    else if(auto blockStmt = dynamic_cast<const BlockStmt*>(&stmt)) lowerBlockStmt(*blockStmt);
    else throw std::runtime_error("unknown statement");
}

void IRGenerate::lowerFunction(const FunctionDec& func){

    std::vector<IRParameter> p;
    for(const auto& param : func.parameters){
        p.push_back(IRParameter(param.name, param.type));
    }

    program.functions.push_back(std::make_unique<IRFunction>(func.name, p, func.returnType, std::vector<std::unique_ptr<IRInstruction>>)){}

    IRFunction function =  IRFunction(func.name, p, func.returnType, std::move(instructions);
    currentFunction = &function;
}


