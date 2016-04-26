/* Sean Heintz - 10053525 */

#ifndef TOK_STRUCT_H
#define TOK_STRUCT_H

/* This structure is required for my interface between C and C++, since Flex's C++ 
support is spurious and confusing. This structure acts as an intermediate between
my proper Token class and the information that Flex pulls. */

typedef struct {
	int type;
	char* text;
	int lineno;
} tok_struct;

#endif
