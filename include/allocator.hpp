#pragma once
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include "ir.hpp"
#include "basic_block.hpp"
#include "cfg.hpp"
#include "liveness.hpp"
#include "interference_graph.hpp"
#include <stack>
#include <unordered_map>
#include <unordered_set>

//turning program variables to physical risc v machine registers through graph coloring

//tutorial used
//https://view.officeapps.live.com/op/view.aspx?src=https%3A%2F%2Fwww.cs.princeton.edu%2Fcourses%2Farchive%2Fspr05%2Fcos320%2Fnotes%2FRegister%2520Allocation.ppt&wdOrigin=BROWSELINK

//colors represent the register
//obviously 2 of the same color cant be beside each other on the graph (they interfere)
//and we will need to spill if no color is available

//all the allocation information regarding an individual interference graph
struct AllocationResult{
    std::unordered_map<std::string, std::string> registers; //variable mapped to a risc V register
    std::unordered_set<std::string> spilled;
};

class RegisterAllocator{
private:
    const std::unordered_map<std::string, InterferenceGraph>& graphs;
    
    //using only the pool of s1-s11 risc v registers (i.e the callee-saved registers)
    //these registers are preserved across function calls 
    std::vector<std::string> availableRegisters = {
    "s1", "s2", "s3", "s4", "s5", "s6",
    "s7", "s8", "s9", "s10", "s11"
    };

    std::unordered_map<std::string, AllocationResult> allocations;

    AllocationResult allocateGraph(const InterferenceGraph& graph);

public:
    RegisterAllocator(const std::unordered_map<std::string, InterferenceGraph>& graphs) : graphs(graphs) {}
    
    void allocateProgram();
    const std::unordered_map<std::string, AllocationResult>& getAllocations() const{
        return allocations;
    }
};