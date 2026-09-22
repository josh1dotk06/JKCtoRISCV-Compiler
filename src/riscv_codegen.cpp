#include "riscv_codegen.hpp"
#include <stdexcept>


std::int64_t RISCVCodeGenerator::align16(std::int64_t value) {
    return (value + 15) / 16 * 16;
}


StackFrame RISCVCodeGenerator::buildStackFrame(const IRFunction& function, const AllocationResult& allocation) {

    //each stack slot is 8 byte since JKC deals with 64 bit ints

    StackFrame frame;
    //reserve space for ra
    //whihc s registers are used??
    //reserve space for those saved registers
    //reserve space for spilled variables
    //align frame size to 16 by

    std::int64_t offset = 0;

    //save ra
    frame.raOffset = offset;
    offset+=8;

    //save every s reg used in the curr function
    for(const auto& pair : allocation.registers){
        const std::string& reg = pair.second;
        //not saved yet, then assign the register an offset
        if(frame.savedRegisterOffsets.find(reg) == frame.savedRegisterOffsets.end()){
            frame.savedRegisterOffsets[reg] = offset;
            offset+=8;
        }
    }

    //give spilled variables their own stack slot
    for(const std::string& variable : allocation.spilled){
        frame.spillOffsets[variable] = offset;
        offset+=8;
    }

    //16 byte aligned, for risc v stack
    //take offset and round to the next multiple of 16, sp requires to be 16 byte aligned
    frame.frameSize = align16(offset);

    return frame;
}


void RISCVCodeGenerator::generateProgram() {

    //for the assembler, it needs to know .text at the top
    output << ".text\n";
    for(const auto& function : program.functions){
        generateFunction(*function);
    }
}


void RISCVCodeGenerator::generateFunction(const IRFunction& function) {

    //get this functions AllocationResult
    //then build the stack frame 
    //emit func label
    //prologue
    //handle params
    //gen instructions
    //epilogue

    auto allocationIt = allocations.find(function.name);

    //make sure it exists
    if(allocationIt == allocations.end()){
        throw std::runtime_error("No allocation found for function: " + function.name);
    }

    //get the og allocation result
    const AllocationResult& allocation = allocationIt->second;

    StackFrame frame = buildStackFrame(function, allocation);

    //function label: like main:
    output << "\n.globl " << function.name << "\n";
    output << function.name << ":\n";

    generatePrologue(function, allocation, frame);

    //move incoming ai args into their allocated locations
    for(std::size_t i = 0; i < function.parameters.size(); i++){
        if(i>=8) throw std::runtime_error("more than 8 parameters not supported");
    
        //set up parameter IRValue
        IRValue parameter;
        parameter.kind = IRValueKind::Variable;
        parameter.name = function.parameters[i].name;
        //move whatevers in ai into register associated with parameter
        //this passes inthe parameter values into the function
        storeValue(parameter, "a"+std::to_string(i), allocation, frame);
    }

    //generate bodyt
    //similar descent style
    for(const auto& instruction : function.instructions){
        generateInstruction(instruction.get(), function, allocation, frame);
    }

    //returns jump to here
    output << function.name << "_epilogue:\n";
    generateEpilogue(function, frame);
}

//create frame and save old CPU state
void RISCVCodeGenerator::generatePrologue(const IRFunction& function, const AllocationResult& allocation, const StackFrame& frame) {

    //move sp down frameSize bytes, meaning reserve frameSize bytes for curr function or whatever
    output << "    addi sp, sp, -" << frame.frameSize << "\n";
    //save return address to top of stack (the raoffset), so we know where to return to
    output << "    sd ra, " << frame.raOffset << "(sp)\n";

    //save every s reg this function plans to use
    //save callers old si registers before this function overrwrites them
    //save them by putting them onto stack
    for(const auto& pair : frame.savedRegisterOffsets){
        output << "    sd " << pair.first << ", " << pair.second << "(sp)\n";
    }
}

//restore old state
void RISCVCodeGenerator::generateEpilogue(const IRFunction& function, const StackFrame& frame) {

   for(const auto& pair : frame.savedRegisterOffsets){
        output << "    ld " << pair.first << ", " << pair.second << "(sp)\n";
   }

   //load back ra after finished
   output << "    ld ra, " << frame.raOffset << "(sp)\n";
   output << "    addi sp, sp, " << frame.frameSize << "\n";
   output << "    ret\n";

}

