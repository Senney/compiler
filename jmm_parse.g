%{
#include <iostream>
#include "Error.h"
#include "Token.h"
#include "ASTNode.h"
#include "ASTTypes.h"

// Some concepts referenced from
// http://gnuu.org/2009/09/18/writing-your-own-toy-compiler/4/

// Define externs
void yyerror(const char* err) { std::cerr << err << std::endl; }
extern int yylex();

BlockNode* root;

%}

// Value types.
%union {
  Token* tok;
  ASTNode* node;
  BlockNode* block;
  VariableDeclarationNode* var;
  FunctionDeclarationNode* func;
  MainFunctionDeclarationNode* main;
  FunctionInvokationNode* finv;
  
  IdentifierNode* ident;
  StatementNode* statement;
  DeclarationNode* decl;
  ExpressionNode* expr;
  AssignmentNode* assign;
  FunctionParamNode* params;
  
  int type;
}

/* Token definitions */
%token INT STRING ID BOOLEAN NUMBER
%token VOID

// If block
%token T_IF T_IFELSE

// Flow Control
%token T_BREAK T_RETURN

// Looping
%token T_WHILE 

// Conditional Operations
%token EQ GE AND OR LE NE LESS GREAT NOT

// Conditional Values
%token TRUE FALSE

// Arithmetic
%token MUL DIV MOD
%token ADD SUB
%token EQUALS

// Formatting
%token LPAREN RPAREN LBRACK RBRACK T_COMMA SEMICOLON

// Define types.
%type <block> program globaldeclarations block blockstatements
%type <statement> blockstatement
%type <func> functiondeclarator
%type <statement> mainfunctiondeclarator 
%type <decl> globaldeclaration functiondeclaration mainfunctiondeclaration variabledeclaration
%type <decl> functionheader
%type <ident> identifier
%type <type> type VOID
%type <node> statement
%type <expr> expression statementexpression assignmentexpression postfixexpression primary literal
%type <expr> conditionalorexpression conditionalandexpression multiplicativeexpression 
%type <expr> additiveexpression relationalexpression equalityexpression unaryexpression
%type <assign> assignment
%type <finv> functioninvocation
%type <params> argumentlist formalparameterlist
%type <var> formalparameter
%type <tok> ID NUMBER STRING TRUE FALSE

%start program

%%

program		: { root = NULL; }
		| globaldeclarations { root = $1; }
		;

literal		: NUMBER
		{ $$ = new LiteralNode($1, INT); }
		| STRING
		{ $$ = new LiteralNode($1, STRING); }
		| TRUE
		{ $$ = new LiteralNode($1, BOOLEAN); }
		| FALSE
		{ $$ = new LiteralNode($1, BOOLEAN); }
		;

type: BOOLEAN { $$ = BOOLEAN; }
		| INT { $$ = INT; }
		;

globaldeclarations	: globaldeclaration { $$ = new BlockNode(); $$->addNode($1); }
			| globaldeclarations globaldeclaration { $1->addNode($2); }
			;

globaldeclaration	: variabledeclaration
			  { $$ = $1; }
			| functiondeclaration
			  { $$ = $1; }
			| mainfunctiondeclaration
			  { $$ = $1; }
			;

variabledeclaration	: type identifier SEMICOLON 
			  { $$ = new VariableDeclarationNode($2, $1); $$->addNode($2); }
			;

identifier		: ID
			  { $$ = new IdentifierNode($1); }
			;

functiondeclaration	: functionheader block
			{ $$ = $1;
			
                          // HACK: Sets the code block for easier code parsing.
                          ((FunctionHeaderNode*)$$)->setCode($2);
			  $$->addNode($2);
			}
			;

functionheader		: type functiondeclarator
			{ $$ = new FunctionHeaderNode($1, $2); $$->addNode($2);
			}
			| VOID functiondeclarator
			{ $$ = new FunctionHeaderNode($1, $2); $$->addNode($2);
			}
			;

functiondeclarator	: identifier LPAREN formalparameterlist RPAREN
			  { $$ = new FunctionDeclarationNode($1, $3); $$->addNode($1); $$->addNode($3); }
			| identifier LPAREN RPAREN
			  { $$ = new FunctionDeclarationNode($1); $$->addNode($1); }
			;

formalparameterlist	: formalparameter { $$ = new FunctionParamNode(); $$->addNode($1); $$->addParam($1); }
			| formalparameterlist T_COMMA formalparameter
			{ $$->addNode($3); $$->addParam($3); }
			;

formalparameter		: type identifier
			{ $$ = new VariableDeclarationNode($2, $1); $$->addNode($2); }
			;

mainfunctiondeclaration	: mainfunctiondeclarator block
			{ 
			  $$ = new MainFunctionNode((MainFunctionDeclarationNode*)$1, $2);
			  $$->addNode($1);
			  $$->addNode($2);
			}
			;

mainfunctiondeclarator	: identifier LPAREN RPAREN
			  { $$ = new MainFunctionDeclarationNode($1); $$->addNode($1); }
			;

block			: LBRACK blockstatements RBRACK
			  { $$ = $2; }
			| LBRACK RBRACK
			  { $$ = new BlockNode(); }
			;

blockstatements		: blockstatement
			  { $$ = new BlockNode(); $$->addNode($1); }
			| blockstatements blockstatement
			  { $$->addNode($2); }
			;

