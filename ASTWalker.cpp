#include <sstream>
#include <vector>
#include <iostream>
#include "Error.h"
#include "SymbolTable.h"
#include "ASTWalker.h"

ASTWalker::ASTWalker(ASTNode* node, std::string filename) {
  this->head = node;
  this->indent = 0;
  this->mainFound = false;
  this->m_filename = filename.c_str();
  
  // Setup the RTS symbol table.
  
}

std::string ASTWalker::walkString() {
  std::string ret = "";
  
  ASTNode* cur = head;
  
  ret = walkChild(cur);
  
  return ret;
}

void ASTWalker::insertSymbol(std::string name, int type) {     
  head->addSymbol(new Token(Token::IDENTIFIER_STRING, -1, name), type);
}


bool ASTWalker::runSemanticCheck() {
  bool sym_tbl = buildSymbolTable();
  bool sym_def = checkSymbols();
  bool typ_chk = /*typeCheck();*/ true; /* FIXME */
  bool mis_chk = miscCheck();
  
  if (!mainFound) {    
    error(m_filename, 0, "ERROR: No main function defined.\n");
    return false; 
  }
  
  return (sym_tbl && sym_def && typ_chk && mis_chk);
}

bool ASTWalker::buildSymbolTable() {
  recurseSymbolTable(head);
  return true;
}

bool ASTWalker::checkSymbols() {
  return recurseSymbolExists(head);
}

bool ASTWalker::typeCheck() {
  return recurseTypeCheck(head);
}

bool ASTWalker::miscCheck() {
  return recurseMiscCheck(head);
}

bool ASTWalker::recurseSymbolTable(ASTNode* node) {
  if (node == NULL) { 
    return false;
  }

  std::string ntype = node->getType();
  if (ntype == VAR_DECL_NODE) {
    // Cast and add a symbol to the table.    
    VariableDeclarationNode* sym = (VariableDeclarationNode*)node;
    
    // Symbol Redefinition.
    if (findSymbol(node, sym->getID()->getToken()->val) != NULL) {
      error(m_filename, sym->getID()->getToken()->line, "Symbol Redefinition: %s has already been defined.\n", sym->getID()->getToken()->val.c_str());
      return false;
    }
    
    Symbol* s = sym->addSymbol(sym->getID()->getToken(), sym->getVariableType());
    sym->setSymbol(s);
     
  } else if (ntype == MAIN_FUNC_DECL_NODE) {
    MainFunctionDeclarationNode* mfd = (MainFunctionDeclarationNode*)node;
    
    // Check for multiple definitions of this function.
    if (findSymbol(node, mfd->getID()->getToken()->val) != NULL) {
      error(m_filename, mfd->getID()->getToken()->line, "Multiple Main Function Definitions: %s is redefining the main function.\n",
          mfd->getID()->getToken()->toString().c_str());
      return false;
    }
    
    mfd->addSymbol(mfd->getID()->getToken(), 0); 
 
  } else {
    if (ntype == FUNCTION_HEAD_NODE) {
      // Cast and add a symbol to the table.
      FunctionHeaderNode* sym = (FunctionHeaderNode*)node;  
        
      // Check for multiple definitions of this function.
      if (findSymbol(node, sym->getDeclaration()->getID()->getToken()->val) != NULL) {
        error(m_filename, sym->getDeclaration()->getID()->getToken()->line, 
              "Multiple Function Definitions: %s is already defined.\n", sym->getDeclaration()->getID()->getToken()->toString().c_str());
        return false;
      }
       
      sym->addSymbol(sym->getDeclaration()->getID()->getToken(), sym->getFunctionType());
    } 
    
    std::vector<ASTNode*>& nodes = node->getChildren();
    bool good = true;
    for (unsigned int i = 0; i < nodes.size(); i++) {
      bool b = recurseSymbolTable(nodes[i]);

      if (good) good = b;
    }
    
    // If we recurse up to a FunctionParameterNode, we want to copy
    // the symbols from here to the FunctionHeader so that they share
    // the same scope as the rest of the function.
    if (ntype == FUNCTION_PARAM_NODE) {
      FunctionHeaderNode* header;
      if (node->getParent() != NULL && 
           (header = (FunctionHeaderNode*)node->getParent()->getParent()) != NULL &&
           (header->getType() == FUNCTION_HEAD_NODE)) {
         header->symbolTable()->addSymbolTable(node->symbolTable());
      }
      
      // mark those nodes as function nodes.
      std::vector<Symbol*>* syms = header->symbolTable()->getSymbolTableContainer();
      for (unsigned int i = 0; i < syms->size(); i++) {
        syms->at(i)->arg_sym = true;
        syms->at(i)->arg_reg = i;
      }
    }
  }
  
  return true;
}

