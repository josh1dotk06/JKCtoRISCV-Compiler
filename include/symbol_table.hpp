#pragma once

//create symbol table data structure and all its useful methods

#include <string>
#include <vector>
#include <unordered_map>
#include "ast.hpp" //we need jkc type

enum class VariableKind{
    LOCAL,
    PARAMETER
};

//info on each variable
struct VariableSymbol{
    JKCType type;
    VariableKind kind;

    VariableSymbol(JKCType type, VariableKind kind) : type(type), kind(kind) {}

};

struct FunctionSymbol{
    JKCType type;
    std::vector<JKCType> paramTypes;

    FunctionSymbol(JKCType type, std::vector<JKCType> paramTypes) : type(type), paramTypes(std::move(paramTypes)) {}
};

//simplify the map declaration, macro kinda
using Scope = std::unordered_map<std::string, VariableSymbol>;

//a function is separate because they are globally available, 
//whereas variables appear and disappear as we enter and exit blocks
class SymbolTable{
private:
    std::unordered_map<std::string, FunctionSymbol> functions;
    std::vector<Scope> scopes;

public:
    //defaul;t constructor
    SymbolTable() = default;

    //all symbol table specific modification/analysis based methods
    
    void enterScope();
    void exitScope();
    
    //var
    bool declareVariable(const std::string& name, JKCType type, VariableKind kind);
    const VariableSymbol* lookupVariable(const std::string& name) const; //pointer to specified variable
    bool variableExistsInCurrentScope(const std::string& name); //does it exist?
    
    //funct
    bool declareFunction(const std::string&name, JKCType returnType, const std::vector<JKCType>& paramTypes);
    const FunctionSymbol* lookupFunction(const std::string& name) const;
    bool functionExists(const std::string& name);

    //helper func
    bool hasActiveScope() const {return !scopes.empty();}


};
/*
ex
├── functions
│   ├── "main" → return int, parameters []
│   └── "add" → return int, parameters [int, int]
│
└── scopes
    ├── outer scope
    │   ├── x → int
    │   └── y → bool
    │
    └── inner scope
        └── z → int
*/