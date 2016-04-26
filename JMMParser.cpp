#include <iostream>
#include "Error.h"
#include "Token.h"

#include "JMMParser.h"

JMMParser::JMMParser(JMMLexer* lexer) {
	info("Creating Parser.\n");

	setLexer(lexer);
}

void JMMParser::setLexer(JMMLexer* lexer) {
	m_lexer = lexer;
}

bool JMMParser::parse() {
	info("Parsing file...\n");
	return yyparse() == 0;
}
