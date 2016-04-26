/* Sean Heintz - 10053525 */

#ifndef JMM_LEXER_H
#define JMM_LEXER_H

#include <string>
#include <cstdio>
#include <iostream>
#include <vector>

#include "ASTNode.h"
#include "ASTTypes.h"
#include "lex.h"
#include "Error.h"
#include "Tok_Struct.h"
#include "Token.h"

// Global flags passed from command line.
extern bool ContinueLexerError;

/* A class that manages the Lexer for the J-- language specification. Allows for a single token to be retrieved from the input stream. */
class JMMLexer {
private:
	FILE* fileHandle;
	
	Token* createToken(int value, int line, std::string text);
	
	/* Used for keeping track of tokens that were peeked at or pushed back. */
	std::vector<Token*> toks;

public:
	JMMLexer();
	~JMMLexer();
	
	int open(std::string file);
	int close();
	
	Token* getToken();
	Token* peekToken();
	void ungetToken(Token* tok);
};

#endif
