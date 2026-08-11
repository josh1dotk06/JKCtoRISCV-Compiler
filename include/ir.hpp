//defining each individual IR operation structure

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
    IRValueKind kind;

    std::int64_t intValue = 0;
    bool boolValue = false;
    std::string name;

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

    IRConst(IRValue destination, IRValue value);

    //future 
    void print() const override;
};

struct IRMove : IRInstruction{
    IRValue destination;
    IRValue source;

    IRMove(IRValue destination, IRValue source);

    void print() const override;
};

//BinOp t1 = x < 3
struct IRBinOp : IRInstruction{
    IRValue destination;
    IRBinaryOp op;
    IRValue leftVal;
    IRValue rightVal;

    IRBinOp(IRValue destination, IRBinaryOp op, IRValue leftVal, IRValue rightVal);

    void print() const override;
};

struct IRUnaryOpStruct : IRInstruction{
    IRValue destination;
    IRUnaryOp op;
    IRValue value;

    IRUnaryOpStruct(IRValue destination, IRUnaryOp op, IRValue value);

    void print() const override;
};


struct IRLabel : IRInstruction{
    std::string label;

    explicit IRLabel(std::string label);

    void print() const override;
};

//jump L1

struct IRJump : IRInstruction{
    std::string destination;

    explicit IRJump(std::string destination);

    void print() const override;
};

//branch t1, L_true, L_false
struct IRBranch : IRInstruction{
    IRValue condition;
    std::string trueLabel;
    std::string falseLabel;

    IRBranch(IRValue condition, std::string trueLabel, std::string falseLabel);

    void print() const override;
};

//Call t1 = func(x,y)
struct IRCall : IRInstruction{
    IRValue destination;
    std::string functionName;
    std::vector<IRValue> arguments;

    IRCall(IRValue destination, std::string functionName, std::vector<IRValue> arguments);

    void print() const override;
};

struct IRReturn : IRInstruction{
    IRValue value;
    explicit IRReturn(IRValue value);

    void print() const override;
};













