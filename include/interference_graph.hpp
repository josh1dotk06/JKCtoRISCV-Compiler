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

//interference graph algorithm:
/*
For each node, take the liveOut set
we create a new set that represents all variables accessible within the registers
for each node, we start at the last instruction

for each instruction
when we encounter a destination variable, it will then interfere
with every other variable in the register set, thus we create an edge betwene this variable and all other variables in the register set
this variable would have existed in the register set but after this instruction is processed, it is removed (thats because we're going backwards so it wouldnt have been defined yet)
we append the source variables to the register set because since we're going backwards, clearly we know that these variables will be used

going backwards is like going back in time, we already know everything about the current instruction's variables before its effects happen
*/

