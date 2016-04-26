/* Sean Heintz - 10053525 */

#ifndef JMM_TOKEN_H
#define JMM_TOKEN_H

#include <string>

#define NO_ENTRY "NONE"

class Token {
private:
	

public:
	Token(std::string type, int line);
	Token(std::string type, int line, std::string val);
	
	std::string toString();
	
	bool operator==(const Token &other) const;
	
	std::string   type;
	int           line;
	std::string	  val;
	
	static const std::string LITERAL_STRING;
	static const std::string IDENTIFIER_STRING;
	static const std::string NUMBER_STRING;
	static const std::string ERROR_STRING;
	static const std::string EOF_STRING;
	
	// Type strings
	static const std::string INT_STRING;
  static const std::string BOOL_STRING;
  static const std::string VOID_STRING;
};

#endif
