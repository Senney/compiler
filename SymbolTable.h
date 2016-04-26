#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <vector>

#include "Token.h"

// Forward declaration.
class ASTNode;

enum SymbolType { SYM_NONE, SYM_INT, SYM_BOOLEAN, SYM_STRING };

class Symbol {
public:
  Symbol(Token* tok, ASTNode* node, int type) {
    this->token = tok;
    this->node = node;
    setType(type);
    this->global = false;
    this->offset = 0;
    this->aux = ""; // Additional auxiliary information.
    
    this->arg_sym = false;
    this->arg_reg = 0;
  }

  Token* token;
  ASTNode* node;
  SymbolType type;
  
  // For code generation.
  int offset;
  std::string label;
  bool global;
  bool arg_sym;
  int arg_reg;
  std::string aux;
  
  std::string toString();
  
private:
  void setType(int type);
};

/* A symbol table for containing all registered symbols
  in a source file */
class SymbolTable {
public:
  SymbolTable();
  ~SymbolTable();
  
  Symbol* addSymbol(Symbol* sym);
  void removeSymbol(Symbol* sym);
  void removeSymbol(std::string sym);
  Symbol* hasSymbol(std::string sym);
  Symbol* hasSymbol(Symbol* sym);
  
  static Symbol* findSymbol(ASTNode* node, std::string symbolName);
  
  void addSymbolTable(SymbolTable* table);
  std::vector<Symbol*>* getSymbolTableContainer() { return &symbols; }
  
  std::string dumpTableString();
private:
  typedef std::vector<Symbol*>::iterator stab_iter;

  stab_iter _getIterator(Symbol* sym);
  stab_iter _getIterator(std::string sym);

  std::vector<Symbol*> symbols;
};

#endif
