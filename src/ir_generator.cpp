#pragma once
#include "ast.hpp"
#include "ir.hpp"
#include <string>
#include <vector>
#include <unordered_map>
//how to traverse + build at the same time
//use the type_checker's recursive descent approach to traverse the ast nodes
//use emit helper function to individually create our IR structure

//ast nodes are to be traversed, ir nodes are to be built

void IRGenerate::lowerLetStmt(const LetStmt& stmt){
    
}