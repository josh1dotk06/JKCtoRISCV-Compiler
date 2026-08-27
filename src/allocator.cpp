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

AllocationResult allocateGraph(const InterferenceGraph& graph){
    for(const auto& [name,varnode] : graph.variables){
        
    }
}