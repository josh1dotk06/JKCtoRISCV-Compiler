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

void IRGenerate::emit(std::unique_ptr<IRInstruction> instructions){
    (*currentFunction).instructions.push_back(instructions);
}

IRValue IRGenerate::lowerBinaryExpr(const BinaryExpr& expr){
    IRValue temp = makeTemp();
    emit(std::make_unique<IRBinOp>(temp, expr.op, expr.left, expr.right));
}