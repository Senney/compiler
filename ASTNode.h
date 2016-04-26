#ifndef AST_NODE_H
#define AST_NODE_H

#include <vector>
#include <string>
#include <iostream>

#include "SymbolTable.h"

// Forward declaration of SymbolTable and Symbol.
// class Symbol;
// class SymbolTable;

class ASTNode {
public:
  ASTNode(std::string node_type);
  
  ASTNode* addNode(ASTNode* node);  
  std::vector<ASTNode*>& getChildren() { return nodes; }
  std::string getNodeType() { return node_type; }
  
  void setParent(ASTNode* node) { this->parent = node; }
  ASTNode* getParent() { return parent; }
  
  SymbolTable* symbolTable() { return this->stab; }
  Symbol* addSymbol(Token* tok, int type);
  
  Symbol* getSymbol() { return sym; }
  void setSymbol(Symbol* s);
  
  std::string getType() { return node_type; }
  
private:
  Symbol* sym;
  SymbolTable* stab;
  ASTNode* parent;

  std::vector<ASTNode*> nodes;
  std::string node_type;
};

#endif
