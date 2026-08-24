#include "ast.hpp"
#include "ir.hpp"
#include "ir_generator.hpp"
#include "optimizer.hpp"
#include <string>
#include <vector>
#include <memory>
#include <set>
#include <stdexcept>
#include <unordered_map>


void Optimizer::optimize(){
    //a |= b equivalent to a = a | b
    for(const auto& func : program.functions){
        bool changed;
        
        do{
            changed = false;
            changed |= folder(*func);
            changed |= propagator(*func);
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

bool Optimizer::propagator(IRFunction& func){

    //map key is var/temp name, and IRValue is replacement value
    std::unordered_map<std::string, IRValue> knownValues;
    bool changed = false;
    int i = 0;

    for(auto& instruct : func.instructions){
        if(auto des = dynamic_cast<IRMove*>(instruct.get())){
            //first propagate
            if(des->source.kind == IRValueKind::Variable || des->source.kind == IRValueKind::Temporary){
                auto it = knownValues.find(des->source.name);

                if(it != knownValues.end()){
                    des->source = (*it).second; //propagate source with replacement
                    changed = true; //propagation modifies, so changed = true
                }
            }
            knownValues[des->destination.name] = des->source; //update destination with source
            
            for(auto it = knownValues.begin(); it!=knownValues.end(); ){
                if((it->second.kind == IRValueKind::Variable || it->second.kind == IRValueKind::Temporary) && it->second.name == des->destination.name) it = knownValues.erase(it);
                else ++it;
            }
        }
        
        else if(auto* des = dynamic_cast<IRConst*>(instruct.get())){
            for(auto it = knownValues.begin(); it != knownValues.end(); ){
                if((it->second.kind == IRValueKind::Variable || it->second.kind == IRValueKind::Temporary) && it->second.name == des->destination.name) it = knownValues.erase(it);
                else ++it;
            }
            knownValues[des->destination.name] = des->value;
        }

        else if(auto des = dynamic_cast<IRBinOp*>(instruct.get())){
            if(des->leftVal.kind == IRValueKind::Variable || des->leftVal.kind == IRValueKind::Temporary){
                auto it = knownValues.find(des->leftVal.name);
                if(it != knownValues.end()){
                    des->leftVal = (*it).second;
                    changed = true;
                }
            }
            if(des->rightVal.kind == IRValueKind::Variable || des->rightVal.kind == IRValueKind::Temporary){
                auto it = knownValues.find(des->rightVal.name);
                if(it != knownValues.end()){
                    des->rightVal = (*it).second;
                    changed = true;
                }
            }

            for(auto it = knownValues.begin(); it!=knownValues.end(); ){
                if((it->second.kind == IRValueKind::Variable || it->second.kind == IRValueKind::Temporary) && it->second.name == des->destination.name) it = knownValues.erase(it);
                else ++it;
            }
            knownValues.erase(des->destination.name);
            //dont update destination.name with bin result yet
            //save it for later when we constant fold the IRBinOp into a IRConst
        }
        else if(auto des = dynamic_cast<IRUnaryOpStruct*>(instruct.get())){
            if(des->value.kind == IRValueKind::Variable || des->value.kind == IRValueKind::Temporary){
                auto it = knownValues.find(des->value.name);
                if(it != knownValues.end()){
                    des->value = (*it).second;
                    changed = true;
                }                
            }

            for(auto it = knownValues.begin(); it!=knownValues.end(); ){
                if((it->second.kind == IRValueKind::Variable || it->second.kind == IRValueKind::Temporary) && it->second.name == des->destination.name) it = knownValues.erase(it);
                else ++it;
            }
            knownValues.erase(des->destination.name);
        }
        else if(auto des = dynamic_cast<IRCall*>(instruct.get())){
            for(auto& arg : des->arguments){
                if(arg.kind == IRValueKind::Variable || arg.kind == IRValueKind::Temporary){
                    auto it = knownValues.find(arg.name);
                    if(it != knownValues.end()){
                        arg = (*it).second;
                        changed = true;
                    }
                }
            }

            for(auto it = knownValues.begin(); it!=knownValues.end(); ){
                if((it->second.kind == IRValueKind::Variable || it->second.kind == IRValueKind::Temporary) && it->second.name == des->destination.name) it = knownValues.erase(it);
                else ++it;
            }
            knownValues.erase(des->destination.name);
        }
        else if(auto des = dynamic_cast<IRReturn*>(instruct.get())){
            if(des->value.kind == IRValueKind::Variable || des->value.kind == IRValueKind::Temporary){
                auto it = knownValues.find(des->value.name);
                if(it != knownValues.end()){
                    des->value = (*it).second;
                    changed = true;
                }
            }
        }
        else if(auto des = dynamic_cast<IRBranch*>(instruct.get())){
            if(des->condition.kind == IRValueKind::Variable || des->condition.kind == IRValueKind::Temporary){
                auto it = knownValues.find(des->condition.name);
                if(it != knownValues.end()){
                    des->condition = (*it).second;
                    changed = true;
                }
            }
            knownValues.clear();
        }
        else if(auto des = dynamic_cast<IRJump*>(instruct.get())) knownValues.clear();
        else if(auto des = dynamic_cast<IRLabel*>(instruct.get())) knownValues.clear();

    }

    return changed;
}

//all dest inst: cosnt, move, bin, un (not call)
//source: const, move, bin, un, return, branch
//only source: return, branch


// bool Optimizer::eliminator(IRFunction& func){

//     std::set<std::string> needed;
//     bool changed = false;

//     //lambda function can be written within another function (locally defined function)
//     auto addIfVariable = [&](const IRValue& value){
//         if(value.kind == IRValueKind::Variable || value.kind == IRValueKind::Temporary){
//             needed.insert(value.name);
//         }
//     };

//     //handle destination function
//     auto handleDestFunc = [&](const IRValue& value, auto& it) -> bool{
//         if(needed.find(value.name) == needed.end()){
//             it = std::make_reverse_iterator(func.instructions.erase(std::next(it).base()));
//             return true;
//         }
//         return false;
//     };


//     for(auto it = func.instructions.rbegin(); it!=func.instructions.rend(); ){
//         IRInstruction* instruct = (*it).get();

//         if(auto move = dynamic_cast<IRMove*>(instruct)){ 
//             //erase function if destination is not in needed
//             //otherwise, erase the destination from needed, and add its source
//             //also increment to the next iteration
//             if(needed.find(move->destination.name) == needed.end()) changed = handleDestFunc(move->destination, it);
//             else{
//                 needed.erase(move->destination.name);
//                 addIfVariable(move->source);
//                 ++it;
//             }
//         }

//         else if(auto bin = dynamic_cast<IRBinOp*>(instruct)){
//             if(needed.find(bin->destination.name) == needed.end()) changed = handleDestFunc(bin->destination, it);
//             else{ 
//                 needed.erase(bin->destination.name);
//                 addIfVariable(bin->leftVal);
//                 addIfVariable(bin->rightVal);
//                 ++it;
//             }
//         }

//         else if(auto consta = dynamic_cast<IRConst*>(instruct)){
//             if(needed.find(consta->destination.name) == needed.end()) changed = handleDestFunc(consta->destination, it);
//             else{ 
//                 needed.erase(consta->destination.name);
//                 addIfVariable(consta->value);
//                 ++it;
//             }
//         }

//         else if(auto un = dynamic_cast<IRUnaryOpStruct*>(instruct)){
//             if(needed.find(un->destination.name) == needed.end()) changed = handleDestFunc(un->destination, it);
//             else{
//                  needed.erase(un->destination.name);
//                  addIfVariable(un->value);
//                  ++it;
//             }
//         }
//         else if(auto call = dynamic_cast<IRCall*>(instruct)){
//             needed.erase(call->destination.name);

//             for (const auto& arg : call->arguments) addIfVariable(arg);
//             ++it;
//         }
//         else if(auto ret = dynamic_cast<IRReturn*>(instruct)){
//             //return only has source
//             addIfVariable(ret->value);
//             ++it;
//         }
//         else if(auto branch = dynamic_cast<IRBranch*>(instruct)){
//             //branch only has source
//             addIfVariable(branch->condition);
//             ++it;
//         }
//         //jump and label instructions
//         else ++it;
//     }

//     return changed;

// }

//idea: whenever return, jump, or branch statements occur, this means that
//any instructions following this should be removed because by definition they cannot be reached
//UNLESS that instruction is a label, because then it may be able to be reachable from elsewhere so
//it shouldnt be deleted. This ensures the correctness of the remover while also accounting for 
//control flow liveness
bool Optimizer::remover(IRFunction& func){
    bool changed = false;
    for(auto it = func.instructions.begin(); it != func.instructions.end() ;){
         IRInstruction* instruct = (*it).get();
         auto ret = dynamic_cast<IRReturn*>(instruct);
         auto jump = dynamic_cast<IRJump*>(instruct);
         auto branch = dynamic_cast<IRBranch*>(instruct);
         if(ret || jump || branch){
            ++it;
            while(it!=func.instructions.end() && !dynamic_cast<IRLabel*>((*it).get())){
                //shifts vector to current iterator, no need to it++
                it = func.instructions.erase(it);
                changed = true;
            }
         }
         else ++it;
    }
    return changed;
}