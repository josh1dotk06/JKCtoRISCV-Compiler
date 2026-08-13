#include "ir.hpp"
#include <utility>

//static helper for unary
static std::string getIRUnaryOpName(IRUnaryOp op){
    switch(op){
        case IRUnaryOp::Negation:
            return "-";
        case IRUnaryOp::Not:
            return "NOT";
    }
    return "unknown";
}

//static helper
static std::string getIRBinaryOpName(IRBinaryOp op){
    switch (op) {
        case IRBinaryOp::Add:
            return "+";

        case IRBinaryOp::Subtract:
            return "-";

        case IRBinaryOp::Multiply:
            return "*";

        case IRBinaryOp::Divide:
            return "/";

        case IRBinaryOp::Equal:
            return "==";

        case IRBinaryOp::LessThan:
            return "<";

        case IRBinaryOp::LessThanEqual:
            return "<=";

        case IRBinaryOp::GreaterThan:
            return ">";

        case IRBinaryOp::GreaterThanEqual:
            return ">=";

        case IRBinaryOp::And:
            return "AND";

        case IRBinaryOp::Or:
            return "OR";
    }

    return "unknown";
}

//visual representation of our IR
void IRBinOp::print() const{
    std::cout << "BinOp " << destination.toString() << " = " << leftVal.toString() << " " << getIRBinaryOpName(op) << " " << rightVal.toString() << '\n';
}

//const x, 4
void IRConst::print() const{
    std::cout << "Const " << destination.toString() << ", " << value.toString() << '\n';
}

void IRMove::print() const{
    std::cout << "Move " << destination.toString() << ", " << source.toString() << '\n';
}

void IRUnaryOpStruct::print() const{
    std::cout << "UnaryOp " << destination.toString() << " = " << getIRUnaryOpName(op) << " " << value.toString() << '\n';
}

void IRLabel::print() const{
    std::cout << "Label " << label << '\n';
}

void IRJump::print() const{
    std::cout << "Jump " << destination << '\n';
}

void IRBranch::print() const{
    std::cout << "Branch " << condition.toString() << ", " << trueLabel << ", " << falseLabel << '\n';
}

void IRCall::print() const{
    std::cout << "Call " << destination.toString() << " = " << functionName << "(";
    for(int i = 0; i < arguments.size(); ++i){
        if(i == arguments.size() - 1){
            std::cout << arguments[i].toString();
            break;
        }
        std::cout << arguments[i].toString() << ", ";
    }
    std::cout << ")" << '\n';
}

void IRReturn::print() const{
    std::cout << "Return " << value.toString();
}
