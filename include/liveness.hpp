#pragma once
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include "ir.hpp"
#include "basic_block.hpp"
#include "cfg.hpp"
#include <unordered_map>
#include <unordered_set>
#include <optional>


//were still dealing with blocks in this subphase, its just that the blocks have
//now been reorganized into a CFG.

struct NodeData{
    std::unordered_set<std::string> use;
    std::unordered_set<std::string> def;
    std::unordered_set<std::string> liveIn;
    std::unordered_set<std::string> liveOut;
};

class LivenessAnalysis{
private:
    std::unordered_map<const CFGNode*, NodeData> data;
public:
    void computeUseAndDef(const CFG& cfg);
    bool computeLiveIn(const CFG& cfg);
    bool computeLiveOut(const CFG& cfg);
    void processProgram(const std::unordered_map<std::string, CFG>& cfgs);

    //make these static since we'll need them in interference graph.cpp
    //wont need an object to call
    static std::optional<std::string> getDestination(const IRInstruction* instr);
    static std::vector<std::string> getSources(const IRInstruction* instr);

    LivenessAnalysis() = default;
};

