/* A Bison parser, made by GNU Bison 2.0.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
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
     SEMICOLON = 291
   };
#endif
#define INT 258
#define STRING 259
#define ID 260
#define BOOLEAN 261
#define NUMBER 262
#define VOID 263
#define T_IF 264
#define T_IFELSE 265
#define T_BREAK 266
#define T_RETURN 267
#define T_WHILE 268
#define EQ 269
#define GE 270
#define AND 271
#define OR 272
#define LE 273
#define NE 274
#define LESS 275
#define GREAT 276
#define NOT 277
#define TRUE 278
#define FALSE 279
#define MUL 280
#define DIV 281
#define MOD 282
#define ADD 283
#define SUB 284
#define EQUALS 285
#define LPAREN 286
#define RPAREN 287
#define LBRACK 288
#define RBRACK 289
#define T_COMMA 290
#define SEMICOLON 291




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 20 "jmm_parse.g"
typedef union YYSTYPE {
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
} YYSTYPE;
/* Line 1274 of yacc.c.  */
#line 128 "jmm_parse.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;