bool ASTWalker::recurseSymbolExists(ASTNode* node) {
  if (node == NULL) return true;
  
  if (node->getType() == IDENTIFIER_NODE) {
    IdentifierNode* inode = (IdentifierNode*)node;
    
    // Find the symbol in our symbol table by recursing up the tree (and up in levels of scope).    
    std::string name = inode->getToken()->val;
 
    // Skip nodes which already have symbols, as they are definition nodes.
    if (node->getSymbol() != NULL) { 
      return true;
    }
    
    // Attempt to find the symbol in the symtable.
    Symbol* s = findSymbol(node, name);
    if (s == NULL) {
      error(m_filename, inode->getToken()->line, "Symbol Undefined: %s\n", name.c_str());
      return false;
    } else {
      node->setSymbol(s);
    }
  } else {
    std::vector<ASTNode*>& nodes = node->getChildren();
    
    // Set up this alg such that it will continue running even
    // after an undefined symbol has been found.
    bool good = true;
    for (unsigned int i = 0; i < nodes.size(); i++) {
      bool r = recurseSymbolExists(nodes[i]);
      if (good) good = r;
    }
    
    return good;
  }
  
  return true;
}

bool ASTWalker::recurseTypeCheck(ASTNode* node) {
  if (node == NULL) return true;
  
  std::string ntype = node->getType();
  
  bool allgood = true;

  if (ntype == FUNCTION_INVOKE_NODE) {    
    FunctionInvokationNode* fnode = (FunctionInvokationNode*)node;
    IdentifierNode* f_id = (IdentifierNode*)fnode->getID();
    FunctionParamNode* params = fnode->getParams();
    
    std::string functionName = f_id->getToken()->val;
    
    if (f_id == NULL || f_id->getSymbol() == NULL) {    
        std::cerr << "ID Node: " << f_id << " ChildNode: " << fnode->getID() << std::endl;
        std::cerr << "Compiler Error: FunctionInvokation left without a symbol. Function: " << 
        f_id->getToken()->val << " -- Node: " << f_id << std::endl;
      allgood = false;
      goto nextNode;
    }
    
    // Get the FunctionHeader for this invokation.
    ASTNode* defNode = f_id->getSymbol()->node;
    
    // Check we're calling the main function.
    if (defNode->getType() == MAIN_FUNC_DECL_NODE) {
      error(m_filename, f_id->getToken()->line, "Function Invokation Error: %s cannot be called, as it is the entry point.\n",
          f_id->getToken()->val.c_str());
      allgood = false;
      goto nextNode;
    }
    
    // Check function call types && number
    if (defNode->getType() == FUNCTION_HEAD_NODE) {
      // Get the definition items for this function.
      FunctionHeaderNode* fhead = (FunctionHeaderNode*)defNode;
      FunctionDeclarationNode* decl = (FunctionDeclarationNode*)fhead->getDeclaration();
      
      int expectedArgs = decl->getParams()->getNodes().size();
      int actualArgs = params->getNodes().size();      
      if (expectedArgs != actualArgs) {
        error(m_filename, decl->getID()->getToken()->line, "Invalid Number of Arguments: Expected %d and got %d.\n",
              actualArgs, expectedArgs);
        allgood = false;
        goto nextNode;
      }
      
      for (unsigned int i = 0; i < params->getNodes().size(); i++) {
        VariableDeclarationNode* id_def = (VariableDeclarationNode*)decl->getParams()->getNodes()[i];
        ExpressionNode* id_act = (ExpressionNode*)params->getNodes()[i];
      
        int exp = convertType(id_def->getVariableType());
        int act = convertType(expressionType(id_act));
        if (exp != act) {
          error(m_filename, f_id->getToken()->line, "Mismatched Parameter Tpye: Function %s expects type %s, but got %s.\n",
                functionName.c_str(), typeToString(exp).c_str(), typeToString(act).c_str());
          allgood = false;
          goto nextNode;
        }
      }
      
    } else {
      std::cout << "Compiler Error: Symbol for function definition failed. " << f_id->getToken()->toString() << std::endl;
      // FIXME: When the RTS is implemented, fix this.
      //allgood = false;
      goto nextNode;
    }
    
    
  } else if (ntype == OPERATION_NODE) {
    OperationNode* op = (OperationNode*)node;
    
    if (op->getChildren().size() != 2) {
      std::cerr << "Compiler Error: Operation does not have two parameters." << std::endl;
      allgood = false;
      goto nextNode;
    }
    
    // Check the types of the arguments.
    int lhs = expressionType((ExpressionNode*)op->getChildren()[0]);
    int rhs = expressionType((ExpressionNode*)op->getChildren()[1]);
    if (lhs != rhs) {
      std::cerr << "Type Mismatch Error: Left-hand type " << typeToString(lhs) <<
        " incompatible with right-hand type " << rhs << std::endl;
      
      allgood = false;
      goto nextNode;
    }
    
  } else if (ntype == UNARY_OP_NODE) {
    UnaryOperationNode* op = (UnaryOperationNode*)node;
    
    if (op->getChildren().size() != 1) {
      std::cerr << "Compiler Error: Unary Operation does not have a single parameter." << std::endl;
      allgood = false;
      goto nextNode;
    }
  } else if (ntype == ASSIGNMENT_NODE) {
    AssignmentNode* anode = (AssignmentNode*)node;
    
    if (anode->getChildren().size() != 2) {
      std::cerr << "Compiler Error: Assignment Operation does not have two parameters." << std::endl;
      allgood = false;
      goto nextNode;
    }
    
    int rhs = convertType(expressionType((ExpressionNode*)anode->getChildren()[0]));
    int lhs = convertType(expressionType((ExpressionNode*)anode->getChildren()[1]));
    if (lhs != rhs) {
      std::cerr << "Assignment Mismatch: Cannot assign type " << typeToString(rhs) << " to equal type " << 
        typeToString(lhs) << "." << std::endl;
       
      allgood = false;
      goto nextNode;
    } 
  } else if (ntype == RESERVED_NODE) {
    ReservedNode* res = (ReservedNode*)node;
    int t = res->getReservedType();
    
    if (t == T_IF || t == T_WHILE) {
      ExpressionNode* expr = (ExpressionNode*)res->getChildren()[0];
      
      if (isConditional(expr)) {
        std::cerr << "Type Mismatch Error: IF and WHILE expect a boolean argument to operate." << std::endl;
        allgood = false;
        goto nextNode;
      }
    } else if (t == T_RETURN) {
      // Ensure that RETURN statements are returning the correct value.
    
      ASTNode* cur = node;
      
      // Default our return type to void.
      int ret_type = SYM_NONE;
      // Get the return type if the return statement has an expression.
      if (res->getChildren().size() > 0) {
        ExpressionNode* expr = (ExpressionNode*)res->getChildren()[0];
        
        if (isConditional(expr)) ret_type = SYM_BOOLEAN;
        else ret_type = convertType(expressionType(expr));
      }
          
      // Find the function header by walking up the tree.
      while (cur != NULL && cur->getType() != FUNCTION_HEAD_NODE)
        cur = cur->getParent();
        
      if (cur == NULL) {
        std::cerr << "Return Statement Mismatch: This return statement does not match any function." << std::endl;
        allgood = false;
        goto nextNode;
      }
        
      FunctionHeaderNode* head = (FunctionHeaderNode*)cur;
      int func_type = convertType(head->getFunctionType());
      
      if (func_type != ret_type) {
        std::cerr << "Return Value Mismatch: Function " << head->getDeclaration()->getID()->getToken()->val << 
          " return value is not matched by the return statement. Expected " << typeToString(func_type) <<
          " got " << typeToString(ret_type) << std::endl;
          
        allgood = false;
        goto nextNode;
      }
    
    } else if (t == T_BREAK) {
      // Ensure that break statements are contained in while loops.      
      ASTNode* cur = node;
      while (cur != NULL) {
        if (((ReservedNode*)cur)->getReservedType() == T_WHILE) {
          break;
        }
        
        // Loop until our next reserved word.
        do {
          cur = cur->getParent();
        } while (cur != NULL && cur->getType() != RESERVED_NODE);
      }
      
      // If cur is null, we've reached the top of the tree and have nothing left to check.
      if (cur == NULL) {
        std::cerr << "Flow Control Error: Break statements may only be placed within while loops." << std::endl;
        allgood = false;
        goto nextNode;        
      }
    }
  }
  
nextNode:
  std::vector<ASTNode*>& nodes = node->getChildren();

  // Set up this alg such that it will continue running even
  // after an undefined symbol has been found.
  bool good = true;
  for (unsigned int i = 0; i < nodes.size(); i++) {
    bool r = recurseTypeCheck(nodes[i]);
    if (good) good = r;
  }
  
  return ((allgood && good) == true);
}