//dynamic dispatcher
void RISCVCodeGenerator::generateInstruction(const IRInstruction* instruction, const IRFunction& function, const AllocationResult& allocation, const StackFrame& frame){

    if(auto constInst = dynamic_cast<const IRConst*>(instruction)) generateConst(*constInst, allocation, frame);
    else if(auto moveInst = dynamic_cast<const IRMove*>(instruction)) generateMove(*moveInst, allocation, frame);
    else if(auto binOpInst = dynamic_cast<const IRBinOp*>(instruction)) generateBinOp(*binOpInst, allocation, frame);
    else if(auto unaryInst = dynamic_cast<const IRUnaryOpStruct*>(instruction)) generateUnaryOp(*unaryInst, allocation, frame);
    else if(auto labelInst = dynamic_cast<const IRLabel*>(instruction)) generateLabel(*labelInst);
    else if(auto jumpInst = dynamic_cast<const IRJump*>(instruction)) generateJump(*jumpInst);
    else if(auto branchInst = dynamic_cast<const IRBranch*>(instruction)) generateBranch(*branchInst, allocation, frame);
    else if(auto callInst = dynamic_cast<const IRCall*>(instruction)) generateCall(*callInst, allocation, frame);
    else if(auto returnInst = dynamic_cast<const IRReturn*>(instruction)) generateReturn(*returnInst, function, allocation, frame);
    else throw std::runtime_error("Unknown IR instruction");
}


void RISCVCodeGenerator::generateConst(const IRConst& instruction, const AllocationResult& allocation, const StackFrame& frame) {

    std::int64_t value;

    if(instruction.value.kind == IRValueKind::IntegerConstant) value = instruction.value.intValue;
    else if(instruction.value.kind == IRValueKind::BoolConstant) value = instruction.value.boolValue ? 1 : 0;
    else throw std::runtime_error("IRConst value is not a constant");

    const std::string& destination = instruction.destination.name;

    auto registerIt = allocation.registers.find(destination);

    //load immediate into an available register
    if(registerIt != allocation.registers.end()){
        output << "    li " << registerIt->second << ", " << value << "\n";
        return;
    }

    auto spillIt = frame.spillOffsets.find(destination);

    if(spillIt != frame.spillOffsets.end()){
        output << "    li t0, " << value << "\n";
        //not enough registers, so we gotta send these spilled values onto the stack
        output << "    sd t0, " << spillIt->second << "(sp)\n";
        return;
    }

    throw std::runtime_error("No location allocated for: " + destination);

}

//destination = source, i.e read y and place in x
void RISCVCodeGenerator::generateMove(const IRMove& instruction, const AllocationResult& allocation, const StackFrame& frame) {

    //return register
    //use t0 as fallback if spilled on stack/is imediate (otherwise give register)
    std::string sourceRegister = loadValue(instruction.source, "t0", allocation, frame);
    storeValue(instruction.destination, sourceRegister, allocation, frame);
}


void RISCVCodeGenerator::generateBinOp(const IRBinOp& instruction, const AllocationResult& allocation, const StackFrame& frame) {

    //get register for left and right ops
    std::string leftReg = loadValue(instruction.leftVal, "t0", allocation, frame);
    std::string rightReg = loadValue(instruction.rightVal, "t1", allocation, frame);

    //put resultant in temp 2
    switch(instruction.op){
        case IRBinaryOp::Add:
            output << "    add t2, " << leftReg << ", " << rightReg << "\n";
            break;

        case IRBinaryOp::Subtract:
            output << "    sub t2, " << leftReg << ", " << rightReg << "\n";
            break;

        case IRBinaryOp::Multiply:
            output << "    mul t2, " << leftReg << ", " << rightReg << "\n";
            break;

        case IRBinaryOp::Divide:
            output << "    div t2, " << leftReg << ", " << rightReg << "\n";
            break;

        //xor + seqz method
        case IRBinaryOp::Equal:
            output << "    xor t2, " << leftReg << ", " << rightReg << "\n";
            output << "    seqz t2, t2\n";
            break;

        case IRBinaryOp::LessThan:
            output << "    slt t2, " << leftReg << ", " << rightReg << "\n";
            break;

        case IRBinaryOp::GreaterThan:
            output << "    slt t2, " << rightReg << ", " << leftReg << "\n";
            break;

        case IRBinaryOp::LessThanEqual:
            output << "    slt t2, " << rightReg << ", " << leftReg << "\n";
            output << "    xori t2, t2, 1\n";
            break;

        case IRBinaryOp::GreaterThanEqual:
            output << "    slt t2, " << leftReg << ", " << rightReg << "\n";
            output << "    xori t2, t2, 1\n";
            break;

        case IRBinaryOp::And:
            output << "    and t2, " << leftReg << ", " << rightReg << "\n";
            break;

        case IRBinaryOp::Or:
            output << "    or t2, " << leftReg << ", " << rightReg << "\n";
            break;
    }

    storeValue(instruction.destination, "t2", allocation, frame);
}


