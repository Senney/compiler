#include <iostream>
#include "ASTNode.h"

ASTNode::ASTNode(std::string node_type) {
  this->parent = NULL;
  this->node_type = node_type;
  this->sym = NULL;
  
  this->stab = new SymbolTable();
}

// Adds a node to the children of this node.
ASTNode* ASTNode::addNode(ASTNode* node) {
  if (node == NULL) return NULL;

  nodes.push_back(node);
  node->setParent(this);
  return nodes.back();
}

void ASTNode::setSymbol(Symbol* s) {
  if (this->sym != NULL) return;
  this->sym = s;
}

Symbol* ASTNode::addSymbol(Token* tok, int type) {
  // We're at the head.
  if (this->getParent() == NULL) {
    return this->symbolTable()->addSymbol(new Symbol(tok, this, type));
  }
  
  if (this->getParent()->symbolTable() == NULL) {
    std::cerr << "Node " << this->node_type << " has parent, but parent has been robbed of tables." << std::endl;
    return NULL;
  }
  
  Symbol* s = this->getParent()->symbolTable()->addSymbol(new Symbol(tok, this, type));
  
  return s;
}