bool ASTWalker::recurseMiscCheck(ASTNode* node) {
  if (node == NULL) return false;

  if (node->getType() == MAIN_FUNC_DECL_NODE) {
    if (mainFound) {
      std::cerr << "Main Already Declared: A main function has already been declared." << std::endl;
      return false;
    }
    mainFound = true;
  }
  
  // Check to ensure that each function has a return value.
  if (node->getType() == FUNCTION_HEAD_NODE) {
    FunctionHeaderNode* head = (FunctionHeaderNode*)node;
    std::string functionName = head->getDeclaration()->getID()->getToken()->val;
    
    if (head->getFunctionType() == INT || head->getFunctionType() == BOOLEAN) {
      if (!hasReturn(node)) {
        std::cerr << "Function " << functionName << " has no return value." << std::endl;
        return false;
      }
    } else {
      if (hasReturn(node)) {
        std::cerr << "Function " << functionName << " is of type VOID and cannot have a return value." << std::endl;
        return false;
      }
    }
  }
  
  std::vector<ASTNode*>& children = node->getChildren();
  for (unsigned int i = 0; i < children.size(); i++) {
    recurseMiscCheck(children[i]);
  }
  
  return true;
}

// A global method to find a defined symbol.
Symbol* ASTWalker::findSymbol(ASTNode* node, std::string symbolName) {
  ASTNode* cur = node;
  while (cur != NULL) {
    SymbolTable* st = cur->symbolTable();
    if (st == NULL) return NULL;
    
    Symbol* s;
    if ((s = st->hasSymbol(symbolName)) != NULL) return s;
    
    cur = cur->getParent();
  }
  
  return NULL;
}

