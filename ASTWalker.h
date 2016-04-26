#ifndef AST_WALKER
#define AST_WALKER

#include <string>

#include "Error.h"
#include "ASTNode.h"
#include "ASTTypes.h"

class ASTWalker {
public:
  ASTWalker(ASTNode* node, std::string filname);
  
  void insertSymbol(std::string name, int type);
  void insertSymbol(Symbol* s);
  
  // Passes
  bool runSemanticCheck();
  bool buildSymbolTable();
  bool checkSymbols();
  bool typeCheck();
  bool miscCheck();
  
  std::string walkString();
  
  static Symbol* findSymbol(ASTNode* node, std::string stringName);
  
private:
  std::string walkChild(ASTNode* node);
  std::string formatNode(ASTNode* node);
  
  bool recurseSymbolTable(ASTNode* node);
  bool recurseSymbolExists(ASTNode* node);
  bool recurseTypeCheck(ASTNode* node);
  bool recurseMiscCheck(ASTNode* node);
  
  int expressionType(ExpressionNode* node);
  bool isConditional(ExpressionNode* node);
  std::string typeToString(int type);
  int convertType(int type);
  bool hasReturn(ASTNode* node);
  
  // FIXME: Get this non-manually.
  enum yytokentype {
   INT = 258,
   STRING = 259,
   ID = 260,
   BOOLEAN = 261,
   NUMBER = 262,
   VOID = 263,
   T_IF = 264,
   T_IFELSE = 265,
   T_BREAK = 266,
   T_RETURN = 267,
   T_WHILE = 268,
   EQ = 269,
   GE = 270,
   AND = 271,
   OR = 272,
   LE = 273,
   NE = 274,
   LESS = 275,
   GREAT = 276,
   NOT = 277,
   TRUE = 278,
   FALSE = 279,
   MUL = 280,
   DIV = 281,
   MOD = 282,
   ADD = 283,
   SUB = 284,
   EQUALS = 285,
   LPAREN = 286,
   RPAREN = 287,
   LBRACK = 288,
   RBRACK = 289,
   T_COMMA = 290,
   SEMICOLON = 291,
  };
  
  int indent;
  
  bool mainFound;
  
  ASTNode* head;
  const char* m_filename;
};

#endif
