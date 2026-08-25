#pragma once
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include "ir.hpp"
#include <unordered_map>


//generate basic blocks
//basic blocks are basically a chunk of instructions that executes straight
//through without branching in the middle
//it separates disjoint parts of the code

//CFGs will group these blocks so that structuring the control flow is simpler

struct BasicBlock{
    std::string name;
    //basic blocks will point to the instructions in IRFunction without transferring ownership
    //but wait: arent those instructions unique_ptrs? Only one thing can point at them
    //nah, they can only have one owner, but multiple raw pointers can point at them, basicblock will merely reference it
    std::vector<const IRInstruction*> instructions;
};

class BasicBlockBuilder{
private:
    //each function corresponds to a set of basicblocks
    std::unordered_map<std::string, std::vector<BasicBlock>> basicBlocks;
public:
    void buildBlocks(const IRFunction& function, const std::string& name);
    void buildBlockStruct(const IRProgram& program);
};



