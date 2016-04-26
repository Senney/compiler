/* Sean Heintz - 10053525 */

#include "LexerDefs.h"
#include "JMMLexer.h"

#include "jmm_parse.h"

extern int yylex();
extern tok_struct* gettok();

/* Initializer for JMMLexer */
JMMLexer::JMMLexer() {
	fileHandle = NULL;
}

/* Deconstructs the class. */
JMMLexer::~JMMLexer() {
	close();	// Close the file handle.
}

/* 	Opens a file for the flex parser to parse. 
		file - A filename which specifies the location of the source file.
	Returns
		0 - Success.
	   -1 - Failure.
*/
int JMMLexer::open(std::string file) {
	fileHandle = fopen(file.c_str(), "r");
	if (fileHandle == NULL) return -1;
	
	info("File opened successfully.\n");
	
	yyin = fileHandle;
	return 0;
}

/* 	Closes the stream.
	Returns
		0 - Success.
	   -1 - Failure. (file not initialized or an error while closing the file)
*/
int JMMLexer::close() {
	if (fileHandle != NULL) {
		if (fclose(fileHandle) != 0) {
			warning("File handle could not be closed.\n");
			return -1;
		} else {
			fileHandle = NULL;
			info("File handle closed successfully.\n");
			return 0;
		}
	}
	
	return 0;
}

/*	Gets a token from the Flex lexer and returns it.
	Returns:
		A token created based on what was in the file. 
		NULL - An error has occured.
		EOF Token - We have reached the end of the file.
*/
Token* JMMLexer::getToken() {
	// If we have tokens that were pushed back, return these before doing anything with the input stream.
	if (toks.size() > 0) {
		Token* token = toks.back();
		toks.pop_back();
		return token;
	}

	// Get the token from flex so that we can wrap around it.
	tok_struct* tok = gettok();
	
	if (tok == NULL) {
		return NULL;
	}
	
	return createToken(tok->type, tok->lineno, tok->text);
}

/*	Peak at a token from the input stream without moving to the next token.
	Returns:
		A token from the input stream.
		NULL - An error occured.
		EOF Token - We have reached the end of the file.
*/
Token* JMMLexer::peekToken() {
	Token* tok = getToken();
	
	if (tok != NULL)
		ungetToken(tok);
		
	return tok;
}

/*	Pushes the token back so that it may be gotten again.
	Expects:
		A token that was taken from the input stream.
*/
void JMMLexer::ungetToken(Token* tok) {
	if (tok == NULL)
		return;
		
	toks.push_back(tok);
}

/* 	Create a token from inputs supplied by Flex.
	Expects
		value - Non-zero integer. 0 values return a NULL token.
		line  - The line number.
		text  - The text that has been supplied by yytext.
	Returns
		NULL - Value supplied was invalid or 0.
		Token* - A pointer to a token that represents the read-in value.
*/
Token* JMMLexer::createToken(int value, int line, std::string text) {
	Token* token = NULL;
	
	/* Switch based on the value that was returned from yylex() */
	switch (value) {
		case LEXER_ERROR:
			if (ContinueLexerError) {
				token = new Token(Token::ERROR_STRING, line);
			}
		case LEXER_QUIT:
			break;
		case STRING:
			token = new Token(Token::LITERAL_STRING, line, text);
			break;
		case NUMBER:
			token = new Token(Token::NUMBER_STRING, line, text);
			break;		
		case ID:
			token = new Token(Token::IDENTIFIER_STRING, line, text);
			break;
		case END_OF_FILE:
			token = new Token(Token::EOF_STRING, line);
			break;
		default:
			token = new Token(text, line);
			break;
	}
	
	return token;
}

