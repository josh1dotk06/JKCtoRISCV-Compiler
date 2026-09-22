#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include "ir.hpp"
#include "allocator.hpp"


struct StackFrame {

    //spilled variable (gets offset from sp)
    std::unordered_map<std::string, std::int64_t> spillOffsets;
    //saved s register -> offset from sp
    std::unordered_map<std::string, std::int64_t> savedRegisterOffsets;
    //basically where the old ra is saved
    std::int64_t raOffset = 0;

    std::int64_t frameSize = 0;
};


class RISCVCodeGenerator{
private:
    const IRProgram& program;
    const std::unordered_map<std::string, AllocationResult>& allocations;
    std::ostringstream output;

    ////////STACK FRAME/////////////

    StackFrame buildStackFrame(const IRFunction& function, const AllocationResult& allocation);

    //////////FUNCTIONS///////////////

    void generateFunction(const IRFunction& function);
    void generatePrologue(const IRFunction& function, const AllocationResult& allocation, const StackFrame& frame);

    void generateEpilogue(const IRFunction& function, const StackFrame& frame);


    //////// IR INSTRUCTIONS ///////////////

    void generateInstruction(const IRInstruction* instruction, const IRFunction& function, const AllocationResult& allocation, const StackFrame& frame);
    void generateConst(const IRConst& instruction, const AllocationResult& allocation, const StackFrame& frame);
    void generateMove(const IRMove& instruction, const AllocationResult& allocation, const StackFrame& frame);
    void generateBinOp(const IRBinOp& instruction, const AllocationResult& allocation, const StackFrame& frame);
    void generateUnaryOp(const IRUnaryOpStruct& instruction, const AllocationResult& allocation, const StackFrame& frame);
    void generateLabel(const IRLabel& instruction);
    void generateJump(const IRJump& instruction);
    void generateBranch(const IRBranch& instruction, const AllocationResult& allocation, const StackFrame& frame);
    void generateCall(const IRCall& instruction, const AllocationResult& allocation, const StackFrame& frame);
    void generateReturn(const IRReturn& instruction, const IRFunction& function, const AllocationResult& allocation, const StackFrame& frame);

    std::string loadValue(const IRValue& value, const std::string& scratchRegister, const AllocationResult& allocation, const StackFrame& frame);
    void storeValue(const IRValue& destination, const std::string& sourceRegister, const AllocationResult& allocation, const StackFrame& frame);
    static std::int64_t align16(std::int64_t value);


public:

    RISCVCodeGenerator(const IRProgram& program, const std::unordered_map<std::string, AllocationResult>& allocations) : program(program), allocations(allocations) {}


    void generateProgram();

    std::string getAssembly() const {
        return output.str();
    }
};