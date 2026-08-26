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
    void computeUseAndDef(CFG& cfg);
    bool computeLiveIn(CFG& cfg);
    bool computeLiveOut(CFG& cfg);
    void processProgram(std::unordered_map<std::string, CFG> cfgs);
    LivenessAnalysis() = default;
};

