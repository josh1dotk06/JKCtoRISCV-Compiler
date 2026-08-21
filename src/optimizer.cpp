#include "ast.hpp"
#include "ir.hpp"
#include "ir_generator.hpp"
#include "optimizer.hpp"
#include <string>
#include <vector>
#include <memory>


void Optimizer::optimize(){
    //a |= b equivalent to a = a | b
    for(const auto& func : program.functions){
        bool changed;
        
        do{
            changed = false;
            changed |= folder(*func);
            changed |= propagator(*func);
            changed |= eliminator(*func);
            changed |= remover(*func);
        } while(changed);
    }
}

bool Optimizer::folder(IRFunction& func){

    int i = 0;
    bool folded = false;
    for(auto& instruct : func.instructions){
        bool changed = false;
        if(auto* bin = dynamic_cast<IRBinOp*>(instruct.get())){ //use get to turn unique_ptr into raw base-clasee pointers, needed for dynamic casting
            std::int64_t numericVal = 0;
            bool boolVal = false;
            std::int64_t rightNum = bin->rightVal.intValue;
            std::int64_t leftNum = bin->leftVal.intValue;
            
            if((*bin).leftVal.kind == IRValueKind::IntegerConstant && (*bin).rightVal.kind == IRValueKind::IntegerConstant){              
                if(bin->op == IRBinaryOp::Add) numericVal = rightNum + leftNum;
                else if(bin->op == IRBinaryOp::Multiply) numericVal = rightNum * leftNum;
                else if(bin->op == IRBinaryOp::Subtract) numericVal = leftNum - rightNum;
                else if(bin->op == IRBinaryOp::Divide){
                    if(rightNum == 0) throw std::runtime_error("cannot divide by 0");
                    numericVal = leftNum/rightNum;
                }
                else if(bin->op == IRBinaryOp::Equal) boolVal = (leftNum == rightNum);
                else if(bin->op == IRBinaryOp::LessThan) boolVal = (leftNum < rightNum);
                else if(bin->op == IRBinaryOp::LessThanEqual) boolVal = (leftNum <= rightNum);
                else if(bin->op == IRBinaryOp::GreaterThan) boolVal = (leftNum > rightNum);
                else if(bin->op == IRBinaryOp::GreaterThanEqual) boolVal = (leftNum >= rightNum);
                changed = true;
            }
            if((*bin).leftVal.kind == IRValueKind::BoolConstant && (*bin).rightVal.kind == IRValueKind::BoolConstant){
                if(bin->op == IRBinaryOp::Equal) boolVal = (bin->leftVal.boolValue == bin->rightVal.boolValue);
                else if(bin->op == IRBinaryOp::Or) boolVal = (bin->leftVal.boolValue || bin->rightVal.boolValue);
                else if(bin->op == IRBinaryOp::And) boolVal = (bin->leftVal.boolValue && bin->rightVal.boolValue);
                changed = true;
            }

            if(changed){
                IRValue result;
                if(bin->op == IRBinaryOp::Add || bin->op == IRBinaryOp::Subtract || bin->op == IRBinaryOp::Multiply || bin->op == IRBinaryOp::Divide) result.kind = IRValueKind::IntegerConstant;
                else result.kind = IRValueKind::BoolConstant;
                result.intValue = numericVal;
                result.boolValue = boolVal;
                func.instructions[i] = std::make_unique<IRConst>(std::move((*bin).destination), result);        
                folded = true;
            } 
        }

        if(auto* un = dynamic_cast<IRUnaryOpStruct*>(instruct.get())){
            bool boolVal = false;
            std::int64_t numericVal = 0;
            IRValue result;
            if(un->value.kind == IRValueKind::BoolConstant){
                if(un->op == IRUnaryOp::Not) boolVal = !(un->value.boolValue);
                changed = true;
                result.kind = IRValueKind::BoolConstant;
            }
            else if(un->value.kind == IRValueKind::IntegerConstant){
                if(un->op == IRUnaryOp::Negation) numericVal = -1*(un->value.intValue);
                changed = true;
                result.kind = IRValueKind::IntegerConstant;
            }
            if(changed){ 
                result.boolValue = boolVal;
                result.intValue = numericVal;
                func.instructions[i] = std::make_unique<IRConst>(std::move(*un).destination, result);
                folded = true;
            }
        }
        i++;
    }
    return folded;
}



