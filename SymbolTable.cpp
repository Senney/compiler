#include <sstream>
#include "SymbolTable.h"

enum types {
  INT = 258,
  STRING = 259,
  ID = 260,
  BOOLEAN = 261,
  NUMBER = 262,
  VOID = 263
};

// Helper function which converts a token type to an enum.	
void Symbol::setType(int type) {
  // FIXME: Find a way to get the types in from jmm_parse.h without messing up
  // our entire include chain. For now we can hard-code the values with the assumption
  // that they won't change.
  if (type == INT) this->type = SYM_INT;
  else if (type == BOOLEAN) this->type = SYM_BOOLEAN;
  else if (type == STRING) this->type = SYM_STRING;
  else this->type = SYM_NONE;
}

std::string Symbol::toString() {
  std::stringstream ss;
  
  ss << "[SYMBOL] " << token->val << ":";
  switch (type) {
    case SYM_NONE:
      ss << "VOID";
      break;
    case SYM_INT:
      ss << "INT";
      break;
    case SYM_BOOLEAN:
      ss << "BOOL";
      break;
    case SYM_STRING:
      ss << "STRING";
      break;
    default:
      ss << "UNDEFINED";
      break;
  }
  
  ss << " L" << token->line;
  
  return ss.str();
}

SymbolTable::SymbolTable() {

}

SymbolTable::~SymbolTable() {

}

// Adds a symbol to the symbol table, and returns a reference to that symbol.
Symbol* SymbolTable::addSymbol(Symbol* sym) {
  if (hasSymbol(sym)) return NULL;   
  symbols.push_back(sym);
  return symbols.back();
}

// Removes a symbol from the symbol table.
void SymbolTable::removeSymbol(Symbol* sym) {
  if (!hasSymbol(sym) || symbols.empty()) return;
  symbols.erase(_getIterator(sym));
}
 
// Removes a symbol from the symbol table based on the name.
void SymbolTable::removeSymbol(std::string sym) {
  if (!hasSymbol(sym) || symbols.empty()) return;
  symbols.erase(_getIterator(sym));
}

// Determines if a symbol exists in the table (based on the name), and returns that symbol.
//  Returns NULL if no symbol is found.
Symbol* SymbolTable::hasSymbol(std::string sym) {
  stab_iter it = _getIterator(sym);
  
  if (it != symbols.end()) return *it;
  return NULL;
}

// Determines if a symbol exists in the table and returns that symbol.
//  Returns NULL if no symbol is found.
Symbol* SymbolTable::hasSymbol(Symbol* sym) {
  stab_iter it = _getIterator(sym);
  
  if (it != symbols.end()) return *it;
  return NULL;
}

// Adds another table to this one.
void SymbolTable::addSymbolTable(SymbolTable* table) {
  std::vector<Symbol*>* stb = table->getSymbolTableContainer();
  
  this->symbols.insert(this->symbols.end(), stb->begin(), stb->end());
}

// Prints the symbol table.
std::string SymbolTable::dumpTableString() {
  std::stringstream ss;
  
  for (unsigned int i = 0; i < symbols.size(); i++) {
    ss << symbols[i]->toString() << std::endl;
  }
  
  return ss.str();
}

// Returns the iterator to a symbol in the symbol table.
std::vector<Symbol*>::iterator SymbolTable::_getIterator(Symbol* sym) {
  if (symbols.empty()) return symbols.end();
  std::vector<Symbol*>::iterator it;
  
  for (it = symbols.begin(); it != symbols.end(); it++) {
    Symbol* _symbol = *it;
    if (_symbol->token == sym->token) {
      return it;
    }
  }
  
  return symbols.end();
}

// Returns the iterator to a symbol in the symbol table.
std::vector<Symbol*>::iterator SymbolTable::_getIterator(std::string sym) {
  if (symbols.empty()) return symbols.end();
  std::vector<Symbol*>::iterator it;
  
  for (it = symbols.begin(); it != symbols.end(); it++) {
    Symbol* _symbol = *it;
    if (_symbol->token->val == sym) {
      return it;
    }
  }
  
  return symbols.end();
}


