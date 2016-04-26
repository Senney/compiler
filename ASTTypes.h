#ifndef AST_TYPES_H
#define AST_TYPES_H

#include <vector>
#include <string>
#include "SymbolTable.h"
#include "Token.h"
#include "ASTNode.h"

// Definitions of strings that are used for Statement Identification
#define BLOCK_NODE              "BlockNode"
#define STATEMENT_NODE          "Statement"
#define EXPRESSION_NODE         "Expression"
#define DECLARATION_NODE        "Declaration"
#define IDENTIFIER_NODE         "Identifier"
#define VAR_DECL_NODE           "VariableDeclaration"
#define FUNCTION_PARAM_NODE     "FunctionParam"
#define FUNCTION_DECL_NODE      "FunctionDeclaration"
#define FUNCTION_HEAD_NODE      "FunctionHeader"
#define FUNCTION_INVOKE_NODE    "FunctionInvokation"
#define MAIN_FUNC_NODE          "MainFunctionNode"
#define MAIN_FUNC_DECL_NODE     "MainFunctionDeclaration"
#define OPERATION_NODE          "Operation"
#define LITERAL_NODE            "Literal"
#define UNARY_OP_NODE           "UnaryOperation"
#define ASSIGNMENT_NODE         "Assignment"
#define RESERVED_NODE           "ReservedNode"


/* A node to contain a block of statements as children. */
class BlockNode : public ASTNode {
public:
  BlockNode() : ASTNode(BLOCK_NODE) { }
private:
};

/* A general node for statements. */
class StatementNode : public ASTNode {
public:
  StatementNode() : ASTNode(STATEMENT_NODE) { }
  StatementNode(std::string node_type) : ASTNode(node_type) { }
};	

/* A general node for all expressions. */
class ExpressionNode : public ASTNode {
public:
  ExpressionNode() : ASTNode(EXPRESSION_NODE) { }
  ExpressionNode(std::string node_type) : ASTNode(node_type) { }
};

/* A general node for all declaration types (function, variable) */
class DeclarationNode : public StatementNode {
public:
  DeclarationNode() : StatementNode(DECLARATION_NODE) { }
  DeclarationNode(std::string node_type) : StatementNode(node_type) { }
};

/* Stores information relevant to an identifier. */
class IdentifierNode : public ExpressionNode {
public:  
  IdentifierNode() : ExpressionNode(IDENTIFIER_NODE) { }
  IdentifierNode(Token* tok) : ExpressionNode(IDENTIFIER_NODE) {
    this->token = tok;
  }
  
  Token* getToken() { return token; }

private:
  Token* token;
};

/* Stores information relevant to a variable declaration and inserts new 
variables in to the parents' symbol table */
class VariableDeclarationNode : public DeclarationNode {
public:
  VariableDeclarationNode(IdentifierNode* id, int type) : DeclarationNode(VAR_DECL_NODE) {
   this->id = id;
   this->type = type;
  }

  IdentifierNode* getID() { return this->id; }
  int getVariableType() { return this->type; }
  
private:
  IdentifierNode* id;
  int type;
};

class FunctionParamNode : public ExpressionNode {
public:
  FunctionParamNode() : ExpressionNode(FUNCTION_PARAM_NODE) {
  }
  
  void addParam(ASTNode* n) { paramNodes.push_back(n); }
  std::vector<ASTNode*>& getNodes() { return paramNodes; }
  
private:  
  std::vector<ASTNode*> paramNodes;
};

class FunctionDeclarationNode : public StatementNode {
public:
  FunctionDeclarationNode() : StatementNode(FUNCTION_DECL_NODE) { }
  
  FunctionDeclarationNode(IdentifierNode* id) : StatementNode(FUNCTION_DECL_NODE) {
    this->id = id;
    this->params = new FunctionParamNode;
  }
  FunctionDeclarationNode(IdentifierNode* id, FunctionParamNode* params) 
      : StatementNode(FUNCTION_DECL_NODE) { 
    this->id = id;
    this->params = params;
  }
  
  IdentifierNode* getID() { return this->id; };
  FunctionParamNode* getParams() { return this->params; }
private:
  IdentifierNode* id;
  FunctionParamNode* params;
};

