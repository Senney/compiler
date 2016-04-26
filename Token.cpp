/* Sean Heintz - 10053525 */

#include "Token.h"

const std::string Token::LITERAL_STRING = "string";
const std::string Token::IDENTIFIER_STRING = "identifier";
const std::string Token::NUMBER_STRING = "number";
const std::string Token::ERROR_STRING = "error";
const std::string Token::EOF_STRING = "eof";

const std::string Token::INT_STRING = "int";
const std::string Token::BOOL_STRING = "boolean";
const std::string Token::VOID_STRING = "void";

/* Constructs the Token with the value entry set to the NO_ENTRY value */
Token::Token(std::string type, int line) {
	this->type = type; this->line = line; this->val = NO_ENTRY;
}

/* Constructs the token with all values defined. */
Token::Token(std::string type, int line, std::string val) {
	this->type = type; this->line = line; this->val = val;
}

/* Convert the Token to a string that represents the token. */
#include <sstream>
std::string Token::toString() {
	std::stringstream ss;
	ss << "Token(" << this->type << ", " << this->line << ", " << this->val << ")";
	return ss.str();
}

bool Token::operator==(const Token &other) const {
  if (this->type == other.type && this->line == other.line && this->val == other.val) return true;
  return false;
}

