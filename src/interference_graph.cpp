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
#include "interference_graph.hpp"

void InterferenceGraphBuilder::makeEdge(std::string funcName, std::string var1, std::string var2){
    //push each variables information into the other ones edge vector
    //all the sideeffects we're doing to progGraphs, thus none of the functions have to technically return anything
    //or do any modifications themselves
    if(var1 == var2) return;

    //ensure both nodes exist in the graph
    //they might not exist since a source could be called as var2 
    //when destination is called as var1, before that source 
    //has even been processed in the same instruction

    auto& node1 = progGraphs[funcName].variables[var1];
    auto& node2 = progGraphs[funcName].variables[var2];

    node1.name = var1;
    node2.name = var2;

    node1.edges.insert(&node2);
    node2.edges.insert(&node1);

}

void InterferenceGraphBuilder::buildProgGraphs(const std::unordered_map<std::string, CFG>& cfgs){
    for(const auto& [name, cfg] : cfgs){
        progGraphs[cfg.functionName].functionName = cfg.functionName;
        buildGraph(cfg);
    }
}

void InterferenceGraphBuilder::buildGraph(const CFG& cfg){
    for(const auto& node : cfg.nodes){
        std::unordered_set<std::string> currRegisters = data.at(&node).liveOut; //not tryna modify data, thus use .at instead of []
        for(auto it = node.block->instructions.rbegin(); it != node.block->instructions.rend(); ++it){
            const auto& instruct = *it;
            auto des = LivenessAnalysis::getDestination(instruct);
            
            if(des){
                //if the varnode doesnt exist yet, make it
                //if it does exist, then the bottom 2 lines basically do nothing (dont risk overriding if it already exists)
                auto& varNode = progGraphs[cfg.functionName].variables[*des];
                varNode.name = *des;
                currRegisters.erase(*des);
                for(const auto& var : currRegisters){
                    makeEdge(cfg.functionName,*des, var);
                }
            }   

            auto sources = LivenessAnalysis::getSources(instruct);
            for(const auto& source : sources){
                auto& varNode = progGraphs[cfg.functionName].variables[source];
                varNode.name = source;
                currRegisters.insert(source);
            }
        }
    }
}