class FunctionHeaderNode : public DeclarationNode {
public:
  FunctionHeaderNode() : DeclarationNode(FUNCTION_HEAD_NODE) { }
  FunctionHeaderNode(int &type, FunctionDeclarationNode* decl) : DeclarationNode(FUNCTION_HEAD_NODE) {
    this->type = type;
    this->decl = decl;
  }
 
  FunctionDeclarationNode* getDeclaration() { return decl; }
  void setCode(BlockNode* code) { this->block = code; }
  BlockNode* getCode() { return block; }
  int getFunctionType() { return type; }
   
private:
  int type;
  FunctionDeclarationNode* decl;
  BlockNode* block;
};

class FunctionInvokationNode : public ExpressionNode {
public:
  FunctionInvokationNode(IdentifierNode* id) : ExpressionNode(FUNCTION_INVOKE_NODE) {
    this->id = id;
    params = new FunctionParamNode();
  }
  
  FunctionInvokationNode(IdentifierNode* id, FunctionParamNode* params) : ExpressionNode(FUNCTION_INVOKE_NODE) {
    this->id = id;
    this->params = params;
  }
  
  IdentifierNode* getID() { return id; }
  FunctionParamNode* getParams() { return params; }
  
private:
  IdentifierNode* id;
  FunctionParamNode* params;
  
};

class MainFunctionDeclarationNode;
class MainFunctionNode : public DeclarationNode {
public:
  MainFunctionNode(MainFunctionDeclarationNode* node, BlockNode* code) : DeclarationNode(MAIN_FUNC_NODE) {
    this->mfd = node;
    this->code = code;
  }
  
  MainFunctionDeclarationNode* getDeclaration() { return this->mfd; }
  BlockNode* getCode() { return this->code; }
  
private:
  MainFunctionDeclarationNode* mfd;
  BlockNode* code;
  
};

class MainFunctionDeclarationNode : public StatementNode {
public:
  MainFunctionDeclarationNode(IdentifierNode* id) : StatementNode(MAIN_FUNC_DECL_NODE) {
    this->id = id;
  }
  
  IdentifierNode* getID() { return this->id; }
private:
  IdentifierNode* id;
};

class OperationNode : public ExpressionNode {
public:
  OperationNode(ExpressionNode* lhs, int op, ExpressionNode* rhs) : ExpressionNode(OPERATION_NODE) {
    this->lhs = lhs;
    this->rhs = rhs;
    this->operation = op;
  }
  
  int getOperation() { return operation; }
  
  ExpressionNode* getRHS() { return this->rhs; }
  ExpressionNode* getLHS() { return this->lhs; }
  
private:
  ExpressionNode *rhs, *lhs;
  int operation;
};

class LiteralNode : public ExpressionNode {
public:
  LiteralNode(Token* tok, int type) : ExpressionNode(LITERAL_NODE) {
    this->token = tok;
    this->type = type;
  }
  
  int getLiteralType() { return this->type; }
  Token* getToken() { return this->token; }
  
private:
  int type;
  Token* token;
};

class UnaryOperationNode : public ExpressionNode {
public:
  UnaryOperationNode(int type, ExpressionNode* rhs) :
    ExpressionNode(UNARY_OP_NODE) {
    this->type = type;
    this->rhs = rhs;
  }
  
  int getOperation() { return this->type; }
  
  ExpressionNode* getRHS() { return this->rhs; }
  
private:
  int type;
  ExpressionNode* rhs;
};

class AssignmentNode : public ExpressionNode {
public:
  AssignmentNode(IdentifierNode* lhs, ExpressionNode* rhs) :
      ExpressionNode(ASSIGNMENT_NODE) {
    this->lhs = lhs;
    this->rhs = rhs;
  }
  
  ExpressionNode* getRHS() { return rhs; }
  IdentifierNode* getLHS() { return lhs; }
  
private:
  IdentifierNode* lhs;
  ExpressionNode* rhs;
};

class ReservedNode : public ExpressionNode {
public:
  ReservedNode(int type) : ExpressionNode(RESERVED_NODE) {
    this->type = type;
    this->aux = "";
  }
  
  int getReservedType() { return this->type; }
  std::string aux;
  
private:
  int type;
};

#endif
