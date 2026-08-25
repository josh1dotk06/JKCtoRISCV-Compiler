#pragma once
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include "ir.hpp"
#include "basic_block.hpp"
#include <unordered_map>

//individual nodes for our CF graph
struct CFGNode{
    const BasicBlock* block;
    //list of all blocks (or nodes) that can go to IT and that IT can go to
    //obviously within each function for now
    std::vector<CFGNode*> to;
    std::vector<CFGNode*> from;
    CFGNode() = default;
};

//one cfg per function, thus entire structure would prob be a vector of CFGs
struct CFG{
    std::string functionName;
    std::vector<CFGNode> nodes;
    CFG() = default;
};

class CFGBuilder{
private:
    std::unordered_map<std::string, CFG> cfgs;
    CFGNode* getNodeByLabel(CFG& cfg, const std::string& labelName);
public:
    void buildCFGs(const std::unordered_map<std::string, std::vector<BasicBlock>>& basicBlocks);
    
    //per function
    void buildCFG(const std::vector<BasicBlock>& blocks, const std::string& functionName);
};



