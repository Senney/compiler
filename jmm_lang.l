/* Sean Heintz - 10053525 */

/* Code from this has been referenced from http://www.cs.princeton.edu/~appel/modern/c/software/flex/flex.html */
/* Return values for yylex are defined in LexerDefs.h */

%{
	#include <stdio.h>

	#include "Error.h"
	#include "Token.h"
	#include "Tok_Struct.h"
	#include "ASTNode.h"
	#include "ASTTypes.h"
	#include "jmm_parse.h"
	#include "LexerDefs.h"
%}

%option noyywrap
%option nounput

/* State Definitions */
%x	comment
%x 	str_lit

NUMBER	[0-9]+				
STRING	[a-zA-Z_][a-zA-Z0-9_]*

COMMENT	"//"

/* String lit regex referenced from http://stackoverflow.com/questions/2039795/regular-expression-for-a-string-literal-in-flex-lex */
/* From user Jonathan Feinberg */
STR_LIT \"

/* Match any escape character except newline and quotation. */
STR_IN	(\\.|[^\\"])*

/* Newline */
NL		\n

/* Whitespace */
WS	[ \t\n\r]*

/* Mathematical and Conditional Symbols */
SYM_ADD "+"
SYM_SUB "-"
SYM_MUL "*"
SYM_DIV "/"
SYM_MOD "%"
SYM_NEG	"!"
CON_EQ	"=="
CON_L	"<"
CON_G	">"
CON_LE	"<="
CON_GE	">="
CON_NEQ	"!="
CON_AND	"&&"
CON_OR	"||"

EQUAL	"="

/* Type definitions */
T_INT	"int"
T_BOOL	"boolean"
T_VOID	"void"

/* Other reserved words */
V_TRUE	"true"
V_FALSE	"false"
IF		"if"
ELSE	"else"
WHILE	"while"
BREAK	"break"
RETURN	"return"

/* Reserved symbols */
L_PAR	"("
R_PAR	")"
L_BRA	"{"
R_BRA	"}"
COMMA	","
END_L	";"

%%

{COMMENT} 		BEGIN(comment);
<comment>.		
<comment>{NL}	BEGIN(INITIAL);

{STR_LIT}		BEGIN(str_lit);
<str_lit>{STR_IN}	{ yylval.tok = new Token(Token::LITERAL_STRING, yylineno, yytext); return STRING; }
<str_lit><<EOF>>	{ panic("<L:%d> String not closed before EOF. Check for missing quotation mark.\n", yylineno); BEGIN(INITIAL); return LEXER_ERROR; }
<str_lit>{STR_LIT}	BEGIN(INITIAL);
yylval
<<EOF>>		{ return *yytext; }

{SYM_ADD}	{ yylval.tok = new Token(yytext, yylineno); return ADD; }
{SYM_SUB}	{ yylval.tok = new Token(yytext, yylineno); return SUB; }
{SYM_MUL}	{ yylval.tok = new Token(yytext, yylineno); return MUL; }
{SYM_DIV}	{ yylval.tok = new Token(yytext, yylineno); return DIV; }
{SYM_MOD}	{ yylval.tok = new Token(yytext, yylineno); return MOD; }
{SYM_NEG}	{ yylval.tok = new Token(yytext, yylineno); return NOT; }

{CON_EQ}	{ yylval.tok = new Token(yytext, yylineno); return EQ; }
{CON_L}		{ yylval.tok = new Token(yytext, yylineno); return LESS; }
{CON_G}		{ yylval.tok = new Token(yytext, yylineno); return GREAT; }
{CON_LE}	{ yylval.tok = new Token(yytext, yylineno); return LE; }
{CON_GE}	{ yylval.tok = new Token(yytext, yylineno); return GE; }
{CON_NEQ}	{ yylval.tok = new Token(yytext, yylineno); return NE; }
{CON_AND}	{ yylval.tok = new Token(yytext, yylineno); return AND; }
{CON_OR}	{ yylval.tok = new Token(yytext, yylineno); return OR; }

{EQUAL}		{ yylval.tok = new Token(yytext, yylineno); return EQUALS; }

{T_INT}		{ yylval.tok = new Token(yytext, yylineno); return INT; }
{T_BOOL}	{ yylval.tok = new Token(yytext, yylineno); return BOOLEAN; }
{T_VOID}	{ yylval.tok = new Token(yytext, yylineno); return VOID; }

{V_TRUE}	{ yylval.tok = new Token(yytext, yylineno); return TRUE; }
{V_FALSE}	{ yylval.tok = new Token(yytext, yylineno); return FALSE; }
{IF}		{ yylval.tok = new Token(yytext, yylineno); return T_IF; }
{ELSE}		{ yylval.tok = new Token(yytext, yylineno); return T_IFELSE; }
{WHILE}		{ yylval.tok = new Token(yytext, yylineno); return T_WHILE; }
{BREAK}		{ yylval.tok = new Token(yytext, yylineno); return T_BREAK; }
{RETURN}	{ yylval.tok = new Token(yytext, yylineno); return T_RETURN; }

{L_PAR}		{ yylval.tok = new Token(yytext, yylineno); return LPAREN; }
{R_PAR}		{ yylval.tok = new Token(yytext, yylineno); return RPAREN; }
{L_BRA}		{ yylval.tok = new Token(yytext, yylineno); return LBRACK; }
{R_BRA}		{ yylval.tok = new Token(yytext, yylineno); return RBRACK; }	
{COMMA}		{ yylval.tok = new Token(yytext, yylineno); return T_COMMA; }
{END_L}		{ yylval.tok = new Token(yytext, yylineno); return SEMICOLON; }

{NUMBER} 	{ yylval.tok = new Token(Token::NUMBER_STRING, yylineno, yytext); return NUMBER; }
{STRING}  	{ yylval.tok = new Token(Token::IDENTIFIER_STRING, yylineno, yytext); return ID; }

{WS} 		{ }

. 		{ panic("<L:%d> Unexpected character '%s' encountered.\n", yylineno, yytext); return LEXER_ERROR; }

%%

tok_struct* gettok() {
	tok_struct* tok = (tok_struct*)malloc(sizeof *tok);
	
	tok->type = yylex();
	tok->text = yytext;
	tok->lineno = yylineno;
	
	return tok;
}