void RISCVCodeGenerator::generateUnaryOp(const IRUnaryOpStruct& instruction, const AllocationResult& allocation, const StackFrame& frame) {

    std::string operand = loadValue(instruction.value, "t0", allocation, frame);

    switch(instruction.op){
        case IRUnaryOp::Negation:
            output << "    neg t1, " << operand << "\n";
            break;
        
            //seqz, not "not", that is bw not
        case IRUnaryOp::Not:
            output << "    seqz t1, " << operand << "\n";
            break;
    }

    storeValue(instruction.destination, "t1", allocation, frame);
}


void RISCVCodeGenerator::generateLabel(const IRLabel& instruction) {

    //label, then some block beneath it
    output << instruction.label << ":\n";
}


void RISCVCodeGenerator::generateJump(const IRJump& instruction) {

    //unconditional back to the destination
    output << "    j " << instruction.destination << "\n";
}


void RISCVCodeGenerator::generateBranch(const IRBranch& instruction, const AllocationResult& allocation, const StackFrame& frame) {
    std::string conditionReg = loadValue(instruction.condition, "t0", allocation, frame);

    //branch if != 0
    output << "    bnez " << conditionReg << ", " << instruction.trueLabel << "\n";
    //auto jump to false
    output << "    j "  << instruction.falseLabel << "\n";
    
}

//e.g call t1 = func(x,y)
void RISCVCodeGenerator::generateCall(const IRCall& instruction, const AllocationResult& allocation, const StackFrame& frame) {

    if(instruction.arguments.size() > 8){
        throw std::runtime_error("More than 8 arguments isn't supported currently");
    }

    //put each arg into a0-a7
    int i = 0;
    for(const auto& arg : instruction.arguments){
        std::string argRegister = loadValue(arg, "t0", allocation, frame);
        //take the loaded argRegister and move the value into ai (arg reg)
        output << "    mv a" << i << ", " << argRegister << "\n";
        ++i;
    }

    output << "    call " << instruction.functionName << "\n";

    //store the ret value (which is in a0), into dest
    storeValue(instruction.destination, "a0", allocation, frame);
}


void RISCVCodeGenerator::generateReturn(const IRReturn& instruction, const IRFunction& function, const AllocationResult& allocation, const StackFrame& frame) {

    //move register to a0, the return value
    std::string valueReg = loadValue(instruction.value, "t0", allocation, frame);

    output << "    mv a0, " << valueReg << "\n";
    //restore epilogue of the curr function
    //restores things like return address (ra), and sp
    //j is unconditional jump
    output << "    j " << function.name << "_epilogue\n";

}

//give register containig this ir val
//if allocated, then return that register, if spilled, ld into scratch reg (t0), if imm then li into t0
std::string RISCVCodeGenerator::loadValue(const IRValue& value, const std::string& scratchRegister, const AllocationResult& allocation, const StackFrame& frame) {

    //IMMEDIATE CASE::::::: --->>  just li into scratch (like we just said)
    if(value.kind == IRValueKind::IntegerConstant){
        output << "    li " << scratchRegister << ", " << value.intValue << "\n";
        return scratchRegister;
    }
    if(value.kind == IRValueKind::BoolConstant){
        output << "    li " << scratchRegister << ", " << (value.boolValue ? 1 : 0) << "\n";
        return scratchRegister;
    }


    //ALREADY ALLOCATED CASE 0000000 ------------->>>> find it and return the reg
    auto registerIt = allocation.registers.find(value.name);

    if(registerIt != allocation.registers.end()){
        return registerIt->second;
    }

    //SPILLED CASE 00000000000 ------------------>>>>>> lo0ad from curr stack (sp) into scratch
    auto spillIt = frame.spillOffsets.find(value.name);

    if(spillIt != frame.spillOffsets.end()){
        output << "    ld " << scratchRegister << ", " << spillIt->second << "(sp)\n";
        return scratchRegister;
    }

    throw std::runtime_error("No location for value: " + value.name);
}


//take cpu register containing some result and store that value back into the location of IRValue
//if the IRValue is spilled, its not associated with any register, its on stack (use sd)
void RISCVCodeGenerator::storeValue(const IRValue& destination, const std::string& sourceRegister, const AllocationResult& allocation, const StackFrame& frame) {

    //register destination: mv into it
    auto registerIt = allocation.registers.find(destination.name);
    if(registerIt != allocation.registers.end()){
        output << "    mv " << registerIt->second << ", " << sourceRegister << "\n";
        return;
    }

    //spill case: sd onto stack
    auto spillIt = frame.spillOffsets.find(destination.name);

    if(spillIt != frame.spillOffsets.end()){
        output << "    sd " << sourceRegister << ", " << spillIt->second << "(sp)\n";
        return;
    }


    throw std::runtime_error("no location allocated for: " + destination.name);
}