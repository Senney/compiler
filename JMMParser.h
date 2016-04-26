#ifndef JMM_PARSER_H
#define JMM_PARSER_H

#include "ASTNode.h"
#include "ASTTypes.h"
#include "JMMLexer.h"
#include "jmm_parse.h"

// Bring in external values from bison.
extern YYSTYPE yylval;
extern int yyparse();
extern BlockNode* root;
	
class JMMParser {
private:
	JMMLexer* m_lexer;

public:
	JMMParser(JMMLexer* lexer);
	
	void setLexer(JMMLexer* lexer);
	bool parse();
	
	ASTNode* getASTRoot() { return root; }
};

#endif