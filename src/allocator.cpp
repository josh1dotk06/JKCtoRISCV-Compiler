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
#include "allocator.hpp"


void RegisterAllocator::allocateProgram(){
    allocations.clear(); //just in case it runs more than once
    for(const auto& [name, graph] : graphs){
        allocateGraph(graph);
    }
}


void RegisterAllocator::allocateGraph(const InterferenceGraph& graph){

    AllocationResult& allocation = allocations[graph.functionName];

    std::stack<std::string> registerStack;
    std::unordered_set<std::string> potentialSpills;

    //simplification phase (pushing onto stack)
    std::unordered_map<std::string, std::int64_t> degree;
    std::unordered_set<std::string> remaining; //remaining nodes with deg < k

    for(auto& [name,varnode] : graph.variables){
        degree[name] = varnode.edges.size();
        remaining.insert(name);
    }

    //must keep repeating, since we decrease degree, some nodes may meet the
    //deg < k condition even after it has already been processed
    while(!remaining.empty()){
        auto chosen = remaining.end();

        //finding nodes of degree < k
        for(auto it = remaining.begin(); it != remaining.end(); ++it){
            if(degree[*it] < k){
                chosen = it;
                break;
            }
        }

        //if there are none then its potential spill
        if(chosen == remaining.end()){
            chosen=remaining.begin();
            potentialSpills.insert(*chosen);
        }

        //update
        std::string name = *chosen;
        registerStack.push(name);
        remaining.erase(name);

        //remove node from graph by decreasing degree of neighboring nodes to this node
        for(const auto* neighbor : graph.variables.at(name).edges){
            if(remaining.find(neighbor->name) != remaining.end()) degree[neighbor->name]--;
        }
    }

    //========================================================================================================

    //coloring phase
    while(!registerStack.empty()){
        std::string name = registerStack.top();
        registerStack.pop();
        auto& varnode = graph.variables.at(name); //graph is const, needs .at
        std::unordered_set<std::string> copySet = availableRegisters;

        //check for register usage by its neighboring nodes, it cannot
        //share the same register so remove it for any neighboring nodes
        for(const auto& neighbors : varnode.edges){
            //might exist, might not, we just need all available registers after processing every neighbor
            
            auto it = allocation.registers.find(neighbors->name);
            if(it!=allocation.registers.end()) copySet.erase(it->second);
        }

        //any random register
        if(copySet.size() != 0) allocation.registers[name] = *copySet.begin();
        else allocation.spilled.insert(name);

    }
}