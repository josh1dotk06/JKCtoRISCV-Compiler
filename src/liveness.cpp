#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include "ir.hpp"
#include "basic_block.hpp"
#include <unordered_map>
#include <unordered_set>
#include "cfg.hpp"
#include "liveness.hpp"
#include <optional>

//get variable name for the destination variable in the instruction
std::optional<std::string> LivenessAnalysis::getDestination(const IRInstruction* instr){
    if(auto x = dynamic_cast<const IRConst*>(instr))
        return x->destination.name;

    if(auto x = dynamic_cast<const IRMove*>(instr))
        return x->destination.name;

    if(auto x = dynamic_cast<const IRBinOp*>(instr))
        return x->destination.name;

    if(auto x = dynamic_cast<const IRUnaryOpStruct*>(instr))
        return x->destination.name;

    if(auto x = dynamic_cast<const IRCall*>(instr))
        return x->destination.name;
    return std::nullopt;
}

//get variable names for the sources in the instruction
//sources U destinations = all variables
std::vector<std::string> LivenessAnalysis::getSources(const IRInstruction* instr) {
    std::vector<std::string> sources;

    auto add = [&](const IRValue& v){
        if(v.kind == IRValueKind::Variable || v.kind == IRValueKind::Temporary) sources.push_back(v.name);
    };

    if(auto x = dynamic_cast<const IRMove*>(instr)) add(x->source);

    else if(auto x = dynamic_cast<const IRBinOp*>(instr)){
        add(x->leftVal);
        add(x->rightVal);
    }
    else if (auto x = dynamic_cast<const IRUnaryOpStruct*>(instr)) add(x->value);
    else if (auto x = dynamic_cast<const IRBranch*>(instr)) add(x->condition);
    else if (auto x = dynamic_cast<const IRCall*>(instr))
        for(const auto& arg : x->arguments) add(arg);

    else if(auto x = dynamic_cast<const IRReturn*>(instr)) add(x->value);
    return sources;
}


void LivenessAnalysis::processProgram(const std::unordered_map<std::string, CFG>& cfgs){
    data.clear(); //just in case it gets run multiple times in one session
    for(const auto& [name, cfg] : cfgs){
        computeUseAndDef(cfg);
        bool changed = false;
        do{
            changed = false;
            changed |= computeLiveOut(cfg);
            changed |= computeLiveIn(cfg);
        } while(changed);
    }
}

//use contains all variables that are used in this block which are
//not yet defined in this block
void LivenessAnalysis::computeUseAndDef(const CFG& cfg){
    //process each node within the INDIVIDUAL cfg
    for(const auto& node : cfg.nodes){
        const BasicBlock& block = *node.block;
        std::unordered_set<std::string> definedSoFar;

        //in one pass
        for(const auto& instruct : block.instructions){
            auto sources = getSources(instruct);

            for(const auto& source : sources){
                if(definedSoFar.find(source) == definedSoFar.end()) data[&node].use.insert(source);
            }

            auto dest = getDestination(instruct);
            if(dest){
                definedSoFar.insert(*dest);
                data[&node].def.insert(*dest);
            }
        }
    }
}

//all USE variables are also in liveIn
//as well, they could be in liveOut - def, which is the same as saying
//they must be in liveOut but not in def
//requires multiple passes to completely fill up liveIn hence why its bool

//logical formula: liveIn = use ∪ (liveOut - def)
bool LivenessAnalysis::computeLiveIn(const CFG& cfg){
    bool changed = false;
    for(const auto& node : cfg.nodes){
        for(const auto& variable : data[&node].use){
            if(data[&node].liveIn.insert(variable).second) changed = true;
        }

        for(const auto& variable : data[&node].liveOut){
            if(data[&node].def.find(variable) == data[&node].def.end()){
                if(data[&node].liveIn.insert(variable).second) changed = true;
            }
        }
    }

    return changed;
}

//logical formula: liveOut[B] = ⋃ liveIn[S] for each successor S of B
bool LivenessAnalysis::computeLiveOut(const CFG& cfg){
    bool changed = false;
    for(const auto& node : cfg.nodes){
        for(const auto& nextNode : node.to){
            for(const auto& variable : data[nextNode].liveIn){
                if(data[&node].liveOut.insert(variable).second) changed = true;
            }
        }
    }
    return changed;
}