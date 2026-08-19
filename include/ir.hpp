//defining each individual IR operation structure and the containers to hold them
//containers: {program, function}

#pragma once
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "ast.hpp" //jkc type

enum class IRBinaryOp{
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

enum class IRUnaryOp{
    Not,
    Negation
};

enum class IRValueKind{
    IntegerConstant,
    BoolConstant,
    Variable,
    Temporary
};

//can be t1, t0, x, y, true, 43, whatever
struct IRValue{
    IRValueKind kind = IRValueKind::Temporary;

    std::int64_t intValue = 0;
    bool boolValue = false;
    std::string name;

    IRValue() = default;
    std::string toString() const;
};


//base abstract ir instruction similar to ASTNode
struct IRInstruction{
    virtual ~IRInstruction() = default;
    virtual void print() const = 0;
};

struct IRConst : IRInstruction{
    IRValue destination;
    IRValue value;

    IRConst(IRValue destination, IRValue value) : destination(std::move(destination)), value(std::move(value)) {}

    //future 
    void print() const override;
};

struct IRMove : IRInstruction{
    IRValue destination;
    IRValue source;

    IRMove(IRValue destination, IRValue source) : destination(std::move(destination)), source(std::move(source)) {}

    void print() const override;
};

//BinOp t1 = x < 3
struct IRBinOp : IRInstruction{
    IRValue destination;
    IRBinaryOp op;
    IRValue leftVal;
    IRValue rightVal;

    IRBinOp(IRValue destination, IRBinaryOp op, IRValue leftVal, IRValue rightVal) : destination(std::move(destination)), op(op), leftVal(std::move(leftVal)), rightVal(std::move(rightVal)) {}

    void print() const override;
};

struct IRUnaryOpStruct : IRInstruction{
    IRValue destination;
    IRUnaryOp op;
    IRValue value;

    IRUnaryOpStruct(IRValue destination, IRUnaryOp op, IRValue value) : destination(std::move(destination)), op(op), value(std::move(value)) {}

    void print() const override;
};


struct IRLabel : IRInstruction{
    std::string label;

    explicit IRLabel(std::string label) : label(std::move(label)) {}

    void print() const override;
};

//jump L1

struct IRJump : IRInstruction{
    std::string destination;

    explicit IRJump(std::string destination) : destination(std::move(destination)) {}

    void print() const override;
};

//branch t1, L_true, L_false
struct IRBranch : IRInstruction{
    IRValue condition;
    std::string trueLabel;
    std::string falseLabel;

    IRBranch(IRValue condition, std::string trueLabel, std::string falseLabel) : condition(std::move(condition)), trueLabel(std::move(trueLabel)), falseLabel(std::move(falseLabel)) {}

    void print() const override;
};

//Call t1 = func(x,y)
struct IRCall : IRInstruction{
    IRValue destination;
    std::string functionName;
    std::vector<IRValue> arguments;

    IRCall(IRValue destination, std::string functionName, std::vector<IRValue> arguments) : destination(std::move(destination)), functionName(std::move(functionName)), arguments(std::move(arguments)) {}

    void print() const override;
};

struct IRReturn : IRInstruction{
    IRValue value;
    explicit IRReturn(IRValue value) : value(std::move(value)) {}

    void print() const override;
};

struct IRParameter{
    std::string name;
    JKCType type;

    IRParameter(std::string name, JKCType type) : name(std::move(name)), type(type) {}
};

//functions that contian each set of instructions for a specific function (like main)
struct IRFunction{
    std::string name;
    std::vector<IRParameter> parameters;
    JKCType returnType;
    //unique ptr because of polymorphism, there are various IR operations
    std::vector<std::unique_ptr<IRInstruction>> instructions;

    IRFunction(std::string name, std::vector<IRParameter> parameters, JKCType returnType, std::vector<std::unique_ptr<IRInstruction>> instructions) : name(std::move(name)), parameters(std::move(parameters)), returnType(returnType), instructions(std::move(instructions)) {} 

    void print() const;
};

struct IRProgram{
    std::vector<std::unique_ptr<IRFunction>> functions;

    void print() const;
};

//IRProgram and IRFunction are not operations, they are containers that will
//organize the instructions













