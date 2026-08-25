#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include "ir.hpp"
#include "basic_block.hpp"
#include <unordered_map>
#include "cfg.hpp"

CFGNode* CFGBuilder::getNodeByLabel(CFG& cfg, const std::string& labelName){

    for(auto& node : cfg.nodes){
        if(node.block->instructions.empty()){
            continue;
        }

        const IRInstruction* frontInstruct = node.block->instructions.front();
        const IRLabel* label = dynamic_cast<const IRLabel*>(frontInstruct);
        if(label && label->label == labelName) return &node;
    }
    return nullptr;
}


void CFGBuilder::buildCFGs(const std::unordered_map<std::string, std::vector<BasicBlock>>& basicBlocks){
    
    for(const auto& [name, blocks] : basicBlocks) buildCFG(blocks, name); 
}

void CFGBuilder::buildCFG(const std::vector<BasicBlock>& blocks, const std::string& functionName){

    //prevent copying cfg so no dangling pointers: use reference to refer to the one that exists in cfgs
    CFG& cfg = cfgs[functionName]; //creates one if cfgs[functionName] doesnt exist yet
    cfg.functionName = functionName;

    cfg.nodes.clear(); //just in case we run this more than once

    //cfgs[functionName];
    for(const auto& currBlock : blocks){
        CFGNode node;
        node.block = &currBlock;
        cfg.nodes.push_back(node);
    }

    int i = 0;
    for(auto& currNode : cfg.nodes){
        const IRInstruction* frontInstruct = currNode.block->instructions.front();
        const IRLabel* label = dynamic_cast<const IRLabel*>(frontInstruct);
        const IRInstruction* currBackInstruct = currNode.block->instructions.back();

        if(label){
            for(auto& fromNode : cfg.nodes){
                const IRInstruction* backInstruct = fromNode.block->instructions.back();
                const IRBranch* branch = dynamic_cast<const IRBranch*>(backInstruct);
                const IRJump* jump = dynamic_cast<const IRJump*>(backInstruct);
                if(branch && (branch->falseLabel == label->label || branch->trueLabel == label->label)){
                    currNode.from.push_back(&fromNode);
                }
                else if(jump && jump->destination == label->label){
                    currNode.from.push_back(&fromNode);
                }
            }
        }

        //now fill in the to vector for this function
        //this vector can only contain at most 2 other cfg nodes (if its a branch instruction at the end)
        const IRBranch* branch = dynamic_cast<const IRBranch*>(currBackInstruct);
        const IRJump* jump = dynamic_cast<const IRJump*>(currBackInstruct);
        const IRReturn* ret = dynamic_cast<const IRReturn*>(currBackInstruct);

        if(branch){
            currNode.to.push_back(getNodeByLabel(cfg, branch->trueLabel));
            currNode.to.push_back(getNodeByLabel(cfg, branch->falseLabel));
        }
        else if(jump){
            currNode.to.push_back(getNodeByLabel(cfg, jump->destination));
        }
        //fallthrough, meaning it stopped on a label and not a terminating instruction
        else if(!jump && !ret && !branch && i + 1 < cfg.nodes.size()){
            currNode.to.push_back(&cfg.nodes[i + 1]);
            //handles the from as well
            cfg.nodes[i + 1].from.push_back(&currNode);
        }
        //return functions will do nothing to the "to" vector
        i++;
    }
}