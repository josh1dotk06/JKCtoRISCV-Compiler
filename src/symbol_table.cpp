#include "ast.hpp"
#include "symbol_table.hpp"
#include <string>
#include <vector>
#include <unordered_map>

//create new empty default scopes object
void SymbolTable::enterScope(){
    scopes.emplace_back();
}

void SymbolTable::exitScope(){
    if(scopes.empty()) throw std::runtime_error("cannot exit scope: no active scope");
    scopes.pop_back();
}

//var
bool SymbolTable::declareVariable(const std::string& name, JKCType type, VariableKind kind){
    
    if(scopes.empty()){
        throw std::runtime_error("cannot declare variable: no active scopes");
    }

    //unordered map that directly references to the back of scopes (same object), aka becomes an alias
    //thus emplacing into currentScope modifies scopes
    Scope& currentScope = scopes.back();
    bool state = currentScope.emplace(name, VariableSymbol(type, kind)).second;
    //second cuz emplace returns {it, insertsuccess}, basically return a success or not
    return state;
}

//NOTE The outermost scope is the first item in the vector? 
//And the innermost is the last item in the vector
const VariableSymbol* SymbolTable::lookupVariable(const std::string& name) const{
    //start searching from inner to outermost scope, thus use reverse
    //also rbegin returns an iterator, which we dereference (similar to lowerbound)
    for(auto scope = scopes.rbegin(); scope != scopes.rend(); ++scope){
        auto match = (*scope).find(name);
        if(match != (*scope).end()) return &(*match).second; //gives the VariableSymbol
    }
}
bool SymbolTable::variableExistsInCurrentScope(const std::string& name){
    if(scopes.empty()) return false;

    const Scope& currScope = scopes.back();
    return currScope.find(name) != currScope.end();
}

//funct
bool SymbolTable::declareFunction(const std::string&name, JKCType returnType, const std::vector<JKCType>& paramTypes){
    return functions.emplace(name, FunctionSymbol(returnType, paramTypes)).second;

}

const FunctionSymbol* SymbolTable::lookupFunction(const std::string& name) const{
    auto match = functions.find(name);
    if(match != functions.end()) return &(*match).second; //return functionsymbol
    return nullptr;
}

bool SymbolTable::functionExists(const std::string& name){
    auto match = functions.find(name);
    if(match != functions.end()) return true;
    else return false;
}