int ASTWalker::expressionType(ExpressionNode* node) {
  if (node == NULL) return -1;
  
  // Check if we've reached a terminator.
  if (node->getType() == LITERAL_NODE) {
    LiteralNode* lit = (LiteralNode*)node;
    return lit->getLiteralType();    
    
  } else if (node->getType() == IDENTIFIER_NODE) {
    IdentifierNode* id = (IdentifierNode*)node;
    
    if (id->getSymbol() == NULL) {
      std::cerr << "Identifier didn't have a symbol attached. " << id->getToken()->val << std::endl;
      return -1;
    }
    
    return id->getSymbol()->type;
    
  } else if (node->getType() == FUNCTION_INVOKE_NODE) {
    FunctionInvokationNode* func = (FunctionInvokationNode*)node;
    IdentifierNode* f_id = (IdentifierNode*)func->getID();
    
    if (f_id->getSymbol() == NULL) {
      std::cerr << "Function " << f_id->getToken()->val << " is undefined. Check compiler sanity." << std::endl;
      return -1;
    }
    
    return f_id->getSymbol()->type;
    
  } else if (node->getType() == OPERATION_NODE) {
    OperationNode* op = (OperationNode*)node;
    if (op->getChildren().size() != 2) {
      std::cerr << "Error: Unacceptable number of arguments on a binary operation." << std::endl;
      return -1;
    }
    
    // Recurse on the left and right hand sides.
    int lhs = convertType(expressionType((ExpressionNode*)op->getChildren()[0]));
    int rhs = convertType(expressionType((ExpressionNode*)op->getChildren()[1]));
    if (lhs == rhs) { return lhs; }
    else {
      std::cerr << "Type Mismatch During Evaluation: Type " << typeToString(lhs) << " is incompatible with type " <<
        typeToString(rhs) << "." << std::endl; 
        return -1;
    } 
  } else if (node->getType() == UNARY_OP_NODE) {
    UnaryOperationNode* op = (UnaryOperationNode*)node;
    if (op->getChildren().size() != 1) {
      std::cerr << "Compiler Error: Unacceptable number of arguments on a Unary Operation." << std::endl;
      return -1;
    }
    
    // Recurse on the left and right hand sides.
    int lhs = convertType(expressionType((ExpressionNode*)op->getChildren()[0]));
    return lhs;
  }
  
  std::cerr << "Compiler Error: Unexpected item " << node->getType() << " in Expression." << std::endl;
  return -1;
}

