#pragma once
#include "ast.hpp"
#include "ir_generator.hpp"
#include <string>
#include <vector>

class Optimizer{
private:
    IRProgram& program;
    
    bool folder(IRFunction& func);
    bool propagator(IRFunction& func);
    // bool eliminator(IRFunction& func);
    bool remover(IRFunction& func);

public:
    explicit Optimizer(IRProgram& program) : program(program) {}
    IRProgram& getProgram(){
        return program;
    }
    void optimize();
};