blockstatement		: variabledeclaration
			{ $$ = $1;
			}
			| statement
			{ $$ = $<statement>1;
			}
			;

statement		: block { $$ = $1; }
			| SEMICOLON { $$ = NULL; }
			| statementexpression SEMICOLON
			  { $$ = $1; }
			| T_BREAK SEMICOLON
			  { $$ = new ReservedNode(T_BREAK); }
			| T_RETURN expression SEMICOLON
			  { $$ = new ReservedNode(T_RETURN);
			    $$->addNode($2);
			  }
			| T_RETURN SEMICOLON
			  { $$ = new ReservedNode(T_RETURN); }
			| T_IF LPAREN expression RPAREN statement
			  { $$ = new ReservedNode(T_IF); 
			    $$->addNode($3);
			    $$->addNode($5);
			  }
			| T_IF LPAREN expression RPAREN statement T_IFELSE statement
			  { $$ = new ReservedNode(T_IFELSE);
			    $$->addNode($3);
			    $$->addNode($5);
			    $$->addNode($7);
			  }
			| T_WHILE LPAREN expression RPAREN statement
			  { $$ = new ReservedNode(T_WHILE);
			    $$->addNode($3);
			    $$->addNode($5);
			  }
			;

statementexpression	: assignment { $$ = $1; }
			| functioninvocation { $$ = $1; }
			;

primary			: literal { $$ = $1; }
			| LPAREN expression RPAREN { $$ = $2; }
			| functioninvocation { $$ = $1; }
			;

argumentlist		: expression
			{ $$ = new FunctionParamNode(); $$->addNode($1); $$->addParam($1);
			}
			| argumentlist T_COMMA expression
			{ $$->addNode($3); $$->addParam($3); }
			;

functioninvocation	: identifier LPAREN argumentlist RPAREN
			{ $$ = new FunctionInvokationNode($1, $3); $$->addNode($1); $$->addNode($3); }
			| identifier LPAREN RPAREN
			{ $$ = new FunctionInvokationNode($1); $$->addNode($1); }
			;

postfixexpression	: primary
			{ $$ = $1; }
			| identifier
			{ $$ = $1; }
			;

unaryexpression		: SUB unaryexpression
			{ $$ = new UnaryOperationNode(SUB, $2); $$->addNode($2); }
			| NOT unaryexpression
			{ $$ = new UnaryOperationNode(NOT, $2); $$->addNode($2); }
			| postfixexpression
			{ $$ = $1; }
			;
			
multiplicativeexpression: unaryexpression { $$ = $1; }
			| multiplicativeexpression MUL unaryexpression
			{ $$ = new OperationNode($1, MUL, $3); $$->addNode($1); $$->addNode($3); }
			| multiplicativeexpression DIV unaryexpression
			{ $$ = new OperationNode($1, DIV, $3); $$->addNode($1); $$->addNode($3); }
			| multiplicativeexpression MOD unaryexpression
			{ $$ = new OperationNode($1, MOD, $3); $$->addNode($1); $$->addNode($3); }
			;

additiveexpression	: multiplicativeexpression { $$ = $1; }
			| additiveexpression ADD multiplicativeexpression
			{ $$ = new OperationNode($1, ADD, $3); $$->addNode($1); $$->addNode($3); }
			| additiveexpression SUB multiplicativeexpression
			{ $$ = new OperationNode($1, SUB, $3); $$->addNode($1); $$->addNode($3); }
			;

relationalexpression	: additiveexpression { $$ = $1; }
			| relationalexpression LESS additiveexpression
			{ $$ = new OperationNode($1, LESS, $3); $$->addNode($1); $$->addNode($3); }
			| relationalexpression GREAT additiveexpression
			{ $$ = new OperationNode($1, GREAT, $3); $$->addNode($1); $$->addNode($3); }
			| relationalexpression LE additiveexpression
			{ $$ = new OperationNode($1, LE, $3); $$->addNode($1); $$->addNode($3); }
			| relationalexpression GE additiveexpression
			{ $$ = new OperationNode($1, GE, $3); $$->addNode($1); $$->addNode($3); }
			;

equalityexpression	: relationalexpression { $$ = $1; }
			| equalityexpression EQ relationalexpression
			{ $$ = new OperationNode($1, EQ, $3); $$->addNode($1); $$->addNode($3); }
			| equalityexpression NE relationalexpression
			{ $$ = new OperationNode($1, NE, $3); $$->addNode($1); $$->addNode($3); }
			;

conditionalandexpression: equalityexpression { $$ = $1; }
			| conditionalandexpression AND equalityexpression
			{ $$ = new OperationNode($1, AND, $3); $$->addNode($1); $$->addNode($3); }
			;

conditionalorexpression	: conditionalandexpression { $$ = $1; }
			| conditionalorexpression OR conditionalandexpression
			{ $$ = new OperationNode($1, OR, $3); $$->addNode($1); $$->addNode($3); }
			;

assignmentexpression	: conditionalorexpression
			{ $$ = $1; }
			| assignment
			{ $$ = $1; }
			;

assignment		: identifier EQUALS assignmentexpression
			{ $$ = new AssignmentNode($1, $3); $$->addNode($1); $$->addNode($3); }
			;

expression		: assignmentexpression { $$ = $1; }
			;
			
			
%%