std::string ASTWalker::typeToString(int type) {
  std::string ret = "INVALID";
  switch (type) {
    case VOID:
    case SYM_NONE:
      ret = "VOID";
      break;
    case INT:
    case SYM_INT:
      ret = "INT";
      break;
    case BOOLEAN:
    case SYM_BOOLEAN:
      ret = "BOOL";
      break;
    case STRING:
    case SYM_STRING:
      ret = "STRING";
      break;
  }
  
  return ret;
}

int ASTWalker::convertType(int type) {
  switch (type) {
    case VOID:
      return SYM_NONE;
    case INT:
      return SYM_INT;
    case BOOLEAN:
      return SYM_BOOLEAN;
    case STRING:
      return SYM_STRING;
  }
  
  return type;
}

bool ASTWalker::isConditional(ExpressionNode* node) {
  if (node == NULL) return false;
  
  if (node->getType() == OPERATION_NODE) {
    OperationNode* op = (OperationNode*)node;
    
    int type = op->getOperation();
    
    switch (type) {
      case OR:
      case AND:
      case EQ:
      case NE:
      case LE:
      case GE:
      case GREAT:
      case LESS:
        return true;
        break;
      default:
        break;
    }
    
    return false;
  } else if (node->getType() == UNARY_OP_NODE) {
    UnaryOperationNode* op = (UnaryOperationNode*)node;
    
    int type = op->getOperation();
    
    switch (type) {
      case NOT:
        return true;
    }
    
    return false;
  }
  
  return false;
}

bool ASTWalker::hasReturn(ASTNode* node) {
  if (node == NULL) return false;
  
  if (node->getType() == RESERVED_NODE) {
    ReservedNode* res = (ReservedNode*)node;
    
    if (res->getReservedType() == T_RETURN) {
      return true;
    }
  }
  
  bool ret = false;
  for (unsigned int i = 0; i < node->getChildren().size(); i++) {
    ret = hasReturn(node->getChildren()[i]);
    if (ret) return ret;
  }
  
  return false;
}

std::string ASTWalker::walkChild(ASTNode* node) {
  if (node == NULL) return "";
  
  std::stringstream ss;
  std::vector<ASTNode*> nodes = node->getChildren();
  
  ss << "{";
  ss << formatNode(node) << '\n';
  this->indent++;
  for (unsigned int i = 0; i < nodes.size(); i++) {
    for (int j = 0; j < indent; j++) ss << '\t';
    ss << walkChild(nodes[i]);
  }
  this->indent--;
  for (int j = 0; j < indent; j++) ss << '\t';
  ss << "}" << '\n';
  
  return ss.str();
}

std::string ASTWalker::formatNode(ASTNode* node) {
  if (node == NULL) return "";
  
  std::stringstream ss;
  std::string type = node->getNodeType();
  ss << "Type(" << type << ") ";

  std::vector<Symbol*> tbl = *node->symbolTable()->getSymbolTableContainer();
  
  if (!tbl.empty()) {
    ss << "Symbols(";
    for (unsigned int i = 0; i < tbl.size(); i++) {
      ss << tbl[i]->token->val;
      if (i < tbl.size() - 1) ss << ", ";
    }
    ss << ")";
  }
  
  return ss.str();
}

