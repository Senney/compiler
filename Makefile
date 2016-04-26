# Sean Heintz - 10053525 

SRC_PARSE = jmm_parse.g

EXEC	= Compiler
LEXER 	= -./$(EXEC) -lp
DIR	= Tests/Lexer/

all: compiler
	
compiler: flex parser obj
	g++ -Wall *.o -g -o $(EXEC)
	cp arm_asm/rts.s .

parser:
	bison -t -d $(SRC_PARSE) -o jmm_parse.c
	
flex: jmm_lang.l
	flex --header-file="lex.h" --yylineno jmm_lang.l
	
obj:
	g++ -Wall -g -c lex.yy.c jmm_parse.c *.cpp

clean:
	rm  -f *~ *.o Compiler lex.yy.c jmm_parse.c jmm_parse.h

