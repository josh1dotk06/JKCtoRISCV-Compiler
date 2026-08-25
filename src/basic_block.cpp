#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include "ir.hpp"
#include "basic_block.hpp"

void BasicBlockBuilder::buildBlockStruct(const IRProgram& program){
    basicBlocks.clear(); //just in case it gets called more than once
    for(const auto& func : program.functions){
        buildBlocks(*func, func->name);
    }
}

void BasicBlockBuilder::buildBlocks(const IRFunction& function, const std::string& name){
    BasicBlock block;
    for(const auto& instruct : function.instructions){
        auto ret = dynamic_cast<IRReturn*>(instruct.get());
        auto jump = dynamic_cast<IRJump*>(instruct.get());
        auto branch = dynamic_cast<IRBranch*>(instruct.get());
        auto label = dynamic_cast<IRLabel*>(instruct.get());

        if(label && !block.instructions.empty()){
            basicBlocks[name].push_back(block);
            block = BasicBlock{}; //same as BasicBlock()
        }

        block.instructions.push_back(instruct.get());

        //terminate current block (but also we added it into the block)
        if(ret || jump || branch){
            basicBlocks[name].push_back(block);
            block = BasicBlock{};
        }
    }
    if(!block.instructions.empty()){
        basicBlocks[name].push_back(block);
    }
}